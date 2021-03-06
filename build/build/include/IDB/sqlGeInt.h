#include "IDB/where.h"

namespace IDB {
	class sqlGeInt: public Where {
		private:
			std::string		col;
			long int		value;

		public:
			sqlGeInt(std::string init_col, long int init_value): col(init_col), value(init_value) { };
			std::string toString();
			unsigned int bind(sql::PreparedStatement *pstmt, unsigned int bind_offset);
	};
}
