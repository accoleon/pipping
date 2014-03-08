// stream_trimmomatic.C
// Streaming functions for trimmomatic
// Xu Junjie, Kevin
// University of Oregon
// 2014-03-06

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sqlite3.h>

#include "commands.h"
#include "stream_trimmomatic.h"
/* file names
TRIM_IN1=$OUTPREF.1.fastq
TRIM_IN2=$OUTPREF.2.fastq
TRIM_OUT_PAIRED1=$OUTPREF.trim.paired.1.fastq
TRIM_OUT_PAIRED2=$OUTPREF.trim.paired.2.fastq
TRIM_OUT_UNPAIRED1=$OUTPREF.trim.unpaired.1.fastq
TRIM_OUT_UNPAIRED2=$OUTPREF.trim.unpaired.2.fastq
TRIM_LOG=trimming_log.txt
*/
namespace pip {
	namespace stream {
		
		Trimmomatic::Trimmomatic() :
			in1(file_prefix),
			in2(file_prefix),
			log(file_prefix) {}
		Trimmomatic::~Trimmomatic() {}
		
		const char* Trimmomatic::file_prefix = "pip-stream-trimmomatic";
		const char* Trimmomatic::create_delta_tbl = "CREATE TABLE trimmomatic_deltas(readid INT,surviving_sequence_len INT,trim_start INT,last_base INT,trim_end INT);";
		const char* Trimmomatic::insert_deltas = "INSERT INTO trimmomatic_deltas VALUES(?1,?2,?3,?4,?5);";
	
		void Trimmomatic::start_stream(sqlite3* db)
		{
			// for now, assume we are always working on paired end data
			create_pipes(); // create the named pipes
			open_pipes(db);
			cleanup_pipes();
			store_deltas(db);
		}
		
		void Trimmomatic::create_pipes()
		{
			//umask(0);
			in1 += ".1.fastq";
			mkfifo(in1.c_str(),S_IFIFO|0666);
			in2 += ".2.fastq";
			mkfifo(in2.c_str(),S_IFIFO|0666);
			log += ".log";
			mkfifo(log.c_str(),S_IFIFO|0666);
			cout << "create pipes succeeded\n";
		}
		
		void Trimmomatic::cleanup_pipes()
		{
			remove(in1.c_str());
			remove(in2.c_str());
			remove(log.c_str());
		}
		
		void Trimmomatic::write_out(string& file, const ustring& output)
		{
			FILE* fp = fopen(file.c_str(),"w");
			cout << file << " opened writing" << endl;
			if (fp) {
				cout << file << " writing" << endl;
				fwrite(output.c_str(),1,output.length(),fp);
			}
			fclose(fp);
			cout << file << " closed" << endl;
		}
		
		void Trimmomatic::read_log()
		{
			FILE* fp = fopen(log.c_str(),"r");
			cout << log << " opened for reading" << endl;
			char buffer[1000];
			while (!feof(fp)) {
				auto read = fread(buffer,1,1000,fp);
				trimlog.append(buffer,read);
			}
			fclose(fp);
			cout << log << " closed" << endl;
		}
		
		void Trimmomatic::store_deltas(sqlite3* db)
		{
			// Create and insert deltas into db
			sqlite3_stmt *stmt = NULL;
			char *sql_error_msg = 0;
			sqlite3_exec(db,create_delta_tbl,NULL,NULL,&sql_error_msg);
			int rc = sqlite3_prepare_v2(db,insert_deltas,-1,&stmt,NULL);
			if (rc != SQLITE_OK) {
				cout << "SQL command failed: " << sqlite3_errmsg(db) << endl;
				return;
			}
			sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sql_error_msg);
			typedef std::string::const_iterator iterator_type;
			typedef pip::stream::trim_result_parser<iterator_type> trim_result_parser;
			trim_result_parser g;
			trim_result tr;
			auto iter = trimlog.cbegin();
			auto end = trimlog.cend();
			while (parse(iter,end,g,tr)) {
				sqlite3_bind_int(stmt,1,tr.rowid); // read id
				sqlite3_bind_int(stmt,2,tr.surviving_length); // surviving sequence length
				sqlite3_bind_int(stmt,3,tr.trim_start); // trimmed from start
				sqlite3_bind_int(stmt,4,tr.last_base); // last surviving base
				sqlite3_bind_int(stmt,5,tr.trim_end); // trimmed from end
				sqlite3_step(stmt);
		    sqlite3_reset(stmt);
				tr = {};
			}
		  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sql_error_msg);
			sqlite3_finalize(stmt);
		}
		
		void Trimmomatic::open_pipes(sqlite3* db)
		{
			cout << "open pipes started\n";
			sqlite3_stmt *stmt = NULL;
			int rc = sqlite3_prepare_v2(db,sqlite::get_reads,-1,&stmt,NULL);
			if (rc != SQLITE_OK) {
				cout << "SQL command failed: " << sqlite3_errmsg(db) << endl;
				return;
			}
			cout << "sqlite prepare ok\n";
			ustring output1, output2;
			//output1.reserve(100000 * 360); // 100k rows expected 360 chars per row
			//output2.reserve(100000 * 360);
			ustring* co = &output1; // current output string used
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				unsigned char pair;
				if (sqlite3_column_int(stmt,7) == 1) {
					co = &output1;
					pair = '1';
				}
				else {
					co = &output2;
					pair = '2';
				}
				// rationale: trimmomatic does not need or care about the defline,
				// it only really works on the sequence/quality lines and its log
				// outputs what sequences were trimmed. Trimming involves either
				// lopping off the front or back of the sequence (and the trim values)
				// are listed in the log, thus all of the output fastq files from
				// trimmomatic are rendered useless - we can recreate the actual trim
				// from the log.
				*co += '@';
				*co += sqlite3_column_text(stmt,12); //rowid
				/*
				*co += sqlite3_column_text(stmt,0); // instrument
				*co += ':';
				*co += sqlite3_column_text(stmt,1); // runid
				*co += ':';
				*co += sqlite3_column_text(stmt,2); // flowcell
				*co += ':';
				*co += sqlite3_column_text(stmt,3); // lane
				*co += ':';
				*co += sqlite3_column_text(stmt,4); // tile
				*co += ':';
				*co += sqlite3_column_text(stmt,5); // x
				*co += ':';
				*co += sqlite3_column_text(stmt,6); // y
				*co += ' ';
				*co += pair; // pair 
				*co += ':';
				*co += (sqlite3_column_int(stmt,8) == 1 ? 'Y' : 'N'); // filter
				*co += ':';
				*co += sqlite3_column_text(stmt,9); // control
				*co += ':';
				*co += sqlite3_column_text(stmt,10); // index sequence*/
				*co += '\n';
				*co += sqlite3_column_text(stmt,11); // sequence\n+\nquality
				*co += '\n';
			}
			sqlite3_finalize(stmt);

			// We need 2 threads to serve both input files simultaneously
			std::thread tin1([&] { write_out(in1,output1); });
			std::thread tin2([&] { write_out(in2,output2); });
			
			// We need 1 thread to receive the log file
			std::thread tlog([&] { read_log(); });
			
			// Wait for threads
			tin1.join();
			tin2.join();
			tlog.join();
		}

	} /* stream */
} /* pip */

