# Find the start of the primer at the front of each read, save the
# location in the 'upstream' table

from sqlite3 import *
from datetime import *
from sys import argv
import re

iupac_regexp = {
	'A' : r'A',	 
	'C' : r'C',	 
	'G' : r'G',	 
	'T' : r'T',
	'R' : r'[AG]', 
	'Y' : r'[CT]',
	'S' : r'[GC]',
	'W' : r'[AT]',
	'K' : r'[GT]',
	'M' : r'[AC]',
	'B' : r'[CGT]',
	'D' : r'[AGT]',
	'H' : r'[ACT]',
	'V' : r'[ACG]',
	'N' : r'[ACGT]'
}

primer_patterns = []

def regexp_from_primers(db):
	"""
	Create regular expressions for the primers, save in a global variable named primer_patterns
	where they can be used by the primer_loc function.
	"""
	for row in db.execute("SELECT sequence FROM primers"):
		e = ''
		for ch in row[0]:
			e += iupac_regexp[ch]
		primer_patterns.append(re.compile(e))

drop_table = "DROP TABLE IF EXISTS upstream"

create_table = """CREATE TABLE upstream AS SELECT id, 
	find_primer(sequence) AS primer_start
	FROM reads"""
	
def find_primer(s):
	"Return the location of primers in sequence s, or None if the primer is not found"
	m = primer_patterns[0].search(s)
	return m.start() if m is not None else None

def make_primer_table(dbname):
	"Top level function:  erase any previous table, execute the CREATE TABLE command."
	db = connect(dbname)
	regexp_from_primers(db)
	db.create_function("find_primer", 1, find_primer)
	query = db.cursor()
	
	query.execute(drop_table)
	query.execute(create_table)
	
	db.commit()
		
# API:	find_primers dbname

if __name__ == "__main__":
	dbname = 'fastq.db' if len(argv) == 1 else argv[1]
	make_primer_table(dbname)
