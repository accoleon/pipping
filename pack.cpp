// Bit Packing functions

// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#include "pack.h"

namespace pipping {
	vector<bool> pack(PackingStyle style, string data) {
		if (!data.empty()) {
			switch(style) {
				case LoopIt:
					return packLoopIt(data);
					break;
			}
		}
	}
	
	vector<bool> packLoopIt(string data) {
		vector<bool> *packed = new vector<bool>();
		string::iterator it;
		for(it = data.begin(); it < data.end(); it++)
		{
			cout << *it << endl;
		}
	}
}