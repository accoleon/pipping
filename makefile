# Makefile for pip (pipeline interface program)

CXX = g++
CXXFLAGS += -O3 -Wall -std=c++11
LDFLAGS +=

all: pip

pip: pip.o Piper.o newpack.o FASTQSequence.o commands.o stream_trimmomatic.o
	${LINK.C} -o $@ $^ -lsqlite3 -lboost_program_options -lconfig++

gref:	gref.o FASTQSequence.o
	${LINK.C} -o $@ $^
	
tests: testPack testFASTQ
	
testPack: FASTQSequence.o newpack.o
	${LINK.C} testPack.C -o $(@) $?
	
testFASTQ: FASTQSequence.o 
	${LINK.C} testFASTQ.C -o $(@) $?
	
testReadFast: testReadFast.o
	${LINK.C} -o $(@) $?

testParse: testParse.o
	${LINK.C} -o $(@) $?

clean:
	/bin/rm -f *~ *.o dump.fastq fastq.db

realclean:      clean
	/bin/rm -f imap
