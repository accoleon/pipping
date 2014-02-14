// pack.cpp
// Bit Packing functions

// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#include "pack.h"

using std::string;
using std::vector;
using std::bitset;
using std::cout;
using std::endl;


 vector<bool> pipping::pack(PackingStyle style, const string &data) {
	if (!data.empty()) {
		switch(style) {
			case For:
				return packFor(data);
				break;
		}
	}
	vector<bool> packed;
	return packed;
}

vector<bool> pipping::packFor(const string &data) {
	vector<bool> packed;
	packed.reserve(data.length()<<1); // 2x string length
	//unsigned char x;
	bitset<2> x;
	for(int i = 0; i < data.length(); i++) {
		x = data[i]>>1;
		//cout << x << endl;
		packed.push_back(x[1]);
		packed.push_back(x[0]);
		//packed.push_back(bitset<2>(data[i]>>1));
	}
	
	return packed;
}

string pipping::unpack(const vector<bool> &data) {
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