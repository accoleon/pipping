// Piper.C
// Manages named piping functions for pip
// Xu Junjie, Kevin
// University of Oregon
// 2014-03-04

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;

#include <libconfig.h++>
using namespace libconfig;

int la;

void createPipe(string filename, int pair) {
	filename += ".";
	filename += std::to_string(pair);
	filename += ".fastq";
	cout << "creating " << filename << "\n";
	umask(0);
	int ret = mkfifo(filename.c_str(),S_IFIFO|0666);
	if (ret == 0) {
		cout << "fifo created" << endl;
	}
}