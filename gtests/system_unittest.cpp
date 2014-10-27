
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
	EXPECT_THROW(callSystem("uptimeee 2>/dev/null",false),logic_error);
	EXPECT_THROW(callSystem("uptimeee 2>/dev/null",true),logic_error);
}

TEST(parseFilePath,parseFilePath) {
	string d,n,b,e;
	parseFilePath("/path/to/some/dir/toto.txt",d,n,b,e);
	EXPECT_EQ(d,"/path/to/some/dir");
	EXPECT_EQ(n,"toto.txt");
	EXPECT_EQ(b,"toto");
	EXPECT_EQ(e,"txt");

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
