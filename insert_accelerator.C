// insert_accelerator.C
// SQLite Insert Accelerator for Python frontend
// Xu Junjie, Kevin
// University of Oregon
// 2014-04-22

#include <iostream>
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
#include <sqlite3.h>
#include <string>
using std::string;
#include <thread>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
namespace bi = boost::interprocess;

// Lock-Free Concurrent Queue from
// https://github.com/cameron314/readerwriterqueue
#include "readerwriterqueue.h"
#include "atomicops.h"

#include "commands.h"
#include "fastq.h"
#include "newpack.h"

// Arguments
// Db Name, filename1, filename2, barcode id

void insert_sequences(string dbName, string fileName1, string fileName2, string barcodeID)
{
	// Check input parameters
	if (dbName.empty() || fileName1.empty() || barcodeID.empty()) {
		return; // early return, do nothing
	}

	// If fileName2 is empty use nonPair method
	bool nonPair = fileName2.empty();

	// Setup memory mapped files for fileName1 and 2
	// create a file mapping from fileName1 in readonly mode
	bi::file_mapping m_file1(fileName1.c_str(),bi::read_only);
	bi::file_mapping m_file2;
	// create the mapped region of the entire file in read only mode
	bi::mapped_region region1(m_file1,bi::read_only);
	bi::mapped_region region2;
	region1.advise(bi::mapped_region::advice_sequential); // inform the OS of our access pattern
	auto* addr1 = (char*)region1.get_address(); // starting pointer
	auto size1 = region1.get_size();
	char* addr2;
	std::size_t size2;
	pip::fastq_parser<const char*> g1;
	pip::fastq fq1;
	char const* f1(addr1); // first iterator/pointer
	char const* l1(f1 + size1); // last iterator/pointer
	pip::fastq_parser<char*> g2;
	pip::fastq fq2;
	char* f2; // first iterator/pointer
	char* l2; // last iterator/pointer
	
	if (!nonPair) { // Map second file
		m_file2 = bi::file_mapping(fileName2.c_str(),bi::read_only);
		region2 = bi::mapped_region(m_file2,bi::read_only);
		addr2 = (char*)region2.get_address();
		size2 = region2.get_size();
		f2 = addr2;
		l2 = f2 + size2;
	}
	
	// Create the concurrent queue
	moodycamel::ReaderWriterQueue<pip::fastq> q(100); // 100 elements
	unsigned long long n = 0;
	// Start the inserts on another thread
	bool can_insert = true;
	std::thread t([&] () {
		// Unless we use a C-Python Wrapper assume we cannot get Python objects
		// So we would have to "get" a db name and open it ourselves
		sqlite3* db;
		int status = sqlite3_open(dbName.c_str(), &db);
	  if (status == SQLITE_OK) {
			// Prepare insert statement
		  sqlite3_stmt *stmt;
			char *sql_error_msg = 0;
		  if (sqlite3_prepare_v2(db,"INSERT INTO reads (barcode_id, read) VALUES (?, ?)",-1,&stmt, NULL) != SQLITE_OK) {
		  	cout << sqlite3_errmsg(db) << endl;
				return 0;
		  }
		  sqlite3_exec(db, "PRAGMA cache_size=400000;PRAGMA journal_mode=MEMORY;PRAGMA locking_mode=EXCLUSIVE;PRAGMA count_changes=OFF;PRAGMA auto_vacuum=NONE;PRAGMA temp_store = MEMORY;PRAGMA synchronous=OFF;BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, &sql_error_msg);
			pip::fastq fq;
			int barcodeIDLength = barcodeID.length();
			
	    while (q.try_dequeue(fq)) {
				pip::pack::Pack packed(fq.sequence,fq.quality,1);
				// Bind parameters to sequence data
				/*sqlite3_bind_text(stmt,1,fq.instrument.c_str(),-1,SQLITE_STATIC);
				sqlite3_bind_int(stmt,2,fq.run);
				sqlite3_bind_text(stmt,3,fq.flowcell.c_str(),-1,SQLITE_STATIC);
				sqlite3_bind_int(stmt,4,fq.lane);
				sqlite3_bind_int(stmt,5,fq.tile);
				sqlite3_bind_int(stmt,6,fq.x);
				sqlite3_bind_int(stmt,7,fq.y);
				sqlite3_bind_int(stmt,8,fq.pair);
				sqlite3_bind_int(stmt,9,fq.filter == 'Y' ? 1:0);
				sqlite3_bind_int(stmt,10,fq.control);
				sqlite3_bind_text(stmt,11,fq.index.c_str(),-1,SQLITE_STATIC);
				sqlite3_bind_int(stmt,12,packed.qualityFormat());*/
				sqlite3_bind_text(stmt,1,barcodeID.c_str(),barcodeIDLength,SQLITE_TRANSIENT);
		    sqlite3_bind_blob(stmt,2,packed.rawData(),fq.sequence.length(),SQLITE_TRANSIENT);
				sqlite3_step(stmt);
		    sqlite3_reset(stmt);
				++n;
	    }
		  sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &sql_error_msg);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
	  }
	});
	unsigned long long i = 0;
	if (nonPair) {
		// Insert single reads
		while (parse(f1,l1,g1,fq1)) {
			q.enqueue(fq1);
			++i;
		}		
	} else {
		// Insert paired reads
		while (parse(f1,l1,g1,fq1) && parse(f2,l2,g2,fq2)) {
			q.enqueue(fq1);
			q.enqueue(fq2);
			i += 2;
		}		
	}
	t.join();
	// Inserts done
	can_insert = false;
	// Sanity Check, number of read sequences should equal # of inserts
	assert(n == i);
}

int main (int argc, char const *argv[])
{
	// Require dbname, and 2 file names: 3 in total
	if (argc >= 4) {
		insert_sequences(argv[1],argv[2],argv[3],argv[4]);
	}
	return 0;
}