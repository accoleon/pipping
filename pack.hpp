// pack.h 
// Bit Packing header

// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#ifndef PACK_H
#define PACK_H

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;

namespace pipping {
	enum PackingStyle {
		LoopIt // Looping iterator style
	};
	static vector<bool> pack(PackingStyle style, string data); // Packs a string
	static vector<bool> packLoopIt(string data); // Packs a string using a Looping iterator style
}

#endif
