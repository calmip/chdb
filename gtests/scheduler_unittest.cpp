
/**
   unit tests for the class Directories, and its subclasses
*/


#include "constypes_unittest.hpp"
#include "../basicscheduler.hpp"
#include <fstream>
using namespace std;

class SchedTestStr : public ChdbTest {
public:
	SchedTestStr() {
		expected_file_names.push_back(input_dir + '/' + "B.txt");
		expected_file_names.push_back(input_dir + '/' + "C/C.txt");
		expected_file_names.push_back(input_dir + '/' + "C/C/C.txt");
		expected_file_names.push_back(input_dir + '/' + "D/C.txt");
		expected_file_names.push_back(input_dir + '/' + "A.txt");

		int n = 5;
		string tmp((char*) &n,sizeof(int));
		expected_bfr  = tmp;
		expected_bfr += input_dir + '/' + "B.txt" + '\0';
		expected_bfr += input_dir + '/' + "C/C.txt" + '\0';
		expected_bfr += input_dir + '/' + "C/C/C.txt" + '\0';
		expected_bfr += input_dir + '/' + "D/C.txt"   + '\0';
		expected_bfr += input_dir + '/' + "A.txt"     + '\0';

		bfr_len  = 5 * input_dir.length();
		bfr_len += 10;
		bfr_len += sizeof(int) + 5 + 7 + 9 + 7 + 5;
		bfr = malloc(bfr_len);

	};
	~SchedTestStr() { free(bfr); };

protected:
	vector_of_strings expected_file_names;
	
	//size_t expected_length;
	string expected_bfr;
	void*  bfr;
	size_t bfr_len;
};

TEST_F(SchedTestStr,vctToBfrStrings) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	Parameters prms(7,argv);
	UsingFs dir(prms);
	BasicScheduler sched(prms,dir);
	vector_of_strings files=dir.getFiles();

	size_t data_len;
	sched.vctToBfr(files,bfr,bfr_len,data_len);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),bfr_len));
	EXPECT_EQ(bfr_len,data_len);

	FREE_ARGV(7);
};

TEST_F(SchedTestStr,bfrToVctStrings) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	char const* bfr = expected_bfr.c_str();

	Parameters prms(7,argv);
	UsingFs dir(prms);
	BasicScheduler sched(prms,dir);

	vector_of_strings file_names;
	size_t  data_size;
	sched.bfrToVct((const void*)bfr,data_size,file_names);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_file_names,file_names);
	
	FREE_ARGV(7);
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

TEST_F(SchedTestInt,vctToBfrInt) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	Parameters prms(7,argv);
	UsingFs dir(prms);
	BasicScheduler sched(prms,dir);
	vector_of_strings files=dir.getFiles();

	vector_of_int values;
	values.push_back(3);
	values.push_back(5);
	values.push_back(7);

	size_t data_len;
	sched.vctToBfr(values,bfr,bfr_len,data_len);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),bfr_len));
	EXPECT_EQ(bfr_len,data_len);

	FREE_ARGV(7);
};

TEST_F(SchedTestInt,bfrToVctInt) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	char const* bfr = expected_bfr.c_str();

	Parameters prms(7,argv);
	UsingFs dir(prms);
	BasicScheduler sched(prms,dir);

	vector_of_int values;
	size_t data_size;
	sched.bfrToVct(bfr,data_size,values);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_values,values);
	
	FREE_ARGV(7);
};

class SchedTestStrInt : public ChdbTest {
public:
	SchedTestStrInt() {
		expected_file_names.push_back(input_dir + '/' + "B.txt");
		expected_file_names.push_back(input_dir + '/' + "C/C.txt");
		expected_file_names.push_back(input_dir + '/' + "C/C/C.txt");
		expected_file_names.push_back(input_dir + '/' + "D/C.txt");
		expected_file_names.push_back(input_dir + '/' + "A.txt");

		// no value, 5 files
		int v[2] = {0,5};
		expected_bfr = string((char*) &v,2*sizeof(int));
		expected_bfr += input_dir + '/' + "B.txt" + '\0';
		expected_bfr += input_dir + '/' + "C/C.txt" + '\0';
		expected_bfr += input_dir + '/' + "C/C/C.txt" + '\0';
		expected_bfr += input_dir + '/' + "D/C.txt"   + '\0';
		expected_bfr += input_dir + '/' + "A.txt"     + '\0';

		// 5 values, 5 files
		int v_1[7] = {5,0,1,2,3,4,5};
		expected_values_1.push_back(0);
		expected_values_1.push_back(1);
		expected_values_1.push_back(2);
		expected_values_1.push_back(3);
		expected_values_1.push_back(4);

		expected_bfr_1 = string((char*) &v_1,7*sizeof(int));
		expected_bfr_1 += input_dir + '/' + "B.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "C/C.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "C/C/C.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "D/C.txt"   + '\0';
		expected_bfr_1 += input_dir + '/' + "A.txt"     + '\0';
		
		bfr_len  = sizeof(int);
		bfr_len += 5 * input_dir.length();
		bfr_len += 10;
		bfr_len += sizeof(int) + 5 + 7 + 9 + 7 + 5;
		bfr = malloc(bfr_len);
	};
	~SchedTestStrInt() { free(bfr); };

protected:
	vector_of_strings expected_file_names;
	vector_of_int expected_values_1;
	string expected_bfr;
	string expected_bfr_1;
	void*  bfr;
	size_t bfr_len;
};

TEST_F(SchedTestStrInt,readwriteToSndBfr) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"5");
	
	Parameters prms(9,argv);
	UsingFs dir(prms);

	// Load (empty) values and file_names from expected_bfr
	BasicScheduler sched(prms,dir);
	sched.readFrmRecvBfr(expected_bfr.c_str());
	EXPECT_EQ(0,sched.return_values.size());
	EXPECT_EQ(expected_file_names,sched.file_names);

	// Create a bfr from the (empty) values and file_names and compare it to expected_bfr
	size_t data_size;
	size_t bfr_size = expected_bfr.length() + 50;
	void * bfr = malloc(bfr_size);
	sched.writeToSndBfr(bfr,bfr_size,data_size);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),data_size));
	EXPECT_EQ(data_size,expected_bfr.length());
	
	// clear file_names and reload values and file_names from  expected_bfr_1
	sched.file_names.clear();
	sched.readFrmRecvBfr(expected_bfr_1.c_str());
	EXPECT_EQ(expected_values_1,sched.return_values);
	EXPECT_EQ(expected_file_names,sched.file_names);

	// write again to bfr and compare value to expected_bfr_1
	sched.writeToSndBfr(bfr,bfr_size,data_size);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr_1.c_str(),data_size));
	EXPECT_EQ(data_size,expected_bfr_1.length());

	free(bfr);

	FREE_ARGV(9);
};

// Step 3. Call RUN_ALL_TESTS() in main().
//
// We do this by linking in src/gtest_main.cc file, which consists of
// a main() function which calls RUN_ALL_TESTS() for us.
//
// This runs all the tests you've defined, prints the result, and
// returns 0 if successful, or 1 otherwise.
//
// Did you notice that we didn't register the tests?  The
// RUN_ALL_TESTS() macro magically knows about all the tests we
// defined.  Isn't this convenient?

