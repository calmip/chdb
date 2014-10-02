#ifndef CONSTYPES_UNITTEST_H
#define CONSTYPES_UNITTEST_H

/**
   \brief Some includes, common to ALL tests
*/

#include <cstdlib>
#include <cstring>
#include "../constypes.hpp"
#include "../parameters.hpp"
#include "../usingfs.hpp"
#include "gtest/gtest.h"

// This macro is useful to initialize argv with read-write area, this is required by simpleOpt
// #define INIT_ARGV(I,VAL) { char a[]=VAL;argv[I] = a; };
#define INIT_ARGV(I,VAL) argv[I]=(char*)malloc(strlen(VAL)+1);strcpy(argv[I],VAL);
#define FREE_ARGV(ARGC) for(int i=0;i<ARGC;i++){free(argv[i]); argv[i]=NULL;}

// the following is used by some test programs
// file NAme and COntent
struct naco {
	naco(const string& n, const string& c) : name(n),content(c) {};
	string name;
	string content;
};

void createFile(const string& d, const naco& n);

// Test fixture -> used by several tests
class ChdbTest: public ::testing::Test {
protected:
	ChdbTest();
	~ChdbTest();
	string input_dir;
};

#endif
