#include "cJSON.hpp"

extern int DB_POOL_SIZE;
extern char *BIND_ADDRESS;
extern char *BIND_ADDRESS6;
extern int LISTEN_PORT;
extern char *DOC_ROOT;
extern char *LOG_FILE;

extern char *DB_URI;
extern char *DB_SCHEMA;
extern char *DB_USER;
extern char *DB_PASSWORD;

extern bool HTTP_LOGGING;


char *config_item( cJSON *root, const char *valueName, bool mandatory = true );
// this is supplied somewhere else outside of this library
void more_config( cJSON *root );
void config_init();
