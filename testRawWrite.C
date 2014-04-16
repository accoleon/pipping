#include <fstream>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <regex>
using std::regex;
#include <string>
using std::string;

#include <ctime>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
namespace bi = boost::interprocess;

#include "newpack.h"
using namespace pip;
#include "fastq.h"

void reportSpeed(unsigned long long n, unsigned long long bytes, clock_t time)
{
	double s = time/(double)CLOCKS_PER_SEC;
	double MB = bytes / 1024 / 1024;
	printf("Pip: %llu sequences imported in %4.2f seconds: %6.1f rps %6.1f MB/s\n",n,s,n/s,MB/s);
}

int tok(char* in)
{
	char c;
	for (int n = 0,c = *(in + n); c != 0; ++n) {
		if (c == ':' || c == ' ' || c == '\n') {
			return n;
		}
	}
}

void mm_struct(const string& filename)
{
	// create a file mapping from filename
	bi::file_mapping m_file(filename.c_str(),bi::read_write);
	// create the mapped region of the entire file
	bi::mapped_region region(m_file,bi::read_write);
	region.advise(bi::mapped_region::advice_sequential); // inform the OS of our access pattern
	auto* addr = (char*)region.get_address();
	pip::fastq fq;
	fq.instrument = "HWI-ST0747";
	fq.run = 277;
	fq.flowcell = "D1M96ACXX";
	fq.lane = 6;
	fq.tile = 1101;
	fq.x = 1232;
	fq.y = 2090;
	fq.pair = 1;
	fq.filter = 'N';
	fq.control = 0;
	fq.index = "";
	fq.sequence = "GGATAGTACTAGGGTATCTAATCCTGTTTGCTCCCCACGCTTTCGCACCTCAGCGTCAGTATCGAGCCAGTGAGCCGCCTTCGCCACTGGTGTTCCTCCGAATATCTACGAATTTCACTGCTACACGCGGAATTCCATCCCCCTCTACCGT";
	fq.quality = "ACCCFFDDFH?FHHIGIJJFJJJJJJJJJJJIIJJJGIJJGIJJJIGIJJJIIIIJJJJHHHHHFFFDCEACCDDCDDD@BDDDDBDDDDDDDDDDDDDBBB@BDEEACDDDDDDDEDDCCDDDADBBDDDDDDDDECCBDDDB@9@AA<<";
	printf("size of fq: %d\n", sizeof(fq));
	memcpy(addr,&fq,sizeof(fq));
	region.flush();
	/*
@HWI-ST0747:277:D1M96ACXX:6:1101:1232:2090 1:N:0:
GGATAGTACTAGGGTATCTAATCCTGTTTGCTCCCCACGCTTTCGCACCTCAGCGTCAGTATCGAGCCAGTGAGCCGCCTTCGCCACTGGTGTTCCTCCGAATATCTACGAATTTCACTGCTACACGCGGAATTCCATCCCCCTCTACCGT
+
ACCCFFDDFH?FHHIGIJJFJJJJJJJJJJJIIJJJGIJJGIJJJIGIJJJIIIIJJJJHHHHHFFFDCEACCDDCDDD@BDDDDBDDDDDDDDDDDDDBBB@BDEEACDDDDDDDEDDCCDDDADBBDDDDDDDDECCBDDDB@9@AA<<
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
	*/
}

int mm_read_and_insert(const string& filename, const string& outname)
{
	// create a file mapping from filename in readonly mode
	bi::file_mapping m_file(filename.c_str(),bi::read_only);
	// create the mapped region of the entire file in read only mode
	bi::mapped_region region(m_file,bi::read_only);
	region.advise(bi::mapped_region::advice_sequential); // inform the OS of our access pattern
	
	// create a file mapping from outname in read_write mode
	std::filebuf fbuf;
	fbuf.open(outname, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	cout << "file created" << endl;
	bi::file_mapping o_file(outname.c_str(),bi::read_only);
	cout << "mapping done" << endl;
	//bi::mapped_region o_region(o_file,bi::read_only);
	cout << "region mapped" << endl;
	//o_region.advise(bi::mapped_region::advice_sequential);
	cout << "advised" << endl;
	auto* addr = (char*)region.get_address(); // starting pointer
	auto size = region.get_size();
	pip::fastq_parser<const char*> g;
	
	//auto* o_addr = (char*)o_region.get_address();
	cout << "starting inserts" << endl;
	if (size > 0) {
	  auto startClock = clock();
		unsigned long long n = 0; // we will be dealing with huge numbers here
		pip::fastq fq;
		char const* f(addr); // first iterator/pointer
		char const* l(f + size); // last iterator/pointer
		while (parse(f,l,g,fq)) {
			//pack::Pack packed(fq.sequence,fq.quality,1);
			//std::memcpy(&o_addr,&fq,sizeof(fq));
			//o_addr += sizeof(fq);
			// Bind parameters to sequence data
			++n;
			//fq = {};
			// Show an update every 100k inserts
			if (n % 100000 == 0)
				cerr << "Inserted " << n << " sequences...\r";
		}
		//o_region.flush();
		auto endClock = clock() - startClock;
		reportSpeed(n,size,endClock);
		return n; // return number of rows inserted
	}
	return 0;
}

int ins_read_and_insert(const string& filename, const string& outname)
{
	// create a file mapping from filename in readonly mode
	//bi::file_mapping m_file(filename.c_str(),bi::read_only);
	// create the mapped region of the entire file in read only mode
	//bi::mapped_region region(m_file,bi::read_only);
	//region.advise(bi::mapped_region::advice_sequential); // inform the OS of our access pattern
	std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {

	
		//auto* o_addr = (char*)o_region.get_address();
		cout << "starting inserts" << endl;
	  auto startClock = clock();
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
		pip::fastq_parser<std::string::const_iterator> g;
		unsigned long long n = 0; // we will be dealing with huge numbers here
		pip::fastq fq;
		auto f = contents.cbegin(); // first iterator/pointer
		auto l = contents.cend(); // last iterator/pointer
		while (parse(f,l,g,fq)) {
			//pack::Pack packed(fq.sequence,fq.quality,1);
			//std::memcpy(&o_addr,&fq,sizeof(fq));
			//o_addr += sizeof(fq);
			// Bind parameters to sequence data
			++n;
			//fq = {};
			// Show an update every 100k inserts
			if (n % 100000 == 0)
				cerr << "Inserted " << n << " sequences...\r";
		}
		//o_region.flush();
		auto endClock = clock() - startClock;
		reportSpeed(n,contents.size(),endClock);
		return n; // return number of rows inserted 
  }
	return 0;
}

int main (int argc, char const *argv[])
{
	if (argc == 2) {
		/*cout << "mm read and insert\n";
		mm_read_and_insert(argv[1],"out.test");
		cout << "instream read and insert\n";
		ins_read_and_insert(argv[1],"out.test");*/
		cout << "mm write\n";
		mm_struct("data/cfirst3.fastq");
		return 0;
	}
	cout << "Usage: ./testRawWrite <fastq file>" << endl;
	return -1;
}