// Scan a FASTQ sequence file

// John Conery
// Oct 2013

// Use this program as a baseline to time how long it takes just to read a
// large FASTQ file.

#include <iostream>
using std::cin;
using std::cout;
using std::endl;
using std::ostream;

#include <fstream>
using std::ifstream;

#include <omp.h>

#include <string>
using std::string;

#include "FASTQSequence.h"

int main(int argc, char *argv[])
{  
	// Start time measurement
	double start = omp_get_wtime();
  string filename(argv[1]);
  ifstream readfile;
    
  readfile.open(filename.c_str());
    
  int count = 0;
  FASTQSequence x;
  
  while (x.fetch(readfile)) {
    // cout << x << endl;
    count += 1;
  }
  
  cout << count << endl;
  double end = omp_get_wtime();
	cout << "Total omp walltime: " << end-start << end;
  return 0;
}
