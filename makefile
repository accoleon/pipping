# Makefile for pip (pipeline interface program)

CXX = g++
CXXFLAGS += -O3 -Wall -std=c++11
LDFLAGS +=

pip:	pip.o FASTQSequence.o newpack.o commands.o
	${LINK.C} -o pip $? -lsqlite3 -lboost_program_options

gref:	gref.o FASTQSequence.o
	${LINK.C} -o gref gref.o FASTQSequence.o

#pip.o:	pip.C
#gref.o:	gref.C
#FASTQSequence.o:	FASTQSequence.C
	
tests: testPack testFASTQ
	
testPack: FASTQSequence.o newpack.o
	${LINK.C} testPack.C -o $(@) $?
	
testFASTQ: FASTQSequence.o 
	${LINK.C} testFASTQ.C -o $(@) $?
	
testReadFast: testReadFast.o
	${LINK.C} -o $(@) $?

clean:
	/bin/rm -f *~ *.o

realclean:      clean
	/bin/rm -f imap
