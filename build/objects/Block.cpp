/*
		Create table Blocks (
			blockID						bigint unsigned not null unique,
			first_seen_when				timestamp not null default current_timestamp,
			best_nonce_account_id		bigint unsigned,
			generator_account_id		bigint unsigned,
			block_reward				bigint unsigned,
			is_our_block				boolean not null default false,
			has_been_shared				boolean not null default false,
			base_target					bigint unsigned,
			forged_when					timestamp null default null,
			scoop						bigint unsigned,
			nonce						bigint unsigned,
			gen_sig_str					char(64),
			deadline					bigint unsigned,
			our_best_deadline			bigint unsigned,
			num_potential_miners		bigint unsigned,
			json						varchar(255),
			primary key					(blockID),
			index						(is_our_block, has_been_shared)
		);
*/

#include "Nonce.hpp"
#include "Share.hpp"
#include "Reward.hpp"

#include "mining-info.hpp"
#include "config.hpp"

#include <math.h>

#include "Block.cxx"


SEARCHMOD( after_blockID, uint64_t );
SEARCHMOD( before_blockID, uint64_t );
SEARCHMOD( has_reward_value, bool );



CHILD_OBJECTS(Nonce, block_nonces);
CHILD_OBJECTS(Share, block_shares);
CHILD_OBJECTS(Reward, block_rewards);



SEARCHPREP {
	SEARCHPREP_SUPER;

	if ( SEARCHMOD_IS_SET(after_blockID) ) {
		IDB::Where *new_clause = new IDB::sqlGtUInt64( "Blocks.blockID", SEARCHMOD_VALUE(after_blockID) );
		SEARCHPREP_ADD( new_clause );
	}

	if ( SEARCHMOD_IS_SET(before_blockID) ) {
		IDB::Where *new_clause = new IDB::sqlLtUInt64( "Blocks.blockID", SEARCHMOD_VALUE(before_blockID) );
		SEARCHPREP_ADD( new_clause );
	}

	if ( SEARCHMOD_IS_SET(has_reward_value) ) {
		IDB::Where *new_clause = new IDB::sqlIsNotNull( "block_reward" );
		SEARCHPREP_ADD( new_clause );
	}

	SEARCHPREP_END;
}


STATIC Block *Block::latest_block() {
	Block blocks;
	blocks.order_by( COL_blockID, ORDER_BY_DESC );
	return blocks.present();
}


STATIC uint64_t Block::latest_blockID() {
	// this code is slightly more raw/closer to SQL
	// as we're just after the blockID
	// so no need to load the whole block record
	// plus blockID should come direct from index

	IDB::Tables tables( Block::_my_table );

	IDB::Options options;
	options.order_by = "blockID DESC";
	options.limit = 1;

	return Block::idbe()->fetchInt( "blockID", &tables, IDB_NO_WHERE, &options );
}


STATIC Block *Block::latest_won_block() {
	Block blocks;
	blocks.is_our_block(true);
	blocks.order_by( COL_blockID, ORDER_BY_DESC );
	return blocks.present();
}


STATIC Nonce *Block::find_best_nonce( uint64_t blockID ) {
	Nonce nonces;
	nonces.blockID( blockID );
	nonces.is_blocks_best_deadline(true);

	return nonces.present();
}


STATIC void Block::recalculate_shares( uint64_t blockID ) {
	// now assumes it is already within a block-serialized transaction

	Nonce nonces;
	nonces.blockID(blockID);
	nonces.is_accounts_best_deadline(true);
	nonces.search();

	std::vector<Nonce *> nonces_to_share;
	uint64_t total_shares = 0.0;
	std::vector<uint64_t> shares;

	while( Nonce *nonce = nonces.result() ) {
		nonces_to_share.push_back( nonce );

		// it's actually possible (although rare) for deadline to be zero!
		uint64_t deadline = nonce->deadline() + 1;

		long double share = pow(deadline, SHARE_SCALING_FACTOR );

		uint64_t big64int = 0x7fffffffffffffff;
		// std::cout << "deadline: " << deadline << ", scaled deadline: " << share << ", share: " << (big64int/share) << std::endl;

		share = big64int / share;
		shares.push_back( share );
		total_shares += share;
	}

	// std::cout << "share total: " << total_shares << std::endl;

	std::unique_ptr<Block> block( Block::load( blockID ) );

	// no need to wipe old shares
	// as existing shares only get insert/updated, never deleted
	long double share_fraction_total = 0.0;

	for(int i=0; i<nonces_to_share.size(); i++) {
		Nonce *nonce = nonces_to_share[i];
		long double share_fraction = (long double)(shares[i]) / (long double)(total_shares);
		share_fraction_total += share_fraction;

		// std::cout << "share fraction: " << share_fraction << std::endl;

		Share share;
		share.blockID(blockID);
		share.accountID( nonce->accountID() );
		share.share_fraction( share_fraction );
		share.deadline( nonce->deadline() );
		share.deadline_string( nonce->deadline_string() );
		share.miner( nonce->miner() );
		share.save();

		delete nonce;
	}

	// std::cout << "share fraction total: " << share_fraction_total << std::endl;
}


void Block::reward_miners() {
	if ( is_our_block() && !has_been_shared() ) {
		uint64_t reward_to_share = block_reward() * BURST_TO_NQT;
		std::cout << "[rewards] reward to share: " << reward_to_share << std::endl;

		// remove fees, etc. from potential reward to be shared
		uint64_t pool_fee = reward_to_share * POOL_FEE_FRACTION;
		reward_to_share -= pool_fee - PAYMENT_SEND_FEE;

		// make this a reward payment
		Reward pool_fee_reward;
		pool_fee_reward.accountID( POOL_FEE_ACCOUNTID );
		pool_fee_reward.amount( pool_fee );
		pool_fee_reward.blockID( blockID() );
		pool_fee_reward.save();

		Share current_shares;
		current_shares.blockID( blockID() );

		uint64_t transaction_fees = current_shares.count() * PAYMENT_SEND_FEE;
		reward_to_share -= transaction_fees;

		std::unique_ptr<Share> historic_shares( Share::historic_shares( blockID() - 1, HISTORIC_BLOCK_COUNT ) );

		std::cout << "[rewards] pool fee: " << pool_fee << ", max transaction fees: " << transaction_fees << ", reward leftover: " << reward_to_share << std::endl;

		uint64_t total_rewarded = 0;

		std::map<uint64_t,Reward *> rewards_by_accountID;

		current_shares.search();
		while( Share *share = current_shares.result() ) {
			uint64_t amount = reward_to_share * share->share_fraction() * CURRENT_BLOCK_REWARD_PERCENT / 100;

			Reward *reward = new Reward();
			reward->amount( amount );

			rewards_by_accountID[ share->accountID() ] = reward;

			std::cout << "[rewards] finding fee " << std::to_string( amount ) << " to " << std::to_string( share->accountID() ) << std::endl;
		}

		historic_shares->search();
		while( Share *share = historic_shares->result() ) {
			uint64_t amount = reward_to_share * share->share_fraction() * (100 - CURRENT_BLOCK_REWARD_PERCENT) / 100;

			if ( rewards_by_accountID.find( share->accountID() ) == rewards_by_accountID.end() )
				rewards_by_accountID[ share->accountID() ] = new Reward();

			Reward *reward = rewards_by_accountID[ share->accountID() ];
			reward->amount( reward->amount() + amount );

			std::cout << "[rewards] historic fee " << std::to_string( amount ) << " to " << std::to_string( share->accountID() ) << std::endl;
		}

		for(auto it : rewards_by_accountID) {
			Reward *reward = it.second;

			reward->blockID( blockID() );
			reward->accountID( it.first );
			reward->save();

			std::cout << "[rewards] account " << reward->accountID() << " reward is: " << reward->amount() << std::endl;
			total_rewarded += reward->amount();
		}

		has_been_shared(true);
		save();

		std::cout << "[rewards] total rewards (excluding fees): " << total_rewarded << std::endl;
	}
}


STATIC uint64_t Block::previous_reward_post_fee() {
	uint64_t latest_blockID = Block::latest_blockID();

	Block blocks;
	blocks.order_by( COL_blockID, ORDER_BY_DESC );
	blocks.has_reward_value(true);

	std::unique_ptr<Block> prev_block( blocks.present() );

	if (!prev_block)
		return 0;

	// works even if prev_block_reward is 0
	return prev_block->block_reward() * (1 - POOL_FEE_FRACTION);
}


STATIC void Block::unpack_gen_sig_str( const std::string gen_sig_str, uint8_t *gen_sig ) {
	for(int i=0; i<32; i++)
		gen_sig[i] = strtoul( gen_sig_str.substr( i<<1, 2).c_str(), NULL, 16 );
}

