//
// Experiment with SQLite3 for next generation sequencing pipelines
// 
// John Conery
// University of Oregon
// Dec 2013
//

#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <sstream>
#include <string>
using std::string;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <sqlite3.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "commands.h"
#include "FASTQSequence.h"
#include "fastq.h"
#include "newpack.h"
#include "Piper.h"
using namespace pip;

sqlite3 *db;

void fast_read(string,string&);
void fast_insert(sqlite3*,string&);

bool init_db(string dbname)
{
  char *sql_error_msg = 0;
  //string command(pip::sqlite::create_db);
  
  int status = sqlite3_open(dbname.c_str(), &db);
  
  if (status == SQLITE_OK) {
		//sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sql_error_msg);
    sqlite3_exec(db, sqlite::create_tbls, NULL, NULL, &sql_error_msg);
		//cout << sql_error_msg;
		//status = sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sql_error_msg);
  }
  
  if (status == SQLITE_OK) {
	  // create functions and bind it to the db
	  sqlite::unpackFn(db);
		//cout << "unpack function bound to db\n";
    return true;
  }  
  else {
    cout << "sqlite3: " << sql_error_msg << endl;
    return false;
  }
}

void makefq(sqlite3* db) {
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

int main(int argc, char *argv[])
{
	// Read in command line options
	
	// Define operations:
	// Init db --> we always need the db - this step is always done
	// Merge file into db
	string input_filename;
	string output_dbname;
	po::options_description desc("Options");
	desc.add_options()
		("help,h,?", "produce help message")
		(",o", po::value<string>(&output_dbname), "name of output DB")
		(",i", po::value<string>(&input_filename), "input filename")
	;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}
	
	if (input_filename.empty()) {
		cout << "pip: no file input" << endl;
		return 1;
	}
	
	// Available commands (tentative):
	// init db
	// merge fastq into db
	// do (workflow)
	// define workflow?
	
	// Testing createPipe
	createPipe("test",1);
	createPipe("test",2);

	string input;
	fast_read(input_filename,input);
  
	if (output_dbname.empty())
		output_dbname += "fastq.db";
	
  if (! init_db(output_dbname.c_str())) {
    cout << "pip: error initializing database" << endl;
    return 1;
  }
    
  fast_insert(db,input);
	//makefq(db);
  sqlite3_close(db);
	
  return 0;
}

void fast_read(string filename, string& out)
{
	std::FILE *fp = std::fopen(filename.c_str(), "rb");
	if (fp) {
		std::fseek(fp, 0, SEEK_END);
		out.resize(std::ftell(fp));
		std::rewind(fp);
		std::fread(&out[0],1,out.size(),fp);
		std::fclose(fp);
	}
}

void fast_insert(sqlite3* db, std::string& input)
{
	using boost::spirit::ascii::space;
	typedef std::string::const_iterator iterator_type;
	typedef pip::fastq_parser<iterator_type> fastq_parser;
	fastq_parser g;
	if (!input.empty()) {
	  auto startClock = clock();
	  sqlite3_stmt *stmt;
		char *sql_error_msg = 0;
	  if (sqlite3_prepare_v2(db,sqlite::insert_rawreads,-1,&stmt, NULL) != SQLITE_OK) {
	  	cout << sqlite3_errmsg(db) << endl;
	  } 
	  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sql_error_msg);
		int n = 0;
		pip::fastq fq;
		auto iter = input.cbegin();
		auto end = input.cend();
		while (parse(iter,end,g,fq)) {
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
		}
	  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sql_error_msg);
		sqlite3_finalize(stmt);
		sqlite3_exec(db,sqlite::normalize_rawreads,NULL,NULL,&sql_error_msg);
		auto endClock = clock() - startClock;
		printf("%d sequences imported in %4.2f seconds\n",n,endClock/(double)CLOCKS_PER_SEC);
	}
}
