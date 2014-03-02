// testPack.C
// Unit Test for packing functions
//
// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#include <fstream>

#include "FASTQSequence.h"
#include "newpack.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;

/*
TEST_CASE("Test packing/unpacking with 4 letter inputs", "[pack]") {
	string in1("ACGT");
	vector<bool> out1 = packSequence(in1);
	CHECK(out1.size() == 8);

	SECTION("output from unpack is identical to input value") {
		string unp1(unpackSequence(out1));
		CHECK(unp1 == in1);
	}
}

TEST_CASE("Test packing/unpacking with long inputs", "[pack]") {
	string in1("AGTTATAACTAGATCGGAAGAGCGGTTCAGCAGGAATGCCGAGACCGATCTCGTATGCCGTCTTCTGCTTGAAAAAAAAAAAAAAACAATATACCACACGTGAACGCTACACTCGCGTGTGAGACCGACACTGCAGTGCGTGCGACGAGCC");
	vector<bool> out1 = packSequence(in1);
	
	SECTION("output from unpack is identical to input value") {
		string unp1(unpackSequence(out1));
		CHECK(unp1 == in1);
	}
}

TEST_CASE("Test packing/unpacking with FASTQ inputs", "[pack]") {
	std::ifstream readfile;
	readfile.open("data/reads.fastq");
	FASTQSequence x;
	vector<bool> t1;
	
	while (x.fetch(readfile)) {
		t1 = packSequence(x.sequence());
		CHECK(unpackSequence(t1) == x.sequence());
	}
}

TEST_CASE("Test quality packing/unpacking with 4 letter inputs", "[pack]") {
	string in1("GHIJ");
	vector<bool> out1 = packQuality(Lossless, in1);

	SECTION("output from unpackQuality is identical to input value") {
		string unp1(unpackQuality(Lossless, out1));
		CHECK(unp1 == in1);
	}
}

TEST_CASE("Test packQuality/unpackQuality with FASTQ inputs", "[pack]") {
	std::ifstream readfile;
	readfile.open("data/reads.fastq");
	FASTQSequence x;
	vector<bool> t1;
	
	while (x.fetch(readfile)) {
		t1 = packQuality(Q15, x.quality());
		CHECK(unpackQuality(Q15, t1) == x.quality());
	}
}

TEST_CASE("Test FASTQSequence packing/unpacking") {
	std::ifstream readfile;
	readfile.open("data/reads.fastq");
	FASTQSequence x;
	FASTQSequence c; // represents compressed sequences
	vector<bool> t1;
	
	while (x.fetch(readfile)) {
		c.unpack(x.compressed_sequence(), x.compressed_quality());
		CHECK(c.sequence() == x.sequence());
		CHECK(c.size() == x.size());
		CHECK(c.quality() == x.quality());
		c.clear();
		x.clear();
	}
}
*/

using namespace pip::pack;

TEST_CASE("Testing find_quality_format 1 fastq sequence") {
	std::ifstream readfile("data/1.fastq");
	FASTQSequence x;
	x.fetch(readfile);
	auto a = find_quality_format(x.quality());
	CHECK(a == Illumina18);
}

TEST_CASE("Testing find_quality_format 3 fastq sequence") {
	std::ifstream readfile("data/first3.fastq");
	FASTQSequence x;
	
	while (x.fetch(readfile)) {
		auto a = find_quality_format(x.quality());
		CHECK(a == Illumina18);
	}
}

TEST_CASE("Testing find_quality_format 100 fastq sequence") {
	std::ifstream readfile("data/reads.fastq");
	FASTQSequence x;
	
	while (x.fetch(readfile)) {
		auto a = find_quality_format(x.quality());
		CHECK(a == Illumina18);
	}
}

TEST_CASE("Testing find_quality_format 100k fastq sequence") {
	std::ifstream readfile("data/kt1_100K.1.fastq");
	FASTQSequence x;
	
	while (x.fetch(readfile)) {
		auto a = find_quality_format(x.quality());
		CHECK(a == Illumina18);
	}
}

TEST_CASE("Testing pack sequence 1 fastq") {
	std::ifstream readfile("data/1.fastq");
	FASTQSequence x;
	x.fetch(readfile);
	
	Pack packed(x.sequence(),x.quality());
	string unpackedSequence, unpackedQuality;
	unpack(packed,unpackedSequence,unpackedQuality);
	CHECK(unpackedSequence == x.sequence());
	CHECK(unpackedQuality == x.quality());
}

TEST_CASE("Testing pack sequence 100 fastq") {
	std::ifstream readfile("data/reads.fastq");
	FASTQSequence x;
	
	while (x.fetch(readfile)) {
		Pack packed(x.sequence(),x.quality());
		string unpackedSequence, unpackedQuality;
		unpack(packed,unpackedSequence,unpackedQuality);
		CHECK(unpackedSequence == x.sequence());
		CHECK(unpackedQuality == x.quality());
	}
}

TEST_CASE("Testing pack sequence 100k fastq") {
	std::ifstream readfile("data/kt1_100K.1.fastq");
	FASTQSequence x;
	
	while (x.fetch(readfile)) {
		Pack packed(x.sequence(),x.quality());
		string unpackedSequence, unpackedQuality;
		unpack(packed,unpackedSequence,unpackedQuality);
		CHECK(unpackedSequence == x.sequence());
		CHECK(unpackedQuality == x.quality());
	}
}

TEST_CASE("Testing losslesspack sequence 100 fastq") {
	std::ifstream readfile("data/reads.fastq");
	FASTQSequence x;
	
	while (x.fetch(readfile)) {
		Pack packed(x.sequence(),x.quality(),1);
		string unpackedSequence, unpackedQuality;
		losslessUnpack(packed,unpackedSequence,unpackedQuality);
		CHECK(unpackedSequence == x.sequence());
		CHECK(unpackedQuality == x.quality());
	}
}

TEST_CASE("Testing losslesspack sequence 100k fastq") {
	std::ifstream readfile("data/kt1_100K.1.fastq");
	FASTQSequence x;
	
	while (x.fetch(readfile)) {
		Pack packed(x.sequence(),x.quality(),1);
		string unpackedSequence, unpackedQuality;
		losslessUnpack(packed,unpackedSequence,unpackedQuality);
		CHECK(unpackedSequence == x.sequence());
		CHECK(unpackedQuality == x.quality());
	}
}
