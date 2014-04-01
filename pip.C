//
// Experiment with SQLite3 for next generation sequencing pipelines
// 
// John Conery, Xu Junjie, Kevin
// University of Oregon
// Dec 2013
//


#include <fstream>
using std::ifstream;
using std::ofstream;
#include <iostream>
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sqlite3.h>
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
namespace bi = boost::interprocess;

#include "commands.h"
#include "FASTQSequence.h"
#include "fastq.h"
#include "newpack.h"
#include "Piper.h"
#include "stream_trimmomatic.h"
using namespace pip;

sqlite3 *db;

namespace pip {
	enum {
		OK = 0, // no errors whatsoever
		DBFILE_SQL_ERROR, // generic sqlite error
		DBFILE_WRONG_FORMAT, // specified database file is not in pip format
		DBFILE_FINALIZED, // specified database file has been finalized, cannot merge
		MERGE_NO_INPUT, // merge: no input files
		STREAM_NOT_SUPPORTED, // stream: application not supported yet
		STREAM_FILE_EXISTS, // stream: output file already exists
	};
	const char* version = "0.1";
	const char* MESSAGE_STRINGS[] = {
		"OK",
		"Error: unable to access sqlite3 database",
		"Error: specified database file is not in pip format",
		"Error: specified database file is finalized and is read-only",
		"Error: merge requires at least 1 input file",
		"Error: streaming to this application is currently not supported",
		"Error: stream output files already exist, please check"
	};
} /* pip */

// Rationale: our inputs are huge (in the 30+gb range, possibly higher), and
// the original technique of reading the entire file into a string would not
// work on most computers (an aciss compute node has 72gb of ram), and we would
// need alternative methods. Current candidates are:
// 1. Memory mapping - platform dependent and no experience atm - done ~103k inserts per sec
// 2. Read in big ram chunks - performance might be affected
// 3. Capability dependent - the function checks for system memory size then
// 		determines which method to use; e.g., input > memory, chunk. input < mem,
//		read entire file into string. (appears to be the most flexible)
// Boost memory mapped files seem to be the easiest and performant way
// to insert large files into the db.
int mm_read_and_insert(const string& filename, sqlite3* db)
{
	// create a file mapping from filename in readonly mode
	bi::file_mapping m_file(filename.c_str(),bi::read_only);
	// create the mapped region of the entire file in read only mode
	bi::mapped_region region(m_file,bi::read_only);
	region.advise(bi::mapped_region::advice_sequential); // inform the OS of our access pattern
	auto* addr = (char*)region.get_address(); // starting pointer
	auto size = region.get_size();
	pip::fastq_parser<const char*> g;
	
	if (size > 0) {
	  auto startClock = clock();
	  sqlite3_stmt *stmt;
		char *sql_error_msg = 0;
	  if (sqlite3_prepare_v2(db,sqlite::insert_rawreads,-1,&stmt, NULL) != SQLITE_OK) {
	  	cout << sqlite3_errmsg(db) << endl;
			return 0;
	  } 
	  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sql_error_msg);
		unsigned long long n = 0; // we will be dealing with huge numbers here
		pip::fastq fq;
		char const* f(addr); // first iterator/pointer
		char const* l(f + size); // last iterator/pointer
		while (parse(f,l,g,fq)) {
			pack::Pack packed(fq.sequence,fq.quality,1);
			// Bind parameters to sequence data
			sqlite3_bind_text(stmt,1,fq.instrument.c_str(),-1,SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt,2,fq.run);
			sqlite3_bind_text(stmt,3,fq.flowcell.c_str(),-1,SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt,4,fq.lane);
			sqlite3_bind_int(stmt,5,fq.tile);
			sqlite3_bind_int(stmt,6,fq.x);
			sqlite3_bind_int(stmt,7,fq.y);
			sqlite3_bind_int(stmt,8,fq.pair);
			sqlite3_bind_int(stmt,9,fq.filter == 'Y' ? 1:0);
			sqlite3_bind_int(stmt,10,fq.control);
			sqlite3_bind_text(stmt,11,fq.index.c_str(),-1,SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt,12,packed.qualityFormat());
	    sqlite3_bind_blob(stmt,13,packed.rawData(),fq.sequence.length(),SQLITE_TRANSIENT);
			sqlite3_step(stmt);
	    sqlite3_reset(stmt);
			++n;
			fq = {};
			// Show an update every 100k inserts
			if (n % 100000 == 0)
				cerr << "Inserted " << n << " sequences...\r";
		}
	  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sql_error_msg);
		sqlite3_finalize(stmt);
		auto endClock = clock() - startClock;
		printf("Pip: %llu sequences imported in %4.2f seconds\n",n,endClock/(double)CLOCKS_PER_SEC);
		return n; // return number of rows inserted
	}
	return 0;
}

// Initialize a new sqlite database
bool init_db(string dbname)
{
  char *sql_error_msg = 0;
  int status = sqlite3_open(dbname.c_str(), &db);
  
  if (status == SQLITE_OK) {
    sqlite3_exec(db, sqlite::create_tbls, NULL, NULL, &sql_error_msg);
  }
  
  if (status == SQLITE_OK) {
	  // create functions and bind it to the db
	  sqlite::unpackFn(db);
    return true;
  }  
  else {
    cout << "sqlite3: " << sql_error_msg << endl;
    return false;
  }
}

// Test function to dump the entire database into a fastq file
void makefq(sqlite3* db)
{
	// dumps the db reads table into a fastq file
	auto start = clock();
	ofstream os("dump.fastq");
	sqlite3_stmt *stmt = NULL;
	
	int rc = sqlite3_prepare_v2(db,sqlite::get_reads,-1,&stmt,NULL);
	if (rc != SQLITE_OK) {
		cout << "SQL command failed: " << sqlite3_errmsg(db) << endl;
		return;
	}
	int n = 0;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		os << '@' << sqlite3_column_text(stmt,0) << ':'; // instrument
		os << sqlite3_column_text(stmt,1) << ':'; // runid
		os << sqlite3_column_text(stmt,2) << ':'; // flowcell
		os << sqlite3_column_text(stmt,3) << ':'; // lane
		os << sqlite3_column_text(stmt,4) << ':'; // tile
		os << sqlite3_column_text(stmt,5) << ':'; // x
		os << sqlite3_column_text(stmt,6) << ' '; // y
		os << sqlite3_column_text(stmt,7) << ':'; // pair 
		os << (sqlite3_column_int(stmt,8) == 1 ? 'Y' : 'N') << ':'; // filter
		os << sqlite3_column_text(stmt,9) << ':'; // control
		os << sqlite3_column_text(stmt,10) << '\n'; // index sequence
		os << sqlite3_column_text(stmt,11) << '\n'; // sequence\n+\nquality
		++n;
	}
	sqlite3_finalize(stmt);
	printf("%d sequences written in %4.2f seconds\n",n,(clock() - start)/(double)CLOCKS_PER_SEC);
}

// Check if the database is valid according to our specifications
int check_db(string dbfile)
{
	// File does not exist - we can create it
	struct stat statBuffer;
	if (stat(dbfile.c_str(),&statBuffer) != 0) {
		init_db(dbfile);
		return 0;
	}
	// File exists - we can try to open the db and verify the correct tables are in there
	int status = sqlite3_open(dbfile.c_str(), &db);
	if (status == SQLITE_OK) { // file opened ok, now to check its contents
		// check tables
		return 0;
	} else {
		cout << "sqlite3: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		return DBFILE_SQL_ERROR;
	}
}

// Normalize the database by moving the duplicated data such as instrument,
// flowcell, index_sequence into separate tables and replace them with int 
// rowids
int normalize_db(string dbfile)
{
	// Check that we have a database
	int check_result = check_db(dbfile);
	if (check_result != OK) { // if there is anything wrong...
		cout << MESSAGE_STRINGS[check_result] << endl;
		return check_result;
	}
	// Checks all done, proceed
	// TODO some sort of progress indicator, for large dbs
	auto allstart = clock();
	sqlite3_exec(db,"PRAGMA mmap_size=32212254720;",NULL,NULL,NULL);
	auto start = clock();
	cerr << "Pip: normalizing the database...\n";
	sqlite3_exec(db,sqlite::normalize_rawreads,NULL,NULL,NULL);
	auto elapsed = clock() - start;
	cerr << "Pip: normalization complete in " << elapsed/(double)CLOCKS_PER_SEC << " seconds\n";
	cerr << "Pip: vacuuming the database to recover free space...\n";
	start = clock();
	sqlite3_exec(db,"VACUUM;",NULL,NULL,NULL);
	sqlite3_close(db);
	elapsed = clock() - start;
	cerr << "Pip: vacuum complete in " << elapsed/(double)CLOCKS_PER_SEC << " seconds\n";
	printf("Pip: Normalized database in %4.2f seconds\n",(clock()-allstart)/(double)CLOCKS_PER_SEC);
	return OK;
}

// Stream the database to specified application
int stream_db(string dbfile, string app)
{
	if (app != "trimmo") // just a stopgap check for now
		return 1;
	// Check that we have a database
	int check_result = check_db(dbfile);
	if (check_result != OK) { // if there is anything wrong...
		cout << MESSAGE_STRINGS[check_result] << endl;
		return check_result;
	}
	sqlite::unpackFn(db);
	// Checks all done, proceed to stream
	auto start = clock();
	stream::Trimmomatic stream;
	stream.start_stream(db);
	sqlite3_close(db);
	//cout << "Pip: Streamed data in " <<  << " seconds" << endl;
	printf("Pip: Streamed data in %4.2f seconds\n",(clock()-start)/(double)CLOCKS_PER_SEC);
	//cout.flush();
	return OK;
}

int main(int argc, char *argv[])
{
	// Read in command line options using boost:program_options
	string dbfile;
	po::options_description desc("Options");
	desc.add_options()
		("help,h,?","Display this help message")
		("database,d",po::value<string>(&dbfile)->default_value("fastq.db"), "Database file")
		("input-file,i",po::value<vector<string> >(),"Input FASTQ file(s)")
		("merge,m","Merge specified FASTQ files to specified database")
		("normalize,n","Normalize the specified database: merges can no longer be done after normalization")
		("stream,s",po::value<string>(),"Stream data to another application")
		("version,v","Display version")
	;
	po::positional_options_description pd;
	pd.add("input-file",-1);
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
	po::notify(vm);
	
	if (vm.count("help")) { // show help
		cout << desc << endl;
		return 1;
	}
	
	if (vm.count("version")) { // show version
		printf("Pip version %s\n",version);
		return 0;
	}
	
	if(vm.count("merge")) { // merge operation
		// Check that we have a database
		int check_result = check_db(dbfile);
		if (check_result != OK) { // if there is anything wrong...
			cout << MESSAGE_STRINGS[check_result] << endl;
			return 1;
		}
		// Check that we have at least 1 input file
		auto& input_files = vm["input-file"].as<vector<string> >();
		if (input_files.size() == 0) {
			// Error, merge cannot be done
			cout << MESSAGE_STRINGS[MERGE_NO_INPUT] << endl;
			return 1; 
		}
		// Checks all done, proceed
		auto start = clock();
		int total_inserted = 0;
		for (auto& file : input_files) {
			string input;
			total_inserted += mm_read_and_insert(file,db);
		}
		
		sqlite3_close(db);
		printf("Merged %lu files in %4.2f seconds\n",input_files.size(),(clock()-start)/(double)CLOCKS_PER_SEC);
	}
	
	if (vm.count("normalize")) { // normalize operation
		normalize_db(dbfile);
	}
	
	if (vm.count("stream")) { // stream operation
		stream_db(dbfile,vm["stream"].as<string>());
	}
	
	// Available commands (tentative):
	// init db
	// merge fastq into db
	// do (workflow)
	// define workflow?

	sqlite3_close(db);
  return 0;
}