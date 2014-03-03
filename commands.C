#include <sqlite3.h>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include "commands.h"
#include "newpack.h"
using namespace pip::pack;

#define SQLITE_DETERMINISTIC    0x800

namespace pip
{
	namespace sqlite
	{
		const char* create_db = "CREATE TABLE reads(defline TEXT,qualityformat INT,data BLOB)";
		const char* insert = "INSERT INTO reads VALUES (?1,?2,?3)";
		
		int unpackFn(sqlite3 *db) {
			return sqlite3_create_function(
				db,
				"unpack",
				3, // 3 parameters: blob, blob length, qualityformat
				SQLITE_UTF8 | SQLITE_DETERMINISTIC,
				NULL,
				&unpack,
				NULL,
				NULL);
		}
		
		void unpack(sqlite3_context *context, int argc, sqlite3_value **argv) {
			if (argc == 3) {
				int length = sqlite3_value_int(argv[1]);
				auto qf = static_cast<QualityFormat>(sqlite3_value_int(argv[2]));
				auto data = (unsigned char*)sqlite3_value_blob(argv[0]);
				string outSequence, outQuality;
				pip::pack::losslessUnpack(data,length,qf,outSequence,outQuality);
				outSequence.reserve(20 + length * 2);
				outSequence += "\n+\n";
				outSequence += outQuality;
				sqlite3_result_text(context, outSequence.c_str(), -1, SQLITE_TRANSIENT);
				return;
			}
			sqlite3_result_null(context);
		}
	} /* sqlite */
} /* pip */