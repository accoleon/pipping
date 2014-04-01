// newpack.h
// New packing functions header
// Xu Junjie, Kevin
// University of Oregon
// 2014-02-28

namespace pip {
	namespace pack {
		enum QualityFormat {
			Sanger = 4,
			Solexa = 3,
			Illumina13 = 2,
			Illumina15 = 1,
			Illumina18 = 0,
			Unrecognized = 5
		};
		enum ValidQF {
			LowerLimit = '!', // any score that goes below '!' on the ASCII code is invalid
			UpperLimit = 'h', // any score that goes above 'h' on the ASCII code is invalid
			Illu18QFLower = '#',
			Illu18QFUpper = 'J',
			SangerQFLower = '!',
			SangerQFUpper = 'I',
			A = 0,
			C = 50,
			T = 100,
			G = 150,
			N = 200,
			bitA = 0,
			bitC = 1,
			bitT = 2,
			bitG = 3
		};
		class Pack {
		public:
			Pack(const std::string& sequence, const std::string& quality);
			Pack(const std::string& sequence, const std::string& quality, int);
			~Pack();
			unsigned int sequenceLength() const;
			QualityFormat qualityFormat() const;
			const unsigned char* rawData() const;
		private:
			unsigned int sequenceLength_;
			QualityFormat qualityFormat_;
			std::vector<unsigned char> v;
			void pack(const std::string& sequence, const std::string& quality);
			inline void losslessPack(const std::string& sequence, const std::string& quality);
		};
		
		void losslessUnpack(
			const Pack& pack,
			std::string& outSequence,
			std::string& outQuality
		);
		void losslessUnpack(
			const unsigned char* data,
			int sequenceLength, 
			QualityFormat qualityFormat,
			std::string &outSequence,
			std::string &outQuality
		);
		void unpack(
			const Pack& pack,
			std::string& outSequence,
			std::string& outQuality
		);
		void unpack(
			const unsigned char* data,
			int sequenceLength, 
			QualityFormat qualityFormat,
			std::string &outSequence,
			std::string &outQuality
		);
		
		QualityFormat find_quality_format(const std::string& quality);
	} /* pack */
} /* pip */
std::ostream& operator<<(std::ostream&, const pip::pack::Pack&);