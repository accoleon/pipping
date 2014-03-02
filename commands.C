#include <sqlite3.h>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include "commands.h"
#include "pack.h"

#define SQLITE_DETERMINISTIC    0x800

namespace pipping
{
	namespace sqlite
	{
		const char *create_db = "create table reads ( id int unsigned not null, defline text, sequence text, quality text )";
		const char *insert = "INSERT INTO reads VALUES (?1,?2,?3,?4)";
		
		void unpackSequenceFn(sqlite3 *db) {
			sqlite3_create_function(
				db,
				"unpackSequence",
				1,
				SQLITE_UTF8 | SQLITE_DETERMINISTIC,
				NULL,
				&unpackSequence,
				NULL,
				NULL);
		}
		
		void unpackSequence(sqlite3_context *context, int argc, sqlite3_value **argv) {
			if (argc == 1) {
				int length = sqlite3_value_bytes(argv[0]) / 2; // 2 bits represent 1 character
				vector<bool> packed;
				packed.resize(length);
				std::copy(packed.data(), length, sqlite3_value_blob(argv[0]));
				string output(pipping::pack::unpackSequence(packed));
				sqlite3_result_text(context, output.c_str(), -1, SQLITE_TRANSIENT);
				return;
			}
			sqlite3_result_null(context);
		}
	} /* sqlite */
} /* pip */