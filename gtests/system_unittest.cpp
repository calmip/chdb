
/**
   unit tests for the functions in system.cpp
*/

#include "constypes_unittest.hpp"
#include "../system.hpp"
//#include <fstream>
//#include <sstream>
//#include <algorithm>
//#include <list>
using namespace std;


TEST_F(ChdbTest1,callSystem) {
	string cmd = "cp -a ";
	cmd += getInputDir();
	cmd += " / 2>/dev/null";
	EXPECT_THROW(callSystem(cmd,true),runtime_error);
	EXPECT_NO_THROW(callSystem(cmd,false));
	EXPECT_NO_THROW(callSystem(cmd));

	cmd = "cp -a ";
	cmd += getInputDir();
	cmd += " ";
	cmd += getInputDir();
	cmd += ".copy";
	EXPECT_NO_THROW(callSystem(cmd));

	cmd = "rm -r ";
	cmd += getInputDir();
	cmd += ".copy";
	EXPECT_NO_THROW(callSystem(cmd));
	cerr << "20 LINES WILL BE WRITTEN TO STDERR - THIS IS NORMAL BEHAVIOUR\n";
	int rvl=0;
	EXPECT_NO_THROW(rvl=callSystem("uptimeee",false));
	EXPECT_EQ(127,rvl);
	EXPECT_THROW(callSystem("uptimeee",true),runtime_error);
}

TEST(Parameters,split) {
	vector_of_strings v;
	vector_of_strings expected_v;

	v=split("un");
	expected_v.push_back("un");
	ASSERT_EQ(expected_v,v);

	v=split("un,deux");
	expected_v.clear();
	expected_v.push_back("un");
	expected_v.push_back("deux");
	ASSERT_EQ(expected_v,v);

	v=split("un,deux,");
	expected_v.clear();
	expected_v.push_back("un");
	expected_v.push_back("deux");
	ASSERT_EQ(expected_v,v);

	v=split("");
	expected_v.clear();
	ASSERT_EQ(expected_v,v);
}

TEST(parseFilePath,parseFilePath) {
	string d,n,b,e;
	parseFilePath("/path/to/some/dir/toto.txt",d,n,b,e);
	EXPECT_EQ("/path/to/some/dir",d);
	EXPECT_EQ("toto.txt",n);
	EXPECT_EQ("toto",b);
	EXPECT_EQ("txt",e);

	parseFilePath("/toto.txt",d,n,b,e);
	EXPECT_EQ(d,"/");
	EXPECT_EQ(n,"toto.txt");
	EXPECT_EQ(b,"toto");
	EXPECT_EQ(e,"txt");
	
	parseFilePath("toto.txt",d,n,b,e);
	EXPECT_EQ(d,".");
	EXPECT_EQ(n,"toto.txt");
	EXPECT_EQ(b,"toto");
	EXPECT_EQ(e,"txt");

	parseFilePath("/path/to/some/dir/toto.",d,n,b,e);
	EXPECT_EQ(d,"/path/to/some/dir");
	EXPECT_EQ(n,"toto.");
	EXPECT_EQ(b,"toto");
	EXPECT_EQ(e,"");

	parseFilePath("/path/to/some/dir/.txt",d,n,b,e);
	EXPECT_EQ(d,"/path/to/some/dir");
	EXPECT_EQ(n,".txt");
	EXPECT_EQ(b,"");
	EXPECT_EQ(e,"txt");

	parseFilePath("/path/to/some/dir/.",d,n,b,e);
	EXPECT_EQ(d,"/path/to/some/dir");
	EXPECT_EQ(n,".");
	EXPECT_EQ(b,"");
	EXPECT_EQ(e,"");

	parseFilePath("/path/to/some/dir/toto",d,n,b,e);
	EXPECT_EQ(d,"/path/to/some/dir");
	EXPECT_EQ(n,"toto");
	EXPECT_EQ(b,"toto");
	EXPECT_EQ(e,"");

}

TEST(isEndingWith,isEndingWith) {
	string name="a/b/c/e.txt1";
	EXPECT_EQ(true,isEndingWith(name,".txt1"));
	EXPECT_EQ(true,isEndingWith(name,"txt1"));
	EXPECT_EQ(false,isEndingWith(name,".txt"));

	name=".txt";
	EXPECT_EQ(false,isEndingWith(name,".txt1"));
}

TEST(isBeginningWith,isBeginningWith) {
	string name="a/b/c/e.txt1";
	EXPECT_EQ(true,isBeginningWith(name,"a/b"));
	EXPECT_EQ(false,isBeginningWith(name,"/a/b"));
	EXPECT_EQ(true,isBeginningWith(name,"a/b/c/e.txt1"));
	EXPECT_EQ(false,isBeginningWith(name,"a/b/c/e.txt10"));
	EXPECT_EQ(true,isBeginningWith(name,""));
}

TEST(mkdir,mkdir) {
	string name="some_directory";
	EXPECT_NO_THROW(mkdir(name));
	EXPECT_EQ(true,fileExists(name));
	EXPECT_THROW(mkdir(name),runtime_error);

	string cmd = "rmdir ";
	cmd += name;
	EXPECT_NO_THROW(callSystem(cmd,true));
}
