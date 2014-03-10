// stream_trimmomatic.h
// Header file for trimmomatic streaming functions
// Xu Junjie, Kevin
// University of Oregon
// 2014-03-06

#include <vector>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace pip {
	namespace stream {
		namespace qi = boost::spirit::qi;
		struct trim_result {
			int rowid;
			int surviving_length;
			int trim_start;
			int last_base;
			int trim_end;
		};
		template <typename Iterator>
		struct trim_result_parser : qi::grammar<Iterator, trim_result()> {
			trim_result_parser() : trim_result_parser::base_type(start)
			{
				using qi::int_;
			
				start %=
					int_ >> ' ' // rowid
					>> int_ >> ' ' // surviving_length
					>> int_ >> ' ' // trim_start
					>> int_ >> ' ' // last_base
					>> int_ >> '\n'; // trim_end
			}
			qi::rule<Iterator, trim_result()> start;
		};
		
		typedef std::basic_string <unsigned char> ustring;
		class Trimmomatic /*: public StreamBase*/ { // don't inherit for now
		public:
			Trimmomatic();
			~Trimmomatic();
			void start_stream(sqlite3*);
		private:
			void create_pipes();
			void cleanup_pipes();
			void open_pipes(sqlite3*);
			void store_deltas(sqlite3*);
			bool write_out(std::string&,const ustring&);
			bool read_log();
			static const char* file_prefix;
			static const char* create_delta_tbl;
			static const char* insert_deltas;
			static const char* get_reads;
			std::string in1;
			std::string in2;
			std::string log;
			std::string trimlog;
		};
	} /* stream */
} /* pip */

BOOST_FUSION_ADAPT_STRUCT(
	pip::stream::trim_result,
	(int, rowid)
	(int, surviving_length)
	(int, trim_start)
	(int, last_base)
	(int, trim_end)
)