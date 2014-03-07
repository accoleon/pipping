// stream_trimmomatic.h
// Header file for trimmomatic streaming functions
// Xu Junjie, Kevin
// University of Oregon
// 2014-03-06

#include <vector>

namespace pip {
	namespace stream {
		class Trimmomatic /*: public StreamBase*/ { // don't inherit for now
		public:
			Trimmomatic();
			~Trimmomatic();
			void start_stream(sqlite3*);
		private:
			void create_pipes();
			void open_pipes(sqlite3*);
			static const char* file_prefix;
			std::string in1;
			std::string in2;
			std::string outp1;
			std::string outp2;
			std::string outup1;
			std::string outup2;
			std::string log;
		};
	} /* stream */
} /* pip */