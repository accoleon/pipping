// testFASTQ.C
// Unit test for FASTQSequence class

// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;


#include "FASTQSequence.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Test parsing defline in FASTQSequence class", "[FASTQ]") {
	std::ifstream readfile;
	readfile.open("data/reads.fastq");
	FASTQSequence x;
	vector<bool> t1;
	int counter = 0;
	while (x.fetch(readfile)) {
		++counter;
		x.parse_defline();
		CHECK(x.instrument_id() == "HWI-ST0747"); // All instruments should be HWI-ST0747
		CHECK(x.run_id() == 277); // All run ids should be 277
		CHECK(x.flow_cell_id() == "D1M96ACXX"); // All flowcell ids should be D1M96ACXX
		CHECK(x.flow_cell_lane() == 6); // All flowcell lanes should be 6
		CHECK(x.tile() == 1101); // All tiles should be 1101
		if (counter == 38)
			CHECK(x.read_filter() == "Y"); // this row has a Y
		else
			CHECK(x.read_filter() == "N"); // All read filters should be N
		CHECK(x.control_bits() == 0); // All control bits should be zero
		//cout << x << endl;
		//CHECK(unpackSequence(t1) == x.sequence());
	}
}