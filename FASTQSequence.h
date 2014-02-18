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

#include <vector>

class FASTQSequence {
  
public:
  FASTQSequence();
  bool fetch(istream &);
  friend ostream& operator<<(ostream &, const FASTQSequence &);  
  int size() const;
  const string &defline();
  const string &sequence();
  const string &quality();
  const std::vector<bool> &compressed_sequence();
  const std::vector<bool> &compressed_quality();
  void parse_defline();
  void fill(const std::vector<bool> &cseq, const std::vector<bool> &cqual);
private:
  string _def1;
  string _seq;
  string _def2;
  string _qual;
  std::vector<bool> _cseq; // compressed sequence
  std::vector<bool> _cqual; // compressed qscore
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
