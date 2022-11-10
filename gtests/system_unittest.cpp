/** This file is part of chdb software
 * 
 * chdb helps users to run embarrassingly parallel jobs on a supercomputer
 *
 * chdb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2015-2018    C A L M I P
 *  chdb is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with chdb.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:
 *        Emmanuel Courcelle - C.N.R.S. - UMS 3667 - CALMIP
 *        Nicolas Renon - Université Paul Sabatier - University of Toulouse)
 */

/**
   unit tests for the functions in system.cpp
*/

#include "constypes_unittest.hpp"
#include "../src/system.hpp"
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
    EXPECT_EQ(true,isEndingWith(name,name));
    EXPECT_EQ(true,isEndingWith(name,".txt1"));
    EXPECT_EQ(true,isEndingWith(name,"txt1"));
    EXPECT_EQ(false,isEndingWith(name,".txt"));

    name=".txt";
    EXPECT_EQ(false,isEndingWith(name,".txt1"));
}

TEST(isBeginningWith,isBeginningWith) {
    string name="a/b/c/e.txt1";
    EXPECT_EQ(true,isBeginningWith(name,name));
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

// Testing vctToBfr AND bfrToVct - Normal case
TEST(vctToBrfString,vctToBfrString) {

    vector_of_strings files={"a/b/c/e.txt","f/g/h/i.txt","j/k"};

    size_t data_size = 0;
    size_t exp_data_size = 0;
    for (auto f: files)
    {
        exp_data_size += f.size();
    }
    size_t bfr_size = 100;
    void* bfr = calloc(100, sizeof(char));

    // Serialize the vector of strings to the buffer
    vctToBfr(files,bfr,bfr_size,data_size);
    EXPECT_GT(data_size, exp_data_size);
    EXPECT_LT(data_size, bfr_size);

    // Recreate the vector of strings from the buffer
    size_t rec_data_size = 0;
    vector_of_strings rec_files;
    bfrToVct(bfr, rec_data_size, rec_files);

    // Should retrieve the same data !
    EXPECT_EQ(data_size, rec_data_size);
    EXPECT_EQ(files, rec_files);

    free(bfr);
};

// Testing vctToBfr AND bfrToVct - Limit case (serialize nothing)
TEST(vctToBfrStringZero,vctToBfrStringZero) {
    vector_of_strings files;

    size_t data_size = 0;

    size_t bfr_size = 100;
    void* bfr = calloc(100, sizeof(char));

    // Serialize the vector of strings to the buffer
    // We just store 0 at start of buffer
    vctToBfr(files,bfr,bfr_size,data_size);
    EXPECT_EQ(data_size, sizeof(int));
    size_t len = * (size_t*)bfr;
    EXPECT_EQ(len, (size_t)0);
   
    // Recreate an empty vector from an empty buffer
    size_t rec_data_size = (size_t)0;

    vector_of_strings rec_files;
    bfrToVct(bfr, rec_data_size, rec_files);

    EXPECT_EQ(rec_data_size,sizeof(int) );
    EXPECT_EQ(rec_files.size(), (size_t)0);

    free(bfr);
}

// Testing vctToBfr - Limit case (buffer two small)
TEST(vctToBfrStringBfr2small,vctToBfrStringBfr2small) {
    vector_of_strings files={"a/b/c/e.txt","f/g/h/i.txt","j/k"};

    size_t data_size = 0;
    size_t exp_data_size = 0;

    size_t bfr_size = 8;
    void* bfr = calloc(8, sizeof(char));

    // Serialize the vector of strings to the buffer
    EXPECT_THROW(vctToBfr(files,bfr,bfr_size,data_size),runtime_error);

    // buffer unchanged
    EXPECT_EQ(data_size, exp_data_size);
    for (size_t i=0; i<bfr_size; ++i)
    {
        EXPECT_EQ(*((char*)bfr + i), 0);
    }

    free(bfr);
}

// Testing vctToBfr<int> AND bfrToVct<int> - Normal case
TEST(vctToBfrInt,vctToBfrInt) {

    vector_of_int values={5,78,23,98,123,65};

    size_t data_size = 0;
    size_t exp_data_size = sizeof(int) + 6 * sizeof(int);

    size_t bfr_size = 100;
    void* bfr = calloc(100, sizeof(char));

    // Serialize the vector of int to the buffer
    vctToBfr<int>(values,bfr,bfr_size,data_size);
    EXPECT_EQ(data_size, exp_data_size);
    EXPECT_LT(data_size, bfr_size);

    // Recreate the vector of int from the buffer
    size_t rec_data_size = 0;
    vector_of_int rec_values;
    bfrToVct<int>(bfr, rec_data_size, rec_values);

    // Should retrieve the same data !
    EXPECT_EQ(data_size, rec_data_size);
    EXPECT_EQ(values, rec_values);

    free(bfr);
}

// Testing vctToBfr<int> AND bfrToVct<int> - Limit case (serialize nothing)
TEST(vctToBfrIntZero,vctToBfrIntZero) {

    vector_of_int values;

    size_t data_size = 0;

    size_t bfr_size = 100;
    void* bfr = calloc(100, sizeof(char));

    // Serialize the vector of int to the buffer
    // We just store 0 at start of buffer
    vctToBfr(values,bfr,bfr_size,data_size);
    EXPECT_EQ(data_size, sizeof(int));
    size_t len = * (size_t*)bfr;
    EXPECT_EQ(len, (size_t)0);
   
    // Recreate an empty vector from an empty buffer
    size_t rec_data_size = (size_t)0;

    vector_of_int rec_values;
    bfrToVct<int>(bfr, rec_data_size, rec_values);

    EXPECT_EQ(rec_data_size,sizeof(int) );
    EXPECT_EQ(rec_values.size(), (size_t)0);

    free(bfr);
}

// Testing vctToBfr<int> - Limit case (buffer two small)
TEST(vctToBfrIntBfr2small,vctToBfrIntBfr2small) {

    vector_of_int values={5,78,23,98,123,65,5,78,23,98,123,65};

    size_t data_size = 0;
    size_t exp_data_size = 0;

    size_t bfr_size = 8;
    void* bfr = calloc(8, sizeof(char));

    // Serialize the vector of int to the buffer
    EXPECT_THROW(vctToBfr<int>(values,bfr,bfr_size,data_size),runtime_error);

    // buffer unchanged
    EXPECT_EQ(data_size, exp_data_size);
    for (size_t i=0; i<bfr_size; ++i)
    {
        EXPECT_EQ(*((char*)bfr + i), 0);
    }

    free(bfr);
}
