// testFastRead.C
// Testing the performance of read methods
// Xu Junjie, Kevin
// University of Oregon
// 2014-03-08

#include <fstream>
#include <string>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>


#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("read 500k sequences using fread")
{
	std::FILE *fp = std::fopen("data/anna_500K.1.fastq", "rb");
  if (fp) {
    std::string contents;
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    //return(contents);
  }
}

TEST_CASE("read 500k sequences using streamin") {
	std::ifstream in("data/anna_500K.1.fastq", std::ios::in | std::ios::binary);
	  if (in) {
	    std::string contents;
	    in.seekg(0, std::ios::end);
	    contents.resize(in.tellg());
	    in.seekg(0, std::ios::beg);
	    in.read(&contents[0], contents.size());
	    in.close();
	  }
}

TEST_CASE("read 500k sequences using read")
{
	int fd = open("data/anna_500K.1.fastq", O_RDONLY);
	if (fd) {
		std::string contents;
		auto len = lseek(fd,0,SEEK_END);
		lseek(fd,0,SEEK_SET);
		contents.resize(len);
		//posix_fadvise(fd,0,len,2);
		read(fd,&contents[0],len);
		close(fd);
	}
}