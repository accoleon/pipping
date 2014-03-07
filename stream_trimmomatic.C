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
			outp1(file_prefix),
			outp2(file_prefix),
			outup1(file_prefix),
			outup2(file_prefix),
			log(file_prefix) {}
		Trimmomatic::~Trimmomatic() {}
		
		const char* Trimmomatic::file_prefix = "pip-stream-trimmomatic";
	
		void Trimmomatic::start_stream(sqlite3* db)
		{
			// for now, assume we are always working on paired end data
			create_pipes(); // create the 7 named pipes
			open_pipes(db);
		}
		
		void Trimmomatic::create_pipes()
		{
			//umask(0);
			in1 += ".1.fastq";
			mkfifo(in1.c_str(),S_IFIFO|0666);
			in2 += ".2.fastq";
			mkfifo(in2.c_str(),S_IFIFO|0666);
			/*outp1 += ".trim.paired.1.fastq";
			mkfifo(outp1.c_str(),S_IFIFO|0666);
			outp2 += ".trim.paired.2.fastq";
			mkfifo(outp2.c_str(),S_IFIFO|0666);
			outup1 += ".trim.unpaired.1.fastq";
			mkfifo(outup1.c_str(),S_IFIFO|0666);
			outup2 += ".trim.unpaired.1.fastq";
			mkfifo(outup2.c_str(),S_IFIFO|0666);
			log += ".log.txt";
			mkfifo(log.c_str(),S_IFIFO|0666);*/
			cout << "create pipes succeeded\n";
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
			typedef std::basic_string <unsigned char> ustring;
			ustring output1, output2;
			//output1.reserve(100000 * 360); // 100k rows expected 360 chars per row
			//output2.reserve(100000 * 360);
			ustring* co = &output1; // current output string used
			int n = 0;
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
				*co += '@';
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
				*co += sqlite3_column_text(stmt,10); // index sequence
				*co += '\n';
				*co += sqlite3_column_text(stmt,11); // sequence\n+\nquality
				*co += '\n';
				//cout << co->c_str() << "\n";
			}
			sqlite3_finalize(stmt);

			// We need 2 proceses to serve both input files simultaneously
			pid_t childPID = fork();
			if (childPID >= 0) { //successful fork
				if (childPID == 0) { // child process
					FILE* fp2 = fopen(in2.c_str(), "w");
					cout << "fp2 opened" << endl;
					if (fp2) {
						cout << "fp2 writing" << endl;
						fwrite(output2.c_str(),1,output2.length(),fp2);
					}
					fclose(fp2);
					cout << "fp2 closed" << endl;
				}
				else { // parent process
					FILE* fp1 = fopen(in1.c_str(), "w");
					cout << "fp1 opened" << endl;
					if (fp1) {
						cout << "fp1 writing" << endl;
						fwrite(output1.c_str(),1,output1.length(),fp1);
					}
					fclose(fp1);
					cout << "fp1 closed" << endl;
				}
			}
			else { // fork failed
				
			}
			
			
			/*FILE* pair1 = fopen(outp1.c_str(), "r");
			char readbuf[370];
			while (fgets(readbuf,370,pair1)) {
				printf("received: %s\n", readbuf);
			}
			fclose(pair1);*/
			/*FILE* fp3 = fopen(in1.c_str(), "w");
			//cout << "did fopen fp1 succeed?\n";
			if (fp3) {
				fwrite(output1.c_str(),1,output1.length(),fp3);
				//cout << "output1 length: " << output1.length();
			}
			fclose(fp3);
			FILE* fp4 = fopen(in2.c_str(), "w");
			if (fp4) {
				fwrite(output2.c_str(),1,output2.length(),fp4);
				//cout << "output2 length: " << output2.length();
			}
			fclose(fp4);*/
		}
	} /* stream */
} /* pip */