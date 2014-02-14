// -*- C++ -*-

// FASTQ sequence objects

// John Conery
// Oct 2013

// Intended use:  instantiate one object, call its 'fetch' method to read
// the sequence attributes from a file.

#ifndef _fastqseq_h
#define _fastqseq_h

#include <iostream>
using std::istream;
using std::ostream;

#include <string>
using std::string;

class FASTQSequence {
  
public:
  FASTQSequence();
  bool fetch(istream &);
  friend ostream& operator<<(ostream &, const FASTQSequence &);  
  int size() const;
  string &defline();
  string &sequence();
  string &quality();
  void parse_defline();
private:
  string _def1;
  string _seq;
  string _def2;
  string _qual;
  char *_instrument;
  char *_run_id;
  char *_flow_cell_id;
  int _tile_number;
  int _x_coord;
  int _y_coord;
  char _pair_end;
  char _filtered;
  int _control_bits;
};

ostream& operator<<(ostream &str, const FASTQSequence &seq);

#endif
