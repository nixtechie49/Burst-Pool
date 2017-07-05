#include "sqlIDB_OPUInt.h"
#include "opIDB_OP.h"
#include <cppconn/prepared_statement.h>

#ifdef DEBUG_WITH_DMALLOC
#include "dmalloc.h"
#endif


namespace IDB { 
	std::string sqlIDB_OPUInt::toString() {
		std::string			output;

		output = col + IDB_SQL_OP + "?";

		return output;
	}

	unsigned int sqlIDB_OPUInt::bind(sql::PreparedStatement *pstmt, unsigned int bind_offset) {
		pstmt->setUInt(bind_offset++, value);

		return bind_offset;
	}
}
