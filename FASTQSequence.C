// Implementation of the FASTQSequence class.


#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;

#include "pack.h"
#include "FASTQSequence.h"


// The constructor just allocates space for all the instance variables (relying
// on the default string constructor to create empty strings).

FASTQSequence::FASTQSequence() 
{ 
};

int FASTQSequence::size() const 
{
  return _seq.length();
}

// Pass a reference to an input stream to fill in the parts of a sequence, assuming
// the sequence is on exactly four lines in the stream.

bool FASTQSequence::fetch(istream &str)
{  
  getline(str, _def1);
  if (str.eof())
    return false;
  getline(str, _seq);
  getline(str, _def2);
  getline(str, _qual);
  
  return true;
}

const string &FASTQSequence::defline() 
{
  return _def1;
}

const string &FASTQSequence::sequence() 
{
	if (_seq.empty() && !_cseq.empty() && !quality().empty()) {
		_seq = pipping::unpackSequence(_cseq);
		pipping::repairSequence(_seq, quality());
	}
  return _seq;
}

const vector<bool> &FASTQSequence::compressed_sequence() {
	if (_cseq.empty() && !_seq.empty()) {
		_cseq = pipping::packSequence(_seq);
	}
	return _cseq;
}

const string &FASTQSequence::quality() 
{
	if (_qual.empty() && !_cqual.empty()) {
		_qual = pipping::unpackQuality(_cqual);
	}
  return _qual;
}

const vector<bool> &FASTQSequence::compressed_quality() {
	if (_cqual.empty() && !_qual.empty()) {
		_cqual = pipping::packQuality(_qual);
	}
	return _cqual;
}

void FASTQSequence::parse_defline()
{
    
}

void FASTQSequence::fill(const vector<bool> &cseq, const vector<bool> &cqual) {
	_cseq = cseq;
	_cqual = cqual;
}


// Using the << operator to print a FASTQSequence object generates three lines, 
// with newlines at the end of each line.

ostream& operator<<(ostream &str, const FASTQSequence &seq) {
  str << seq._def1 << endl;
  str << seq._seq << endl;
  str << seq._def2 << endl;
  str << seq._qual;
  return str;
}


