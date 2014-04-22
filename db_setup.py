#! /usr/bin/env python3

# Create a new SQLite database for a PIP/NGS analysis pipeline.

# John Conery / Kevin Xu Junjie
# University of Oregon
# 2014-04-15

# Usage:
#
#    db_setup.py [--force] dbname --info experiment
#
# Create a new database file with the specified name.  The --info argument
# is the name of a file with barcodes, index sequences, and other information 
# about the sequences.  By default the script checks to make sure the database 
# does not exist already; use --force to re-initialize an existing database.

import sqlite3
import argparse
import os.path

# Strings in this list are names of items from the info file; plural forms
# will be names of tables in the database

items = ['primer', 'offset', 'barcode']

# SQL command templates

create_table = {
	'primer'  : "CREATE TABLE primers ( pair_end INTEGER, sequence TEXT )" ,
	'offset'  : "CREATE TABLE offsets ( pair_end INTEGER, sequence TEXT )" ,
	'barcode' : "CREATE TABLE barcodes ( barcode_id INTEGER PRIMARY KEY AUTOINCREMENT, experiment TEXT, sequence TEXT )" 
}
drop_table = "DROP TABLE IF EXISTS {}s"
insert = "INSERT INTO {}s VALUES (?, ?)"

###

def make_table(db, tbl):
	"Initialize a table -- delete it if it exists already"
	db.execute(drop_table.format(tbl))
	db.execute(create_table[tbl])
	
def init_log(db, msg):
	"Create the log table, insert initial log messages"
	db.execute("DROP TABLE IF EXISTS log")
	db.execute("CREATE TABLE log ( time timestamp, message text )")
	db.execute("INSERT INTO log VALUES ('Database created', DATETIME('NOW'))")
	if msg is not None:
		db.execute("INSERT INTO log VALUES (?, DATETIME('NOW'))", (msg, ))
	
def add_aux_info(filename, dbname, message):
	"Top level function: create the tables, read the file and populate the tables"
	db = sqlite3.connect(dbname)
	
	for tbl in items:
		make_table(db, tbl)
		
	for line in open(filename):
		tbl, loc, seq = line.strip().split()
		if tbl in ['primer', 'offset']:
			db.execute('INSERT INTO {}s VALUES (?, ?)'.format(tbl), (loc, seq))
		elif tbl == 'barcode':
			db.execute('INSERT INTO barcodes (experiment, sequence) VALUES (?, ?)', (loc, seq))
		else:
			print("Unknown item:", line.strip())
						
	init_log(db, message)
	
	db.commit()
		
### 
# Set up command line arguments

def init_api():
	parser = argparse.ArgumentParser(
		description="Initialize a SQLite3 database for a PIP/NGS analysis pipeline.",
		epilog="If --force is not specified the program will print a message and exit if the database exists already."
	)
	parser.add_argument('dbname', help='the name of the SQLite database file')
	parser.add_argument('-i', '--info', metavar='file', required=True, help='the name of a file with experiment information')
	parser.add_argument('-f', '--force', action='store_true', help='re-initialize an existing database')
	parser.add_argument('-m', '--message', metavar='text', help='initial log message')
	return parser.parse_args()
	
###
# Parse the command line arguments, call the top level function...
	
if __name__ == "__main__":
	args = init_api()
	if os.path.exists(args.dbname) and not args.force:
		argparse.ArgumentParser.exit(1, 'Found existing database with that name; use --force to reinitialize')
	add_aux_info(args.info, args.dbname, args.message)
