// pack.cpp
// Bit Packing functions

// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

#include "pack.h"

using std::string;
using std::vector;
using std::bitset;
using std::cout;
using std::endl;


vector<bool> pipping::packSequence(const string &data) {
 	vector<bool> packed;
 	packed.reserve(data.length()<<1); // 2x string length
 	//unsigned char x;
 	bitset<2> x;
 	for(int i = 0; i < data.length(); ++i) {
 		x = data[i]>>1;
 		//cout << x << endl;
 		packed.push_back(x[1]);
 		packed.push_back(x[0]);
 		//packed.push_back(bitset<2>(data[i]>>1));
 	}
	
 	return packed;
}

string pipping::unpackSequence(const vector<bool> &data) {
	string unpacked;
	unpacked.reserve(data.size()>>1); // 0.5x data length
	bitset<2> x;
	for(int i = 0; i < data.size(); i += 2) {
		x[1] = data[i];
		x[0] = data[i + 1];
		//cout << x.to_string();
		if (!x[1] && !x[0]) { // A
			unpacked.append("A");
		} else if (!x[1] && x[0]) { // C
			unpacked.append("C");
		} else if (x[1] && x[0]) { // G
			unpacked.append("G");
		} else { // T 
			unpacked.append("T");
		}
	}
	
	return unpacked;
}

vector<bool> pipping::packQuality(const string &data) {
	// pack quality scores into 4 bit numbers
	vector<bool> packed;
	packed.reserve(data.length()<<2); // 4x string length
	int temp;
	for (int i = 0; i < data.length(); ++i) {
		temp = data[i] - 59; // magic number, from ASCII value (J is 74)
		if (temp < 0) { // quality value below 1
			temp = 0; // cap it at 0 quality
		}
		bitset<4> x(temp);
		
		// push_back bits from most significant bits first
		packed.push_back(x[3]);
		packed.push_back(x[2]);
		packed.push_back(x[1]);
		packed.push_back(x[0]);
		//cout << x << endl;
	}
	
	return packed;
}

string pipping::unpackQuality(const vector<bool> &data) {
	string unpacked;
	char c;
	unpacked.reserve(data.size()>>2); // 1/4 data length
	for(int i = 0; i < data.size(); ++i) {
		c = c << 1 | data[i];
		if ((i + 1) % 4 == 0) {
			if (c == 0) {
				unpacked += '#';
			} else {
				c += 59;
				unpacked += c;
				c = 0;
			}
		}
	}

	return unpacked;
}

void pipping::repairSequence(std::string &sequence, const std::string &quality) {
	if (sequence.length() == quality.length()) { // both strings have to be the same length, otherwise it is an error
		for (int i = 0; i < quality.length(); ++i) {
			if ((int)quality[i] < 59) { // magic number - ASCII value that represents lower bound of quality scores we want to keep (15)
				sequence.replace(i, 1, "N");
			}
		}
	}	
}
