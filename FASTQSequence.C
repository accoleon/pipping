// Implementation of the FASTQSequence class.


#include <iostream>
using std::cout;
using std::endl;
#include <regex>
using std::regex;
#include <string>
using std::stoi;
#include <vector>
using std::vector;


#include "newpack.h"
using namespace pip;

#include "FASTQSequence.h"


// The constructor just allocates space for all the instance variables (relying
// on the default string constructor to create empty strings).

FASTQSequence::FASTQSequence() 
{ 
};




const int FASTQSequence::size() 
{
	if (_seq.empty() && !_cseq.empty()) { // lazy
		sequence();
	}
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
		// _seq = pack::unpackSequence(_cseq);
		// pack::repairSequence(_seq, quality());
	}
  return _seq;
}

const vector<bool> &FASTQSequence::compressed_sequence() {
	if (_cseq.empty() && !_seq.empty()) {
		//_cseq = pack::packSequence(_seq);
	}
	return _cseq;
}

const string &FASTQSequence::quality() 
{
	if (_qual.empty() && !_cqual.empty()) {
		// _qual = pack::unpackQuality(pack::Lossless, _cqual);
	}
  return _qual;
}

const vector<bool> &FASTQSequence::compressed_quality() {
	if (_cqual.empty() && !_qual.empty()) {
		// _cqual = pack::packQuality(pack::Lossless, _qual);
	}
	return _cqual;
}

const string &FASTQSequence::instrument_id() {
	return _instrument_id;
}

int FASTQSequence::run_id() {
	return _run_id;
}

const string &FASTQSequence::flow_cell_id() {
	return _flow_cell_id;
}

int FASTQSequence::flow_cell_lane() {
	return _flow_cell_lane;
}

int FASTQSequence::tile() {
	return _tile;
}

int FASTQSequence::x() {
	return _x_coord;
}

int FASTQSequence::y() {
	return _y_coord;
}

bool FASTQSequence::filter() {
	return _filtered;
}

const string &FASTQSequence::index() {
	return _index;
}

int FASTQSequence::control_bits() {
	return _control_bits;
}

int FASTQSequence::pair() {
	return _pair_end;
}

// @HWI-ST0747:277:D1M96ACXX:6:1101:1232:2090 1:N:0:
// attempting to match above into capture groups
regex const FASTQSequence::rx("^@(.+):(\\d+):(.+):(\\d+):(\\d+):(\\d+):(\\d+) (\\d):([YN]):(\\d+):(.*)");

void FASTQSequence::parse_defline()
{
	
	//cout << "defline: " << _def1 << endl;
	std::smatch results;
	std::regex_search(_def1, results, rx);
	_instrument_id = results[1];
    _run_id = stoi(results[2]);
    _flow_cell_id = results[3];
    _flow_cell_lane = stoi(results[4]);
    _tile = stoi(results[5]);
    _x_coord = stoi(results[6]);
    _y_coord = stoi(results[7]);
    _pair_end = stoi(results[8]);
    _filtered = results[9] == "Y" ? true : false;
    _control_bits = stoi(results[10]);
    _index = results[11];
}

void FASTQSequence::unpack(const vector<bool> &cseq, const vector<bool> &cqual) {
	_cseq = cseq;
	_cqual = cqual;
}

void FASTQSequence::clear() {
	_seq.clear();
	_qual.clear();
	_cseq.clear();
	_cqual.clear();
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


