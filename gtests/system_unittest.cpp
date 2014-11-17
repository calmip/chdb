
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


TEST_F(ChdbTest,callSystem) {
	EXPECT_THROW(callSystem("cp -a inputdir / 2>/dev/null",true),runtime_error);
	EXPECT_NO_THROW(callSystem("cp -a inputdir / 2>/dev/null",false));
	EXPECT_NO_THROW(callSystem("cp -a inputdir / 2>/dev/null"));
	EXPECT_NO_THROW(callSystem("cp -a inputdir inputdir.copy"));
	EXPECT_NO_THROW(callSystem("rm -r inputdir.copy"));
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
