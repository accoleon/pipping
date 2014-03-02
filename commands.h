// commands.h
// holds sqlite db command strings

#ifndef COMMANDS_H
#define COMMANDS_H

#include <sqlite3.h>

namespace pipping {
	namespace sqlite {
		extern const char *create_db;
		extern const char *insert;
		void unpackSequenceFn(sqlite3 *db);
		void unpackQualityFn(sqlite3 *db);
		void unpackSequence(sqlite3_context *context, int argc, sqlite3_value **argv);
	} /* sqlite */
	
} /* pip */

#endif