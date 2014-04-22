#! /usr/bin/env python3

# Import reads into a SQLite database for a PIP/NGS analysis pipeline.

# John Conery / Kevin Xu Junjie
# University of Oregon
# 2014-04-15

# Read a set of fastq files, extract the index from the defline, pack
# the sequence and quality chars into a "BLOB"

import sqlite3
import argparse
import os.path

from FASTQ import *

fetch_barcodes = 'SELECT barcode_id, experiment, sequence FROM barcodes'

def barcode_list(db, args):
	"Return a list of barcodes that identify the files used in this experiment."
	sql = fetch_barcodes
	if isinstance(args.experiment, str):
		sql += ' WHERE experiment = "{}"'.format(args.experiment)
	return db.execute(sql).fetchall()
	
# find_read_tables = 'SELECT name FROM sqlite_master WHERE type = "table" AND name LIKE "R%"'
# create_read_table = 'CREATE TABLE R{} ( read_id INTEGER PRIMARY KEY AUTOINCREMENT, barcode_id INTEGER NOT NULL REFERENCES barcodes, read BLOB )'
# find_experiment = 'SELECT barcode_id FROM R1 WHERE barcode_id = ? LIMIT 1'
# delete_experiment = 'DELETE FROM R{} WHERE barcode_id = ?'

find_read_table = 'SELECT name FROM sqlite_master WHERE type = "table" AND name = "reads"'
create_read_table = 'CREATE TABLE reads ( read_id INTEGER PRIMARY KEY AUTOINCREMENT, barcode_id INTEGER NOT NULL REFERENCES barcodes, read BLOB )'
find_experiment = 'SELECT barcode_id FROM reads WHERE barcode_id = ? LIMIT 1'
delete_experiment = 'DELETE FROM reads WHERE barcode_id = ?'
	
def prepare_tables(db, barcodes, args):
	"""
	Return True if the database is initialized and ready to accept a new set of reads.
	If the reads table does not exist yet create it and return True.  If the 
	barcodes have already been loaded return False unless --force was specified on the
	command line.  If --force was specified erase the old data and return True.
	"""
	if not db.execute(find_read_table).fetchall():
		db.execute(create_read_table)
		return True
	already_loaded = []
	for bc_id, expt, code in barcodes:
		if db.execute(find_experiment, (bc_id,)).fetchall():
			already_loaded.append(bc_id)
	if already_loaded and not args.force:
		return False
	for bc_id in already_loaded:		# if none previousy loaded this is a NOP
		db.execute(delete_experiment, (bc_id,))	
	return True	 
	
insert_blob = 'INSERT INTO reads (barcode_id, read) VALUES (?, ?)'
	
def load_sequences(db, fn1, fn2, bc_id):
	"""
	Read the sequences from a pair of files (R1 and R2) for a specified barcode.  Create
	a FASTQ object for each read, compress it, and insert the compressed form into the 
	reads table.
	
	TBD: if the --accelerate option is specified on the command line use the C++ app
	to fill the table.
	"""
	file1 = open(fn1)
	file2 = open(fn2)
	seq1 = FASTQ.read(file1)
	seq2 = FASTQ.read(file2)
	while seq1 is not None and seq2 is not None:
		for seq in [seq1, seq2]:
			defline = seq.defline()
			if defline.find('N', defline.index(' ')) > 0:
				seq.pack()
				blob = seq.blob()
			else:
				blob = None
			db.execute(insert_blob, (bc_id, blob))
		seq1 = FASTQ.read(file1)
		seq2 = FASTQ.read(file2)
	
file_pattern = 'L1_{exp}_{code1}-{code2}_L001_R{pair}_001.fastq'
	
def import_files(db, codelist, args):
	"""
	Use the barcodes to create the names of the files to load, pass the file names to
	load_sequences to read the sequences and save them in the database.
	"""
	for bc_id, expt, code in codelist:
		fn1 = os.path.join(args.directory, file_pattern.format(exp = expt, code1 = code[:7], code2 = code[7:], pair = 1))
		fn2 = os.path.join(args.directory, file_pattern.format(exp = expt, code1 = code[:7], code2 = code[7:], pair = 2))
		load_sequences(db, fn1, fn2, bc_id)
		db.execute("INSERT INTO log VALUES (?, DATETIME('NOW'))", ('Imported {}, {}'.format(fn1,fn2), ))
		
### 
# Set up command line arguments

def init_api():
	parser = argparse.ArgumentParser(
		description="""Import FASTQ files into a SQLite3 database for a PIP/NGS analysis pipeline.  The script
		constructs names of files to import based on barcodes stored in the database (see db_setup.py).
		""",
	)
	parser.add_argument('dbname', help='the name of the SQLite database file')
	parser.add_argument('-d', '--directory', metavar='dir', required=True, help='name of a directory containing FASTQ files')
	parser.add_argument('-e', '--experiment', metavar='id', required=False, help='import files for experiment id')
	parser.add_argument('-a', '--all', action='store_true', help='import files for all experiments')
	parser.add_argument('-n', '--noimport', action='store_true', help="verify files but don't import")
	parser.add_argument('-f', '--force', action='store_true', help='re-initialize an existing database')
	return parser.parse_args()
	
###
# Parse the command line arguments, call the top level function...
	
if __name__ == "__main__":
	args = init_api()
	if not (args.all or args.experiment):
		argparse.ArgumentParser.exit(1, 'Specify an experiment with -e ID or import all with --all')

	db = sqlite3.connect(args.dbname)
	
	barcodes = barcode_list(db, args)
	
	if not prepare_tables(db, barcodes, args):
		argparse.ArgumentParser.exit(1, 'These sequences were loaded previously; use --force if you want to replace them')
		
	import_files(db, barcodes, args)
	
	db.commit()
