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
#include "newpack.h"
using namespace pip;

sqlite3 *db;

bool init_db(string dbname)
{
  char *sql_error_msg = 0;
  //string command(pip::sqlite::create_db);
  
  int status = sqlite3_open(dbname.c_str(), &db);
  
  if (status == SQLITE_OK) {
    status = sqlite3_exec(db, sqlite::create_db, NULL, NULL, &sql_error_msg);
  }
  
  
  
  if (status == SQLITE_OK) {
	  // create functions and bind it to the db
	  int ret = sqlite::unpackFn(db);
		cout << "unpack function bound to db: " << ret << "\n";
    return true;
  }  
  else {
    cout << "sqlite3: " << sql_error_msg << endl;
    return false;
  }
}

void makefq(sqlite3* db) {
	// dumps the db reads table into a fastq file
	ofstream os("dump.fastq");
	sqlite3_stmt *stmt = NULL;
	
	int rc = sqlite3_prepare_v2(
		db,
		"SELECT defline,unpack(data,length(data),qualityformat) FROM reads;",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK) {
		cout << "sql command failed" << endl;
		return;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		os << sqlite3_column_text(stmt,0) << "\n";
		os << sqlite3_column_text(stmt,1) << "\n";
	}
	sqlite3_finalize(stmt);
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
	
  ifstream readfile(input_filename);
  char *sql_error_msg = 0;
  
	if (output_dbname.empty())
		output_dbname += "fastq.db";
	
  if (! init_db(output_dbname.c_str())) {
    cout << "pip: error initializing database" << endl;
    return 1;
  }
    
  FASTQSequence x;
  
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db,sqlite::insert,-1,&stmt, NULL);
  
  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sql_error_msg);
  
  while (x.fetch(readfile)) {
		pack::Pack packed(x.sequence(), x.quality(),1);
	  sqlite3_bind_text(stmt, 1, x.defline().c_str(), x.defline().length(), SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, packed.qualityFormat());
    sqlite3_bind_blob(stmt, 3, packed.rawData(), x.sequence().length(), SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cout << "fail..." << endl;
    }
    sqlite3_reset(stmt);
  }
  
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sql_error_msg);
  makefq(db);
  sqlite3_close(db);
  return 0;
}

