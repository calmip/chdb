
/**
   unit tests for the class Directories, and its subclasses
*/


#include "constypes_unittest.hpp"
#include "../basicscheduler.hpp"
#include <fstream>
using namespace std;
	
TEST_F(ChdbTest1,Block1) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh #input_path# #output_path# 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file '#output_path#'";

	cout << "NOW CALLING " << cmd << '\n';
	system(cmd.c_str());

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
};

TEST_F(ChdbTest1,Block2) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh #input_path# #output_path# 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file '#output_path#'";
	cmd += "--block-size 2";

	cout << "NOW CALLING " << cmd << '\n';
	system(cmd.c_str());

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
};

TEST_F(ChdbTest1,Block5) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh #input_path# #output_path# 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file '#output_path#'";
	cmd += "--block-size 5";

	cout << "NOW CALLING " << cmd << '\n';
	system(cmd.c_str());

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
};

TEST_F(ChdbTest1,onerror) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh #input_path# #output_path#' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file '#output_path#' ";
	cmd += "--on-error errors.txt ";

	cout << "NOW CALLING " << cmd << '\n';
	system(cmd.c_str());

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
	EXPECT_EQ("1\tD/C.txt\n\n",readFile("errors.txt"));
};

TEST_F(ChdbTest1,fiveslaves) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 5 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh #input_path# #output_path# 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file '#output_path#' ";

	cout << "NOW CALLING " << cmd << '\n';
	system(cmd.c_str());

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
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

