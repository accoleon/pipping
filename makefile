# Makefile for pip (pipeline interface program)

#CXX = /usr/local/Cellar/gcc48/4.8.2/bin/g++-4.8
#CC = /usr/local/Cellar/gcc48/4.8.2/bin/gcc-4.8
CXXFLAGS += -O3 -Wall -Wextra  -std=c++11 -march=native
LDFLAGS +=  -ldl -lpthread
SQLITE_THREADSAFE = 0

all: pip

pip: pip.o newpack.o commands.o stream_trimmomatic.o sqlite3.o
	$(CXX) -o $@ $^ -L/usr/local/Cellar/boost/1.55.0_1/lib -L/usr/local/packages/boost/1.55.0/lib -lboost_program_options
	
insert_accelerator: insert_accelerator.o newpack.o commands.o sqlite3.o
	$(CXX) -o $@ $^ -L/usr/local/Cellar/boost/1.55.0_1/lib -L/usr/local/packages/boost/1.55.0/lib

gref:	gref.o FASTQSequence.o newpack.o sqlite3.o
	$(CXX) -o $@ $^ -L/usr/local/packages/boost/1.55.0_1/lib -lboost_program_options
	
sqlite3.o: sqlite3.c
	/usr/local/Cellar/gcc48/4.8.2/bin/gcc-4.8 -O3 -Wa,-q -ftree-vectorizer-verbose=2 -march=native -o $(@) -c $^
	
tests: testPack testFASTQ
	
testPack: FASTQSequence.o newpack.o
	${LINK.C} testPack.C -o $(@) $?
	
testFASTQ: FASTQSequence.o 
	${LINK.C} testFASTQ.C -o $(@) $?

testFastRead: testFastRead.o

testRawWrite: testRawWrite.o newpack.o
	$(CXX) -o $@ $^ -L/usr/local/Cellar/boost/1.55.0/lib

clean:
	/bin/rm -f *~ *.o dump.fastq fastq.db

realclean:      clean
	/bin/rm -f imap
