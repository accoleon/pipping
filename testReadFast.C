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

// Next fastest
TEST_CASE("Testing speed of istream read in whole chunk resize") {
	ifstream in("data/kt1_100K.1.fastq", std::ios::in | std::ios::binary);
	if (in) {
		string contents;
		in.seekg(0,std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0,std::ios::beg);
		in.read(&contents[0],contents.size());
		in.close();
		//cout << contents << endl;
	}
}
// Fastest
TEST_CASE("Testing speed of fread in whole chunks resize") {
	std::FILE *fp = std::fopen("data/kt1_100K.2.fastq", "rb");
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