// testPack.C
// Unit Test for packing functions
//
// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

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
	vector<bool> out1 = pack(For, in1);
	vector<bool> test1(8);
	// 00011110
	test1[3] = true;
	test1[4] = true;
	test1[5] = true;
	test1[6] = true;
	REQUIRE(out1.size() == 8);
	
	SECTION("output from pack is expected value") {
		REQUIRE(test1 == out1);
	}
	SECTION("output from unpack is identical to input value") {
		string unp1(unpack(out1));
		REQUIRE(unp1 == in1);
	}
}