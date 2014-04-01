// newpack.C
// New packing functions
// Xu Junjie, Kevin
// University of Oregon
// Feb 2014

#include <bitset>
#include <iostream>
using std::cout;
using std::endl;
using std::ostream;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "newpack.h"

namespace pip {
	namespace pack {
		Pack::Pack(const string& sequence, const string& quality)
		{
			/*qualityFormat_ = find_quality_format(quality);
			sequenceLength_ = sequence.length();
			pack(sequence,quality);*/
			Pack(sequence,quality,1);
		}
		
		Pack::Pack(const string& sequence, const string& quality, int)
		{
			qualityFormat_ = find_quality_format(quality);
			sequenceLength_ = sequence.length();
			losslessPack(sequence,quality);
		}
		
		Pack::~Pack() {}
		
		unsigned int Pack::sequenceLength() const
		{
			return sequenceLength_;
		}
		
		QualityFormat Pack::qualityFormat() const
		{
			return qualityFormat_;
		}
		
		const unsigned char* Pack::rawData() const
		{
			return v.data();
		}
		
		void Pack::pack(const string& sequence, const string& quality)
		{
			if (sequenceLength_ != quality.length()) {
				// error
				return;
			}
			v.reserve(sequenceLength_);
			unsigned char c = 0; // temporary output character
			for (unsigned int i = 0; i<sequenceLength_; ++i) {
				// 0b00000110 = 6 bitmask, we want the 2 bits 3 from the right  
				c |= (sequence[i] & 6) << 5; // pack sequence character into the leftmost 2 bits
				switch (qualityFormat_) {
					case Illumina18:
						c |= quality[i]-Illu18QFLower;
						break;
					default:
						break;
				}
				v.push_back(c);
				c = 0;
			}
		}
		
		void Pack::losslessPack(const string& sequence, const string& quality)
		{
			if (sequenceLength_ != quality.length()) {
				// error
				return;
			}
			v.reserve(sequenceLength_);
			unsigned char c = 0; // temporary output character
			for (unsigned int i = 0; i<sequenceLength_; ++i) {
				// give different bases different int ranges
				// 'A': 0-49, 'C': 50-99, 'T': 100-149, 'G': 150-199, 'N': 200+
				switch (sequence[i]) { 
					case 'C':
						c = C;
						break;
					case 'T':
						c = T;
						break;
					case 'G':
						c = G;
						break;
					case 'N':
						c = N;
				}
				switch (qualityFormat_) {
					case Illumina18:
						c += quality[i]-Illu18QFLower;
						break;
					default:
						break;
				}
				v.push_back(c);
				c = 0;
			}
		}
		
		void losslessUnpack(const Pack& pack, string& outSequence, string& outQuality)
		{
			losslessUnpack(
				pack.rawData(),
				pack.sequenceLength(),
				pack.qualityFormat(),
				outSequence,
				outQuality);
		}
		
		void losslessUnpack(const unsigned char* data, int sequenceLength, QualityFormat qualityFormat, string &outSequence, string &outQuality)
		{
			outSequence.reserve(sequenceLength);
			outQuality.reserve(sequenceLength);
			for (auto i = 0; i < sequenceLength; ++i) {
				auto c = data[i];
				// Rebuilds nucleotides based on integer range
				// 'A': 0-49, 'C': 50-99, 'T': 100-149, 'G': 150-199, 'N': 200+
				if (c<C) {
					outSequence += "A";
				}
				else if (c<T) {
					outSequence += "C";
					c -= C;
				}
				else if (c<G) {
					outSequence += "T";
					c -= T;
				}
				else if (c<N) {
					outSequence += "G";
					c -= G;
				}
				else {
					outSequence += "N";
					c -= N;
				}
				switch (qualityFormat) {
					case Illumina18:
						outQuality += (c+Illu18QFLower);
						break;
					default: break;
				}
			}
		}
		
		void unpack(const Pack& pack, string& outSequence, string& outQuality)
		{
			unpack(
				pack.rawData(),
				pack.sequenceLength(),
				pack.qualityFormat(),
				outSequence,
				outQuality);
		}

		void unpack(const unsigned char* data, int sequenceLength, QualityFormat qualityFormat, string &outSequence, string &outQuality)
		{
			outSequence.reserve(sequenceLength);
			outQuality.reserve(sequenceLength);
			for (auto i = 0; i < sequenceLength; ++i) {
				auto c = data[i];
				switch (c >> 6) {
					case bitA:
						outSequence += "A";
						break;
					case bitC:
						outSequence += "C";
						break;
					case bitT:
						outSequence += "T";
						break;
					case bitG:
						outSequence += "G";
				}
				// 63 = 0b00111111 bitmask, we take out the first 2 bits and leave the rest
				c &= 63;
				switch (qualityFormat) {
					case Illumina18:
						outQuality += (c+Illu18QFLower);
						break;
						default: break;
				}
			}
		}
		
		QualityFormat find_quality_format(const string& quality)
		{
			char lowerBoundary = 'E'; // start in the "middle" of the quality
			char upperBoundary = 'E'; // ranges
			for (auto& character : quality) {
				// character is out of valid range, exit
				if (character<ValidQF::LowerLimit || character>ValidQF::UpperLimit) {
					return Unrecognized;
				}
				else if (character<lowerBoundary) {
					lowerBoundary = character; // lower the boundary
				}
				else if (character>upperBoundary) {
					upperBoundary = character; // raise the boundary
				}
			}
			// bitfield to determine quality format
			// start out with all formats possible
			std::bitset<5> format("11111");
			// Check all lower boundaries
			format.set(Illumina15,!(lowerBoundary<'B')); // Illumina 1.5+ impossible
			format.set(Illumina13,!(lowerBoundary<'@')); // Illumina 1.3+ impossible
			format.set(Solexa,!(lowerBoundary<';')); // Solexa impossible
			format.set(Illumina18,!(lowerBoundary<'#')); // Illumina 1.8+ impossible
			// Check all upper boundaries
			format.set(Sanger,!(upperBoundary>'I')); // Sanger impossible
			format.set(Illumina18,!(upperBoundary>'J')); // Illumina 1.8+ impossible
			// Determine what's left that is possible
			if (format[Illumina18]) return Illumina18;
			if (format[Sanger]) return Sanger;
			if (format[Illumina15]) return Illumina15;
			if (format[Illumina13]) return Illumina13;
			if (format[Solexa]) return Solexa;
			return Unrecognized;
		}
	} /* pack */
}

using namespace pip::pack;

ostream& operator<<(ostream& os, const Pack& obj)
{
	switch(obj.qualityFormat()) {
		case Illumina18:
			os << "Illumina 1.8+ Phred+33";
			break;
		case Sanger:
			os << "Sanger Phred+33";
			break;
		case Illumina15:
			os << "Illumina 1.5+ Phred+64";
			break;
		case Illumina13:
			os << "Illumina 1.3+ Phred+64";
			break;
		case Solexa:
			os << "Solexa Solexa+64";
			break;
		default:
			os << "Unrecognized";
	}
	os << " Sequence length: " << obj.sequenceLength();
	os << "\nRaw: " << obj.rawData() << "\n";
	return os;
}