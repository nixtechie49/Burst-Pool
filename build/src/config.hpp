#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

extern std::string BURST_SERVER;
extern std::string POOL_NAME;

// 10 minutes is about 3ish blocks
extern uint64_t ACCOUNT_UPDATE_TIMEOUT;
// our NUMERICAL account ID (BURST-BANK-DT2R-BM8G-FYFRH)
extern uint64_t OUR_ACCOUNT_ID;
extern std::string OUR_ACCOUNT_RS; /* auto-converted into RS form */
// our account passphrase but encoded
// (temporarily decoded in memory when needed)
extern char *OUR_ACCOUNT_PASSPHRASE;
// deadline maximum
// 30 days
extern uint64_t DEADLINE_MAX;
// deadline really bad
// 100 years?
extern uint64_t DEADLINE_BAD;
// how many blocks to go back to determine historic shares
extern uint64_t HISTORIC_BLOCK_COUNT;
// how many blocks to go back to estimate miner's capacity
extern uint64_t HISTORIC_CAPACITY_BLOCK_COUNT;
// number of seconds that must elapse before a miner can submit a nonce again
extern uint64_t SUBMIT_NONCE_COOLOFF;
// pool fee (e.g. 0.02 is 2%)
extern double POOL_FEE_FRACTION;
// NUMBERIC pool fee account (BURST-PFEE-GLEC-243X-6ESTS)
extern uint64_t POOL_FEE_ACCOUNTID;
// minimum amount before a payout is made to a miner
extern uint64_t MINIMUM_PAYOUT;

extern double SHARE_SCALING_FACTOR;

extern uint64_t CURRENT_BLOCK_REWARD_PERCENT;

extern uint64_t RECENT_BLOCK_HISTORY_DEPTH;

extern uint64_t MAX_PAYOUT_BLOCK_DELAY;

extern uint64_t BONUS_ACCOUNT_ID;
extern std::string BONUS_ACCOUNT_RS;
extern char *BONUS_ACCOUNT_PASSPHRASE;

#endif
