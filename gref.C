// Scan a FASTQ sequence file

// John Conery
// Oct 2013

// Use this program as a baseline to time how long it takes just to read a
// large FASTQ file.

#include <iostream>
using std::cin;
using std::cout;
using std::endl;
using std::ostream;

#include <fstream>
using std::ifstream;

//#include <omp.h>

#include <string>
using std::string;

#include <sqlite3.h>

#include "FASTQSequence.h"
#include "newpack.h"

sqlite3 *db;

bool init_db(string dbname)
{
  char *sql_error_msg = 0;
  string command("CREATE TABLE rawreads(instrument TEXT,runid INT,flowcell TEXT,lane INT,tile INT,x INT,y INT,pair INT,filter INT,control INT,index_sequence TEXT,qualityformat INT,data BLOB);");
  
  int status = sqlite3_open(dbname.c_str(), &db);
  
  if (status == SQLITE_OK) {
    status = sqlite3_exec(db, command.c_str(), 0, 0, &sql_error_msg);
  }
  
  if (status == SQLITE_OK) {
    return true;
  }  
  else {
    cout << "sqlite3: " << sql_error_msg << endl;
    return false;
  }
}

int main(int argc, char *argv[])
{  
	char *sql_error_msg = 0;
  string filename(argv[1]);
  ifstream readfile;

	if (! init_db("fastq.db")) {
		cout << "pip: error initializing database" << endl;
	  return 1;
	}
		
  readfile.open(filename.c_str());
    
  int count = 0;
  FASTQSequence x;
	
	char insert_cmnd[] = "INSERT INTO rawreads VALUES (?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13);";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, insert_cmnd, strlen(insert_cmnd), &stmt, NULL);
	
  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sql_error_msg);
	
  while (x.fetch(readfile)) {
		x.parse_defline();
    pip::pack::Pack packed(x.sequence(),x.quality(),1);
			// Bind parameters to sequence data
			sqlite3_bind_text(stmt,1,x.instrument_id().c_str(),-1,SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt,2,x.run_id());
			sqlite3_bind_text(stmt,3,x.flow_cell_id().c_str(),-1,SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt,4,x.flow_cell_lane());
			sqlite3_bind_int(stmt,5,x.tile());
			sqlite3_bind_int(stmt,6,x.x());
			sqlite3_bind_int(stmt,7,x.y());
			sqlite3_bind_int(stmt,8,x.pair());
			sqlite3_bind_int(stmt,9,x.filter() == true ? 1:0);
			sqlite3_bind_int(stmt,10,x.control_bits());
			sqlite3_bind_text(stmt,11,x.index().c_str(),-1,SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt,12,packed.qualityFormat());
	    sqlite3_bind_blob(stmt,13,packed.rawData(),x.sequence().length(),SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cout << "fail..." << endl;
    }
    sqlite3_reset(stmt);
    count += 1;
  }
	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sql_error_msg);
  sqlite3_close(db);
  cout << count << endl;
  //double end = omp_get_wtime();
	//cout << "Total omp walltime: " << end-start << end;
  return 0;
}
