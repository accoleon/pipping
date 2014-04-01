// commands.h
// holds sqlite db command strings

#ifndef COMMANDS_H
#define COMMANDS_H

#include "sqlite3.h"

namespace pip {
	namespace sqlite {
		enum Op {
			MERGE,
			NORMALIZE,
			STREAM		
		};
		extern const char* create_tbls;
		extern const char* insert_rawreads;
		extern const char* normalize_rawreads;
		extern const char* get_reads;
		int unpackFn(sqlite3* db);
		void unpack(sqlite3_context* context,int argc,sqlite3_value** argv);
		void log(sqlite3* db,Op operation,const std::string& table_involved,int sequences,int sequence_len);
	} /* sqlite */
	
} /* pip */

#endif