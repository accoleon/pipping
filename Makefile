# Makefile for pip (pipeline interface program)

CXX = g++
CXXFLAGS += -O4
# LDFLAGS += 

pip:	pip.o FASTQSequence.o
	${LINK.C} -o pip pip.o FASTQSequence.o -lsqlite3

gref:	gref.o FASTQSequence.o
	${LINK.C} -o gref gref.o FASTQSequence.o

pip.o:	pip.C
gref.o:	gref.C
FASTQSequence.o:	FASTQSequence.C

clean:
	/bin/rm -f *~ *.o core

realclean:      clean
	/bin/rm -f imap
