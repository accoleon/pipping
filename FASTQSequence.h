// -*- C++ -*-

// FASTQ sequence objects

// John Conery
// Oct 2013

// Intended use:  instantiate one object, call its 'fetch' method to read
// the sequence attributes from a file.

// Illumina FASTQ reference: http://support.illumina.com/help/SequencingAnalysisWorkflow/Content/Vault/Informatics/Sequencing_Analysis/CASAVA/swSEQ_mCA_FASTQFiles.htm

#ifndef _fastqseq_h
#define _fastqseq_h

#include <iostream>
using std::istream;
using std::ostream;

#include <string>
using std::string;

#include <vector>
#include <regex>

class FASTQSequence {
  
public:
  FASTQSequence();
  bool fetch(istream &);
  friend ostream& operator<<(ostream &, const FASTQSequence &);  
  int size() const;
  const string &defline();
  const string &sequence() const;
  const string &quality() const;
  const std::vector<bool> &compressed_sequence();
  const std::vector<bool> &compressed_quality();
  void parse_defline();
  void unpack(const std::vector<bool> &cseq, const std::vector<bool> &cqual);
  void clear();
  const string& instrument_id();
  int run_id();
  int flow_cell_lane();
  const string& flow_cell_id();
  int tile();
	int x();
	int y();
	int pair();
  int control_bits();
  bool filter();
	const string& index();
private:
  static std::regex const rx; // regular expression for parsing deflines
  string _def1;
  string _seq;
  string _def2;
  string _qual;
  std::vector<bool> _cseq; // compressed sequence
  std::vector<bool> _cqual; // compressed qscore
  string _instrument_id;	// Instrument ID
  int _run_id;			// Run number on instrument
  string _flow_cell_id; // Flowcell ID
  int _flow_cell_lane;	// Lane number
  int _tile;			// Tile number
  int _x_coord;			// X coordinate of cluster
  int _y_coord;			// Y coordinate of cluster
  int _pair_end;		// Read number, 1 can be single read or read 2 of paired end
  bool _filtered;		// Y if the read is filtered, N otherwise
  int _control_bits;	// 0 when no control bits are on, otherwise an even number
  string _index;		// Index sequence5
};

ostream& operator<<(ostream &str, const FASTQSequence &seq);

#endif
