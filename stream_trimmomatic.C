// stream_trimmomatic.C
// Streaming functions for trimmomatic
// Xu Junjie, Kevin
// University of Oregon
// 2014-03-06

#include <chrono>
#include <future>
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
		const char* Trimmomatic::create_delta_tbl = "DROP TABLE IF EXISTS trimmomatic_deltas;CREATE TABLE trimmomatic_deltas(readid INT,surviving_sequence_len INT,trim_start INT,last_base INT,trim_end INT);";
		const char* Trimmomatic::insert_deltas = "INSERT INTO trimmomatic_deltas VALUES(?1,?2,?3,?4,?5);";
		const char* Trimmomatic::get_reads = "SELECT r.rowid,r.pair,unpack(r.data,length(r.data),r.qualityformat) FROM reads r;";
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
			//cout << "create pipes succeeded\n";
		}
		
		void Trimmomatic::cleanup_pipes()
		{
			remove(in1.c_str());
			remove(in2.c_str());
			remove(log.c_str());
		}
		
		bool Trimmomatic::write_out(string& file, const ustring& output)
		{
			FILE* fp = fopen(file.c_str(),"wb");
			//cout << file << " opened writing" << endl;
			if (fp) {
				//cout << file << " writing" << endl;
				fwrite(output.c_str(),1,output.length(),fp);
			}
			fclose(fp);
			//cout << file << " closed" << endl;
			return true;
		}
		
		bool Trimmomatic::read_log()
		{
			FILE* fp = fopen(log.c_str(),"rb");
			//cout << log << " opened for reading" << endl;
			// Reading from named pipes doesn't cause disk I/O and thus very fast,
			// but we still want to reduce the amount of fread calls due to its
			// associated overhead. Since we are running on modern computers, we could
			// work with a large buffer and reduce the number of freads we have to do.
			const int buffer_size = 4096;
			char buffer[buffer_size]; // just a power of 2 buffer
			int n = 0;
			while (!feof(fp)) {
				auto read = fread(buffer,1,buffer_size,fp);
				trimlog.append(buffer,read);
				++n;
			}
			fclose(fp);
			cout << "number of freads/appends actually called while reading log: " << n << endl;
			//cout << log << " closed" << endl;
			return true;
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
			typedef std::string::const_iterator iterator_type;
			typedef pip::stream::trim_result_parser<iterator_type> trim_result_parser;
			trim_result_parser g;
			trim_result tr;
			auto iter = trimlog.cbegin();
			auto end = trimlog.cend();
			sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sql_error_msg);
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
			sqlite3_stmt *stmt = NULL;
			int rc = sqlite3_prepare_v2(db,get_reads,-1,&stmt,NULL);
			if (rc != SQLITE_OK) {
				cout << "SQL command failed: " << sqlite3_errmsg(db) << endl;
				return;
			}
			ustring output1, output2;
			// once metadata is stored we can use metadata to initialize our output
			// to the correct size, should be a performance increases
			output1.reserve(100000 * 200); // currently expect these sizes
			output2.reserve(100000 * 200);
			ustring* co = &output1; // current output string used
			int n = 0;
			auto start = clock();
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				unsigned char pair;
				if (sqlite3_column_int(stmt,1) == 1) {
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
				*co += sqlite3_column_text(stmt,0); // rowid
				*co += '\n';
				*co += sqlite3_column_text(stmt,2); // sequence\n+\nquality
				*co += '\n';
				++n;
			}
			sqlite3_finalize(stmt);
			printf("Pip: %d sequences loaded in %4.2f seconds\n",n,(double)(clock()-start)/CLOCKS_PER_SEC);
			
			// We need 2 threads to serve both input files simultaneously
			// Using C++11 futures (so we can display a progress indicator)
			auto tin1 = std::async(std::launch::async,[&] { return write_out(in1,output1); });
			auto tin2 = std::async(std::launch::async,[&] { return write_out(in2,output2); });
			
			// We need 1 thread to receive the log file
			auto tlog = std::async(std::launch::async,&Trimmomatic::read_log,this);
			
			// Display fancy animation while we wait for threads
			std::chrono::milliseconds span (100);
			static const char spinner[] = "/-\\|";
			int i = 0;
			while (tin1.wait_for(span)==std::future_status::timeout || tin2.wait_for(span)==std::future_status::timeout || tlog.wait_for(span)==std::future_status::timeout ) {
					std::cerr << "Streaming..." << spinner[i % sizeof(spinner)] << "\r";
					++i;
			}
			tin1.get();
			tin2.get();
			tlog.get();
		}

	} /* stream */
} /* pip */

