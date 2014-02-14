// pack.hpp
// Bit Packing header

// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#ifndef PACK_H
#define PACK_H

#include <iostream>
#include <string>
#include <vector>

namespace pipping {
	enum PackingStyle {
		For // Looping iterator style
	};
	std::vector<bool> pack(PackingStyle style, const std::string &data); // Packs a string
	std::vector<bool> packFor(const std::string &data); // Packs a string using a Looping iterator style
	std::string unpack(const std::vector<bool> &data);
}

#endif
