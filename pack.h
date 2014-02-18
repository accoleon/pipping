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
	std::vector<bool> packSequence(const std::string &data); // Packs a sequence
	std::string unpackSequence(const std::vector<bool> &data);
	std::vector<bool> packQuality(const std::string &data); // Packs a quality line
	std::string unpackQuality(const std::vector<bool> &data);
	void repairSequence(std::string &sequence, const std::string &quality);
}

#endif
