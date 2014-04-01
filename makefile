# Makefile for pip (pipeline interface program)

CXX = g++
CC = g++
CXXFLAGS += -O3 -Wall -Wextra -pedantic-errors -std=c++11 -march=native
LDFLAGS +=  -ldl -lpthread

all: pip

pip: pip.o newpack.o commands.o stream_trimmomatic.o sqlite3.o
	${LINK.C} -o $@ $^ -static -lboost_program_options

gref:	gref.o FASTQSequence.o newpack.o sqlite3.o
	${LINK.C} -o $@ $^ -static -lboost_program_options
	
sqlite3.o: sqlite3.c
	gcc -O3 -march=native -o $(@) -c $^
	
tests: testPack testFASTQ
	
testPack: FASTQSequence.o newpack.o
	${LINK.C} testPack.C -o $(@) $?
	
testFASTQ: FASTQSequence.o 
	${LINK.C} testFASTQ.C -o $(@) $?

testFastRead: testFastRead.o

clean:
	/bin/rm -f *~ *.o dump.fastq fastq.db

realclean:      clean
	/bin/rm -f imap
