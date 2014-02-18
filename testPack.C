// testPack.C
// Unit Test for packing functions
//
// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#include <fstream>

#include "FASTQSequence.h"
#include "pack.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;

using namespace pipping;

TEST_CASE("Test packing/unpacking with 4 letter inputs", "[pack]") {
	string in1("ACGT");
	vector<bool> out1 = packSequence(in1);
	REQUIRE(out1.size() == 8);

	SECTION("output from unpack is identical to input value") {
		string unp1(unpackSequence(out1));
		REQUIRE(unp1 == in1);
	}
}

TEST_CASE("Test packing/unpacking with long inputs", "[pack]") {
	string in1("AGTTATAACTAGATCGGAAGAGCGGTTCAGCAGGAATGCCGAGACCGATCTCGTATGCCGTCTTCTGCTTGAAAAAAAAAAAAAAACAATATACCACACGTGAACGCTACACTCGCGTGTGAGACCGACACTGCAGTGCGTGCGACGAGCC");
	vector<bool> out1 = packSequence(in1);
	
	SECTION("output from unpack is identical to input value") {
		string unp1(unpackSequence(out1));
		REQUIRE(unp1 == in1);
	}
}

TEST_CASE("Test packing/unpacking with FASTQ inputs", "[pack]") {
	std::ifstream readfile;
	readfile.open("data/reads.fastq");
	FASTQSequence x;
	vector<bool> t1;
	
	while (x.fetch(readfile)) {
		t1 = packSequence(x.sequence());
		REQUIRE(unpackSequence(t1) == x.sequence());
	}
}

TEST_CASE("Test quality packing/unpacking with 4 letter inputs", "[pack]") {
	string in1("GHIJ");
	vector<bool> out1 = packQuality(in1);

	SECTION("output from unpackQuality is identical to input value") {
		string unp1(unpackQuality(out1));
		REQUIRE(unp1 == in1);
	}
}

TEST_CASE("Test packQuality/unpackQuality with FASTQ inputs", "[pack]") {
	std::ifstream readfile;
	readfile.open("data/reads.fastq");
	FASTQSequence x;
	vector<bool> t1;
	
	while (x.fetch(readfile)) {
		t1 = packQuality(x.quality());
		REQUIRE(unpackQuality(t1) == x.quality());
	}
}

TEST_CASE("Test FASTQSequence packing/unpacking") {
	std::ifstream readfile;
	readfile.open("data/reads.fastq");
	FASTQSequence x;
	FASTQSequence c; // represents compressed sequences
	vector<bool> t1;
	
	while (x.fetch(readfile)) {
		c.fill(x.compressed_sequence(), x.compressed_quality());
		REQUIRE(c.sequence() == x.sequence());
		REQUIRE(c.size() == x.size());
		REQUIRE(c.quality() == x.quality());
	}
}