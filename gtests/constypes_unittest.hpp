#ifndef CONSTYPES_UNITTEST_H
#define CONSTYPES_UNITTEST_H

/**
   \brief Some includes, common to ALL tests
*/

#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
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

// convenient functions
void createFile(const string& d, const naco& n);
string readFile(const string&);
bool existsFile(const string&);

// Test fixture -> used by several tests
class ChdbTest: public ::testing::Test {
protected:
	ChdbTest();
	string input_dir;
	vector<string> expected_file_pathes;
	map<string,string> expected_file_contents;
};

// Another text fixture -> removes the output directory, too
class ChdbTest1 : public ChdbTest {
public:
	~ChdbTest1() { system ("rm -rf inputdir.out"); };
};

// Another test fixture, with other inputdir files
class ChdbTest2: public ChdbTest {
public:
	~ChdbTest2() { system ("rm -rf inputdir.out"); };

protected:
	ChdbTest2();
	string input_dir;
	vector<string> expected_file_pathes;
	map<string,string> expected_file_contents;
};

// Another test fixture, with other inputdir files
class ChdbTest3: public ChdbTest {
public:
	~ChdbTest3() { system ("rm -rf inputdir.out"); };

protected:
	ChdbTest3();
	string input_dir;
	vector<string> expected_file_pathes;
	map<string,string> expected_file_contents;
};

// Some test fixtures used by scheduler_unittest.cpp and buffer_unittest.cpp
class SchedTestStr : public ChdbTest {
public:
	SchedTestStr() {
		int n = 5;
		string tmp((char*) &n,sizeof(int));
		expected_bfr  = tmp;
		expected_bfr += "A.txt";
		expected_bfr += '\0';
		expected_bfr += "B.txt";
		expected_bfr += '\0';
		expected_bfr += "C/C.txt";
		expected_bfr += '\0';
		expected_bfr += "C/C/C.txt";
		expected_bfr += '\0';
		expected_bfr += "D/C.txt";
		expected_bfr += '\0';

		bfr_len = 5;
		bfr_len += sizeof(int) + 5 + 7 + 9 + 7 + 5;
		bfr = malloc(bfr_len);

	};
	~SchedTestStr() { free(bfr); };

protected:
	string expected_bfr;
	void*  bfr;
	size_t bfr_len;
};

class SchedTestInt : public ChdbTest {
public:
	SchedTestInt() {
		expected_values.push_back(3);
		expected_values.push_back(5);
		expected_values.push_back(7);
		int v[]={3,3,5,7};
		string b((char*)v,4*sizeof(int));
		expected_bfr = b;

		bfr_len  = 4*sizeof(int);
		bfr = malloc(bfr_len);
	};
	~SchedTestInt() { free(bfr); };

protected:
	vector_of_int expected_values;
	
	string expected_bfr;
	void*  bfr;
	size_t bfr_len;
};

class SchedTestDbl : public ChdbTest {
public:
	SchedTestDbl() {
		expected_values.push_back(3.14);
		expected_values.push_back(5.28);
		expected_values.push_back(7.98);
		double v[]={3.0,3.14,5.28,7.98};
		string b((char*)v,4*sizeof(double));
		expected_bfr = b;

		bfr_len  = 4*sizeof(double);
		bfr = malloc(bfr_len);
	};
	~SchedTestDbl() { free(bfr); };

protected:
	vector_of_double expected_values;
	
	string expected_bfr;
	void*  bfr;
	size_t bfr_len;
};

class SchedTestStrInt : public ChdbTest {
public:
	SchedTestStrInt() {
		expected_file_pathes.push_back(input_dir + '/' + "A.txt");
		expected_file_pathes.push_back(input_dir + '/' + "B.txt");
		expected_file_pathes.push_back(input_dir + '/' + "C/C.txt");
		expected_file_pathes.push_back(input_dir + '/' + "C/C/C.txt");
		expected_file_pathes.push_back(input_dir + '/' + "D/C.txt");

		// no value, 5 files
		int v[2] = {0,5};
		expected_bfr = string((char*) &v,2*sizeof(int));
		expected_bfr += input_dir + '/' + "A.txt"     + '\0';
		expected_bfr += input_dir + '/' + "B.txt" + '\0';
		expected_bfr += input_dir + '/' + "C/C.txt" + '\0';
		expected_bfr += input_dir + '/' + "C/C/C.txt" + '\0';
		expected_bfr += input_dir + '/' + "D/C.txt"   + '\0';

		// 5 values, 5 files
		int v_1[7] = {5,0,1,2,3,4,5};
		expected_values_1.push_back(0);
		expected_values_1.push_back(1);
		expected_values_1.push_back(2);
		expected_values_1.push_back(3);
		expected_values_1.push_back(4);

		expected_bfr_1 = string((char*) &v_1,7*sizeof(int));
		expected_bfr_1 += input_dir + '/' + "A.txt"     + '\0';
		expected_bfr_1 += input_dir + '/' + "B.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "C/C.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "C/C/C.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "D/C.txt"   + '\0';
		
		bfr_len  = sizeof(int);
		bfr_len += 5 * input_dir.length();
		bfr_len += 10;
		bfr_len += sizeof(int) + 5 + 7 + 9 + 7 + 5;
		bfr = malloc(bfr_len);
	};
	~SchedTestStrInt() { free(bfr); };

protected:
	vector_of_strings expected_file_pathes;
	vector_of_int expected_values_1;
	string expected_bfr;
	string expected_bfr_1;
	void*  bfr;
	size_t bfr_len;
};



#endif
