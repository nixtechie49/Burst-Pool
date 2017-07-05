// generated hpp

#ifndef __HANDLER__webAPI__updates
#define __HANDLER__webAPI__updates

#include "Handler.hpp"
#include "config.hpp"
#include "WebSocketHandler.hpp"

#include "Block.hpp"

#include "cJSON.hpp"
#include <map>

typedef std::map<uint64_t,time_t> seen_accounts_t;

namespace Handlers {

	namespace webAPI {

		class updates: public WebSocketHandler<updates> {
			private:
				bool need_initial_info;
				seen_accounts_t my_seen_accounts;

				static Block *current_block;
				static time_t started_when;

				static bool send_block_json;
				static std::string block_json;
				static std::string shares_json;
				static seen_accounts_t all_seen_accounts;

				void update_account_info( uint64_t accountID );
				void add_account_award( cJSON *awards, char *award_name, Nonce *nonce, time_t block_start_time );
				cJSON *generate_awards( uint64_t blockID, time_t block_start_time );


			public:
				virtual void websocket_alert();
				updates();
				static void init();

		};

	} // webAPI namespace

} // Handlers namespace


#endif
