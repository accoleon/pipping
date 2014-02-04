// Implementation of the FASTQSequence class.

#include "FASTQSequence.h"

#include <iostream>
using std::cout;
using std::endl;

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

string &FASTQSequence::defline() 
{
  return _def1;
}

string &FASTQSequence::sequence() 
{
  return _seq;
}

string &FASTQSequence::quality() 
{
  return _qual;
}

void FASTQSequence::parse_defline()
{
    
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


