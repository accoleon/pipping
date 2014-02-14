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

#include <sqlite3.h>

#include "FASTQSequence.h"

sqlite3 *db;

bool init_db(string dbname)
{
  char *sql_error_msg = 0;
  string command("create table reads ( id int unsigned not null, defline text, sequence text, quality text )");
  
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
  string filename(argv[1]);
  ifstream readfile;
  char *sql_error_msg = 0;
    
  readfile.open(filename.c_str());
  
  if (! init_db("fastq.db")) {
    cout << "pip: error initializing database" << endl;
    return 1;
  }
    
  FASTQSequence x;
  int id = 0;
  
  char insert_cmnd[] = "INSERT INTO reads VALUES (?1,?2,?3,?4)";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, insert_cmnd, strlen(insert_cmnd), &stmt, NULL);
  
  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sql_error_msg);
  
  while (x.fetch(readfile)) {
      sqlite3_bind_int(stmt, 1, id++);
      sqlite3_bind_text(stmt, 2, x.defline().c_str(), x.defline().length(), SQLITE_STATIC);
      sqlite3_bind_text(stmt, 3, x.sequence().c_str(), x.sequence().length(), SQLITE_STATIC);
      sqlite3_bind_text(stmt, 4, x.quality().c_str(), x.quality().length(), SQLITE_STATIC);
      if (sqlite3_step(stmt) != SQLITE_DONE) {
          cout << "fail..." << endl;
      }
      sqlite3_reset(stmt);
  }
  
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sql_error_msg);
  
  sqlite3_close(db);
  return 0;
}

