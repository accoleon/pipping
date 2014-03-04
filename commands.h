// commands.h
// holds sqlite db command strings

#ifndef COMMANDS_H
#define COMMANDS_H

#include <sqlite3.h>

namespace pip {
	namespace sqlite {
		extern const char* create_db;
		extern const char* insert;
		int unpackFn(sqlite3 *db);
		void unpack(sqlite3_context *context, int argc, sqlite3_value **argv);
	} /* sqlite */
	
} /* pip */

#endif