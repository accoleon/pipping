// fastq.h
// Header file for fastq parsing functions
// Xu Junjie, Kevin
// University of Oregon
// 2014-03-05

// Use Boost Spirit Qi libraries to create a high performance recursive parser
// To parse fastq files into structs. Original FASTQSequence.fetch method took
// FASTQSequence.fetch() - 500k sequences ~76 seconds
// fastq parse - 500k sequences ~7 seconds

#define FUSION_MAX_VECTOR_SIZE 20
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>

namespace pip {
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;
	
	struct fastq {
		std::string instrument;
		int run;
		std::string flowcell;
		int lane;
		int tile;
		int x;
		int y;
		int pair;
		char filter;
		int control;
		std::string index;
		std::string sequence;
		std::string quality;
	};
	
	template <typename Iterator>
	struct fastq_parser : qi::grammar<Iterator, fastq()> {
		fastq_parser() : fastq_parser::base_type(start)
		{
			using qi::int_;
			using qi::lit;
			using ascii::char_;
			
			start %=
				'@' >> +(char_ - ':') >> ':' // @ and instrument
				>> int_ >> ':' // runid
				>> +(char_ - ':') >> ':' // flowcell id
				>> int_ >> ':' // flowcell lane
				>> int_ >> ':' // tile
				>> int_ >> ':' // x
				>> int_ >> ' ' // y
				>> int_ >> ':' // pair
				>> char_ >> ':' // filter
				>> int_ >> ':' // control
				>> *(char_ - '\n') >> '\n' // index sequence
				>> +(char_ - lit("\n+\n")) >> lit("\n+\n") // sequence
				>> +(char_ - '\n') >> '\n'; // quality
		}
		qi::rule<Iterator, fastq()> start;
	};
} /* pip */

BOOST_FUSION_ADAPT_STRUCT(
	pip::fastq,
	(std::string, instrument)
	(int, run)
	(std::string, flowcell)
	(int, lane)
	(int, tile)
	(int, x)
	(int, y)
	(int, pair)
	(char, filter)
	(int, control)
	(std::string, index)
	(std::string, sequence)
	(std::string, quality)
)