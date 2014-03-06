// testReadFast.C
// Testing 100k read using entire chunk method
// Xu Junjie, Kevin
// University of Oregon
// 2014-03-02

#include <cstdio>
#include <fstream>
using std::ifstream;
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// Fastest
TEST_CASE("Testing speed of fread in whole chunks resize") {
	std::FILE *fp = std::fopen("data/anna_500K.1.fastq", "rb");
	if (fp) {
		string contents;
		std::fseek(fp, 0, SEEK_END);
		contents.resize(std::ftell(fp));
		std::rewind(fp);
		std::fread(&contents[0],1,contents.size(),fp);
		std::fclose(fp);
		//cout << contents << endl;
	}
}