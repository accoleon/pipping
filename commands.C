#include <iostream>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <sqlite3.h>
#include "commands.h"
#include "newpack.h"
using namespace pip::pack;

#define SQLITE_DETERMINISTIC 0x800 // don't know why but my sqlite3.h doesn't seem to have it

// http://probertson.com/articles/2009/11/30/multi-table-insert-one-statement-air-sqlite/
// Reference for efficient and normalized inserts

namespace pip
{
	namespace sqlite
	{
		const char* create_tbls = "CREATE TABLE rawreads(instrument TEXT,runid INT,flowcell TEXT,lane INT,tile INT,x INT,y INT,pair INT,filter INT,control INT,index_sequence TEXT,qualityformat INT,data BLOB);"
			"CREATE TABLE reads(instrumentid INT,runid INT,flowcellid INT,lane INT,tile INT,x INT,y INT,pair INT,filter INT,control INT,index_sequence INT,qualityformat INT,data BLOB);"
			"CREATE TABLE meta(date TEXT,operation TEXT,table_involved TEXT,rows_involved);"
			"CREATE TABLE instruments(name TEXT);"
			"CREATE TABLE flowcells(name TEXT);"
			"CREATE TABLE index_sequences(name TEXT);";
		const char* insert_rawreads = "INSERT INTO rawreads VALUES (?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13);";
		
		// Normalization - we don't want to normalize everything - only the 
		// components that take up a lot of space (such as TEXT). SQLite already
		// saves int space if they are unused.
		// Normalize instrument id, flowcell id, index sequence
		const char* normalize_rawreads = "INSERT INTO instruments SELECT DISTINCT instrument FROM rawreads;"
			"INSERT INTO flowcells SELECT DISTINCT flowcell FROM rawreads;"
			"INSERT INTO index_sequences SELECT DISTINCT index_sequence FROM rawreads;"
			"INSERT INTO reads SELECT i.rowid,r.runid,f.rowid,r.lane,r.tile,r.x,r.y,r.pair,r.filter,r.control,ind.rowid,r.qualityformat,r.data "
			"FROM rawreads r,instruments i,flowcells f,index_sequences ind "
			"WHERE r.instrument=i.name AND r.flowcell=f.name AND r.index_sequence=ind.name;"
			"DROP TABLE rawreads;VACUUM"; // drop rawreads and vacuum up the space
		const char* get_reads = "SELECT i.name,r.runid,f.name,r.lane,r.tile,r.x,r.y,r.pair,r.filter,r.control,ind.name,unpack(r.data,length(r.data),r.qualityformat) "
			"FROM reads r,instruments i,flowcells f,index_sequences ind "
			"WHERE r.instrumentid=i.rowid AND r.flowcellid=f.rowid AND r.index_sequence=ind.rowid;";
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
				auto data = (unsigned char*)sqlite3_value_blob(argv[0]);
				int length = sqlite3_value_int(argv[1]);
				auto qf = static_cast<QualityFormat>(sqlite3_value_int(argv[2]));
				string outSequence, outQuality;
				pip::pack::losslessUnpack(data,length,qf,outSequence,outQuality);
				outSequence.reserve((length * 2)+3);
				outSequence += "\n+\n";
				outSequence += outQuality;
				sqlite3_result_text(context,outSequence.c_str(),-1,SQLITE_TRANSIENT);
				return;
			}
			sqlite3_result_null(context);
		}
	} /* sqlite */
} /* pip */