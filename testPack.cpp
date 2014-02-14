// testPack.C
// Unit Test for packing functions
//
// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#include "pack.h"

using namespace pipping;

int main (int argc, char const *argv[])
{
	string in1 = "AAAA";
	vector<bool> out1 = pack(LoopIt, in1);
	for (vector<bool>::const_iterator it = out1.begin(); it != out1.end(); it++) {
		cout << *it;
	}
	
	return 0;
}