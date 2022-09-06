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
   unit tests for the class Directories, and its subclasses
*/

#include "../src/constypes.hpp"
#include "constypes_unittest.hpp"
#include "../src/system.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <list>
#include<memory>
using namespace std;

#define OUTEXT ".out"
#ifdef OUTDIRPERSLAVE
#define SLAVEOUTEXT ".out.1"
#else   
#define SLAVEOUTEXT OUTEXT
#endif

using ::testing::TestWithParam;
using ::testing::Values;

/**
   \brief compute output directory name from input directory and from directory type
          Supported directories = UsingFs
 */

string computeOutputDir(const string& input_dir,const string& directory_type) {
    string output_dir = input_dir;
    // inputdir ==> inputdir.out
    if (directory_type == "UsingFs") {
        output_dir += OUTEXT;

    // inputdir.db ==> inputdir.out.db
    } else if (directory_type == "UsingBdbh") {
        output_dir = output_dir.substr(0,output_dir.length()-3);
        output_dir += OUTEXT;
        output_dir += ".db";
    } else {
        throw(logic_error("unknown directory_type"));
    }
    return output_dir;
}

// testing makeOutDir
TEST_P(TestCase1,makeOutDir) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");
    
    Parameters prms(11,argv);

    // start a block, dir will be destroyed at end of block
    {
        unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
        Directories& dir = *aptr_dir.get();
        dir.setRank(0,1);

        // NOTE - This test may NOT WORK CORRECTLY an an nfs filesystem !
        //        It should work correctly on Lustre
        // create inputdir.out - do not append the rank to the directory name, remove the directory if exists
        EXPECT_NO_THROW(dir.makeOutDir(false,true));
        
        // replace inputdir.out
        EXPECT_NO_THROW(dir.makeOutDir(false,true));

        // could not create inputdir.out (already exists)
        EXPECT_THROW(dir.makeOutDir(false,false),runtime_error);

        // Is it really a Berkeleydb ?
        // NOTE - This test may NOT WORK CORRECTLY an an nfs filesystem !
        if ( GetParam()->getDirectoryType()=="UsingBdbh" ) {
            string out_dir = prms.getOutDir();
            EXPECT_EQ(true,fileExists(out_dir+"/data"));
            EXPECT_EQ(true,fileExists(out_dir+"/metadata"));
        }
    }

    // remove inputdir.out
    string cmd="rm -r " + prms.getOutDir();
    EXPECT_NO_THROW(callSystem(cmd,true));

    // start another block, dir will be destroyed at end of block
    {
        unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
        Directories& dir = *aptr_dir.get();

        // Call with rank_flg=true, directory inputdir.out.1 created... or not
        dir.setRank(1,2);
        EXPECT_NO_THROW(dir.makeOutDir(true,true));
        string out_1 = getInputDir();
#ifdef OUTDIRPERSLAVE
        out_1 += ".out.1";
        cmd = "ls -ld " + out_1 + " >/dev/null 2>&1";
        EXPECT_NO_THROW(callSystem(cmd,true));
        cmd = "rm -r " + out_1;
        EXPECT_NO_THROW(callSystem(cmd,true));
#else
        out_1 += ".out";
        cmd = "ls -ld " + out_1 + " >/dev/null 2>&1";
        EXPECT_THROW(callSystem(cmd,true),runtime_error);
        cmd = "rm -r " + out_1 + " >/dev/null 2>&1";
        EXPECT_THROW(callSystem(cmd,true),runtime_error);
#endif

        // DIRECTORY inputdir.out not CREATED ANYWAY !
        cmd="rm -r " + prms.getOutDir();
        cmd += " 2>/dev/null";
        EXPECT_THROW(callSystem(cmd,true),runtime_error);
    }

    FREE_ARGV(11);
}

// testing makeTempOutDir
TEST_P(TestCase1,MakeTempOutDir) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");

    Parameters prms(11,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
    //UsingFs dir(prms);

    dir.makeOutDir(false,true);
    dir.makeTempOutDir();
    string tmp_out = dir.getTempOutDir();

    string blank = "";
    string cmd = "ls -l ";
    string output_dir = computeOutputDir(getInputDir(),GetParam()->getDirectoryType());
    string out = output_dir + "_??????";
    cmd += out;
    cmd += " >/dev/null 2>&1";

    // TESTING usingfs
    if (GetParam()->getDirectoryType() == "UsingFs" ) {
        // If tmp not used callSystem should throw an exception (inputdir_????? does not exist)
#ifdef NOTMP
        EXPECT_EQ(output_dir,tmp_out);
        EXPECT_EQ(prms.getOutDir(),tmp_out);
        EXPECT_THROW(callSystem(cmd,true),runtime_error);
#else
        if (GetParam()->getTmpDir() == "none") {
            EXPECT_EQ(output_dir,tmp_out);
            EXPECT_EQ(prms.getOutDir(),tmp_out);
            EXPECT_THROW(callSystem(cmd,true),runtime_error);
        }
        else
        {
            EXPECT_NE(blank,tmp_out);
            EXPECT_NO_THROW(callSystem(cmd,true));
            string cmd1="rmdir ";
            cmd1 += tmp_out;
            EXPECT_NO_THROW(callSystem(cmd1,true));
        }
#endif
    } else {    
        // Testing UsingBdbh
        EXPECT_NE(blank,tmp_out);
        EXPECT_NO_THROW(callSystem(cmd,true));
        string cmd1="rmdir ";
        cmd1 += tmp_out;
        EXPECT_NO_THROW(callSystem(cmd1,true));
    }
    FREE_ARGV(11);
};

// testing getTempOutDir/getTempInDir, with or not a temporary directory
TEST_P(TestCase1,getTempOut_or_InDir) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");

    Parameters prms(11,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();

    // exception tempdir not yet initialized !
    EXPECT_THROW(dir.getTempOutDir(),logic_error);
    EXPECT_THROW(dir.getTempInDir(),logic_error);

    // exception outdir not yet initialized !
    EXPECT_THROW(dir.makeTempOutDir(),logic_error);
    EXPECT_THROW(dir.getOutDir(),logic_error);

    // making things correctly
    EXPECT_NO_THROW(dir.makeOutDir(false,true));
    EXPECT_NO_THROW(dir.makeTempOutDir());

    EXPECT_NO_THROW(dir.getTempOutDir());
    string tmp_out_dir = dir.getTempOutDir();
    string tmp_in_dir  = dir.getTempInDir();
    
    string output_dir = computeOutputDir(getInputDir(),GetParam()->getDirectoryType());
    EXPECT_EQ((string) output_dir, dir.getOutDir());

    string cmd = "rmdir ";
    cmd += output_dir;

    // TESTING UsingFs
    if (GetParam()->getDirectoryType() == "UsingFs" ) {

        // UsingFs, the name of the tmp in directory is ALSO the name of the input directory
        EXPECT_EQ(tmp_in_dir,getInputDir());

#ifdef NOTMP
        // NOTMP => The name of the tmp directory is ALSO the name of the output directory
        EXPECT_EQ((string) output_dir,tmp_out_dir);
        EXPECT_NO_THROW(callSystem(cmd));
#else
        if (GetParam()->getTmpDir() == "none") {
            EXPECT_EQ((string) output_dir,tmp_out_dir);
            EXPECT_NO_THROW(callSystem(cmd));
        }
        else
        {
            // Temporary => Some other name
            string s=GetParam()->getTmpDir()+'/'+output_dir+'_';
            size_t l=s.length();
            EXPECT_EQ(s,tmp_out_dir.substr(0,l));
            
            cmd = "rmdir ";
            cmd += s;
            cmd += "??????";
            EXPECT_NO_THROW(callSystem(cmd));
        }
#endif
    } else {
        // Testing UsingBdbh
        string s=GetParam()->getTmpDir()+'/'+output_dir+'_';
        size_t l=s.length();
        EXPECT_EQ(s,tmp_out_dir.substr(0,l));
        EXPECT_EQ(s,tmp_in_dir.substr(0,l));
        EXPECT_EQ(true,isEndingWith(tmp_out_dir,(string)"output"));
        EXPECT_EQ(true,isEndingWith(tmp_in_dir,(string)"input"));
        
        cmd = "rmdir ";
        cmd += s;
        cmd += "??????";
        EXPECT_NO_THROW(callSystem(cmd+"/input"));        
        EXPECT_NO_THROW(callSystem(cmd+"/output"));
        EXPECT_NO_THROW(callSystem(cmd));
    }
    FREE_ARGV(11);
}

// Testing Directory WITHOUT the --out-files parameter
// Parameters are checked when setRank is called... 
TEST_P(TestCase1,withoutOutfiles) {
    cout << GetParam()->getDescription() << '\n';
    
    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    //INIT_ARGV(9,"--out-files");
    //INIT_ARGV(10,"%out-dir%/%path%");

    Parameters prms(9,argv);

    // Create a directory with rank 0 --> Parameters are checked
    unique_ptr<Directories> aptr_dir_master(GetParam()->createDirectory(prms));
    Directories& dir_master = *aptr_dir_master.get();

    // Create a directory with rank 1 --> Parameters are NOT checked
    unique_ptr<Directories> aptr_dir_slave(GetParam()->createDirectory(prms));
    Directories& dir_slave = *aptr_dir_slave.get();
    
    if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
        EXPECT_THROW(dir_master.setRank(0,2),runtime_error);
        EXPECT_NO_THROW(dir_slave.setRank(1,2));
    } else {
        EXPECT_NO_THROW(dir_master.setRank(0,2));
        EXPECT_NO_THROW(dir_slave.setRank(1,2));
    }
}    
    
// testing Directory::completeFilePath, with or without temporary directory
TEST_P(TestCase1,completeFilePath) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");
    
    Parameters prms(11,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);
    dir.setRank(1,2);
    dir.makeOutDir(true,true);
    dir.makeTempOutDir();

    string f1 = "A/B/C/D.txt";

    string s1 = "%out-dir%/%path%";
    string s1_1 = "%path%";
    string s1_2 = s1_1;
    string s2 = "%out-dir%/%name%";
    string s3 = "%out-dir%/%basename%.out";
    string s4 = "%out-dir%/%dirname%/%basename%.out";
    string s5 = s1 + "%%" + s1 + "%%" + s2 + "%%" + s2 + "%%" + s3 + "%%" + s3;
    string s6 = "%in-dir%";
    dir.completeFilePath(f1,s1);

    // Testing the behaviour when %out-dir% is not specified
    dir.completeFilePath(f1,s1_1);
    dir.completeFilePath(f1,s1_2,true);

    dir.completeFilePath(f1,s2);
    dir.completeFilePath(f1,s3);
    dir.completeFilePath(f1,s4);
    dir.completeFilePath(f1,s5);
    dir.completeFilePath(f1,s6);

    string slave_outdir = dir.getTempOutDir();
    string expected_s1 = slave_outdir + "/A/B/C/D.txt";
    string expected_s1_1="A/B/C/D.txt";
    string expected_s1_2 = expected_s1;
    string expected_s2 = slave_outdir + "/D.txt";
    string expected_s3 = slave_outdir + "/D.out";
    string expected_s4 = slave_outdir + "/A/B/C/D.out";
    string expected_s5 = expected_s1 + "%%" + expected_s1 + "%%" + expected_s2 + "%%" + expected_s2 + "%%" + expected_s3 + "%%" + expected_s3;
    string expected_s6 = dir.getTempInDir();
    vector_of_strings block;
    vector_of_strings expected_block;

    EXPECT_EQ(expected_s1,s1);
    EXPECT_EQ(expected_s1_1,s1_1);
    EXPECT_EQ(expected_s1_2,s1_2);
    EXPECT_EQ(expected_s2,s2);
    EXPECT_EQ(expected_s3,s3);
    EXPECT_EQ(expected_s4,s4);
    EXPECT_EQ(expected_s5,s5);
    EXPECT_EQ(expected_s6,s6);

    FREE_ARGV(11);
}

// testing UsingFs::findOrCreateDir (subdirectories of output dir)
TEST_P(TestCase1,findOrCreateDir) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");
    
    Parameters prms(11,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);

    // create the directory a/b/c/d/e, then the file f.txt
    EXPECT_NO_THROW(dir.findOrCreateDir("a/b/c/d/e/f.txt"));
    { ofstream f("a/b/c/d/e/f.txt"); }

    // nothing to do (the directories are already created)
    EXPECT_NO_THROW(dir.findOrCreateDir("a/b/c/d/e/f.txt"));

    // exception because f.txt exists and is not a directory
    // Two different pathes in the function
    EXPECT_THROW(dir.findOrCreateDir("a/b/c/d/e/f.txt/g"),runtime_error);
    EXPECT_THROW(dir.findOrCreateDir("a/b/c/d/e/f.txt/g/h"),runtime_error);

    //exception because authorization problems
    EXPECT_THROW(dir.findOrCreateDir("/etc/something/f.txt"),runtime_error);

    string cmd="rm -r a";
    system(cmd.c_str());

    FREE_ARGV(11);
}

// testing getFiles, unsorted (ie sorted in their names)
TEST_P(TestCase1,getFiles_Unsorted) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");
    
    Parameters prms(11,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);

    vector_of_strings found_files=dir.getFiles();
    vector_of_strings expected_files;
    string topdir = "";
    if ( GetParam()->getDirectoryType() == "UsingBdbh" ) topdir = "inputdir1/";
        
    expected_files.push_back(topdir + "A.txt");
    expected_files.push_back(topdir + "B.txt");
    expected_files.push_back(topdir + "C/C.txt");
    expected_files.push_back(topdir + "C/C/C.txt");
    expected_files.push_back(topdir + "D/C.txt");
    EXPECT_EQ(expected_files,found_files);

    // again
    found_files=dir.getFiles();
    EXPECT_EQ(expected_files,found_files);

    FREE_ARGV(11);

}

// testing getFiles in iter type
TEST_P(TestCase1,getFiles_iter) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"1 10 2");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-dir");
    INIT_ARGV(10,"iter.out");
    
    Parameters prms(11,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();

    vector_of_strings found_files=dir.getFiles();
    vector_of_strings expected_files = {"1","3","5","7","9"};
    EXPECT_EQ(expected_files,dir.getFiles());
    found_files=dir.getFiles();
    EXPECT_EQ(expected_files,dir.getFiles());

    FREE_ARGV(7);

}

// testing getFiles, sorted in size
TEST_P(TestCase1,getFiles_Sorted) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--sort-by-size");
    INIT_ARGV(8,"--tmp-dir");
    INIT_ARGV(9,GetParam()->getTmpDir().c_str());
    INIT_ARGV(10,"--out-files");
    INIT_ARGV(11,"%out-dir%/%path%");
    
       // Sort by size
    Parameters prms(12,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();

    dir.setRank(0,4);
    vector_of_strings found_files=dir.getFiles();
    list<string> expected_files;
        string topdir = "";
        if ( GetParam()->getDirectoryType() == "UsingBdbh" ) topdir = "inputdir1/";
    expected_files.push_back(topdir + "C/C/C.txt");
    expected_files.push_back(topdir + "D/C.txt");
    expected_files.push_back(topdir + "A.txt");
    expected_files.push_back(topdir + "B.txt");
    expected_files.push_back(topdir + "C/C.txt");
    expected_files.reverse();
    EXPECT_EQ(true,equal(expected_files.begin(),expected_files.end(),found_files.begin()));

    // again
    found_files=dir.getFiles();
    EXPECT_EQ(true,equal(expected_files.begin(),expected_files.end(),found_files.begin()));

    FREE_ARGV(12);

}

// Testing blocks of 1 file (default)
TEST_P(TestCase1,block1) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");

    Parameters prms(11,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);

    vector_of_strings block;
    vector_of_strings expected_block;

    int nb_files = dir.getNbOfFiles();
    EXPECT_EQ(5,nb_files);

    // Add a blank file name, it will be ignored !
    dir.files.push_back("");
    EXPECT_EQ(5,dir.getNbOfFiles());

    string topdir = "";
        if ( GetParam()->getDirectoryType() == "UsingBdbh" ) topdir = "inputdir1/";

    // 1th blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "A.txt");
    EXPECT_EQ(expected_block,block);

    // 2nd blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "B.txt");
    EXPECT_EQ(expected_block,block);

    // 3rd blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "C/C.txt");
    EXPECT_EQ(expected_block,block);

    // 4th blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "C/C/C.txt");
    EXPECT_EQ(expected_block,block);

    // 5th blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "D/C.txt");
    EXPECT_EQ(expected_block,block);

    // 6th blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back("");
    EXPECT_EQ(expected_block,block);

    // next blcks: empty
    block = dir.nextBlock();
    expected_block.clear();
    EXPECT_EQ(expected_block,block);

    FREE_ARGV(11);

}
// Testing blocks of 2 files
TEST_P(TestCase1,block2) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[13];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"2");
    INIT_ARGV(9,"--tmp-dir");
    INIT_ARGV(10,GetParam()->getTmpDir().c_str());
    INIT_ARGV(11,"--out-files");
    INIT_ARGV(12,"%out-dir%/%path%");
    
    Parameters prms(13,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);

    vector_of_strings block;
    vector_of_strings expected_block;

    // Read the files
    dir.readFiles();

        string topdir = "";
        if ( GetParam()->getDirectoryType() == "UsingBdbh" ) topdir = "inputdir1/";

    // 1st blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "A.txt");
    expected_block.push_back(topdir + "B.txt");
    EXPECT_EQ(expected_block,block);

    // 2nd blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "C/C.txt");
    expected_block.push_back(topdir + "C/C/C.txt");
    EXPECT_EQ(expected_block,block);

    // 3rd blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "D/C.txt");
    EXPECT_EQ(expected_block,block);

    // next blcks: empty
    block = dir.nextBlock();
    expected_block.clear();
    EXPECT_EQ(expected_block,block);

    FREE_ARGV(13);

}

// Testing nextBlock, block size == total number of files
TEST_P(TestCase1,block3) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[13];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"5");
    INIT_ARGV(9,"--tmp-dir");
    INIT_ARGV(10,GetParam()->getTmpDir().c_str());
    INIT_ARGV(11,"--out-files");
    INIT_ARGV(12,"%out-dir%/%path%");
    
    Parameters prms(13,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);

    vector_of_strings block;
    vector_of_strings expected_block;

    // Read the files
    dir.readFiles();

        string topdir = "";
        if ( GetParam()->getDirectoryType() == "UsingBdbh" ) topdir = "inputdir1/";

    // 1st blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "A.txt");
    expected_block.push_back(topdir + "B.txt");
    expected_block.push_back(topdir + "C/C.txt");
    expected_block.push_back(topdir + "C/C/C.txt");
    expected_block.push_back(topdir + "D/C.txt");
    EXPECT_EQ(expected_block,block);

    // next blcks: empty
    block = dir.nextBlock();
    expected_block.clear();
    EXPECT_EQ(expected_block,block);

    FREE_ARGV(13);

}

// Testing nextBlock, block size > total number of files
TEST_P(TestCase1,block4) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[13];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"10");
    INIT_ARGV(9,"--tmp-dir");
    INIT_ARGV(10,GetParam()->getTmpDir().c_str());
    INIT_ARGV(11,"--out-files");
    INIT_ARGV(12,"%out-dir%/%path%");

    Parameters prms(13,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);

    vector_of_strings block;
    vector_of_strings expected_block;

    // Read the files
    dir.readFiles();

        string topdir = "";
        if ( GetParam()->getDirectoryType() == "UsingBdbh" ) topdir = "inputdir1/";

    // 1st blck
    block = dir.nextBlock();
    expected_block.clear();
    expected_block.push_back(topdir + "A.txt");
    expected_block.push_back(topdir + "B.txt");
    expected_block.push_back(topdir + "C/C.txt");
    expected_block.push_back(topdir + "C/C/C.txt");
    expected_block.push_back(topdir + "D/C.txt");
    EXPECT_EQ(expected_block,block);

    // next blcks: empty
    block = dir.nextBlock();
    expected_block.clear();
    EXPECT_EQ(expected_block,block);

    FREE_ARGV(13);

}

// Testing executeExternalCommand
TEST_P(TestCase1,ExternalCommand) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");

    Parameters prms(11,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);

    string f,cmd;

    // create output directory and temp directory
    dir.setRank(0,1);
    dir.makeOutDir(false,true);
    dir.makeTempOutDir();

    // input, output files
    vector_of_strings output_files;
    vector_of_strings input_files;
        string topdir = "";
        if ( GetParam()->getDirectoryType() == "UsingBdbh" ) topdir = "inputdir1/";

    output_files.push_back(topdir + "A.out");  // creating output_files[0]
    input_files.push_back(topdir + "A.txt");   // creating input_files[0]

    cmd = "./coucou.sh";
    int rvl=0;
    cerr << "10 LINES WILL BE WRITTEN TO STDERR - THIS IS NORMAL BEHAVIOUR\n";
    EXPECT_NO_THROW(rvl=dir.executeExternalCommand(input_files,cmd,output_files));
    EXPECT_EQ(127,rvl);

    input_files[0] = topdir + "Z.txt";  // input file does not exist, should throw an exception !
    EXPECT_THROW(rvl=dir.executeExternalCommand(input_files,cmd,output_files),exception);

    input_files[0] = topdir + "B.txt";
    cmd = "./ext_cmd.sh " + dir.getTempInDir() + '/' + input_files[0] + " " + dir.getTempOutDir() + '/' + input_files[0];
    EXPECT_EQ(0,dir.executeExternalCommand(input_files,cmd,output_files));
//    if ( GetParam()->getDirectoryType() == "UsingBdbh" ) exit(1);

    // Should return 1 (this file is in "error")
    input_files[0] = topdir + "D/C.txt";
    output_files[0]= input_files[0];
    cmd = "./ext_cmd.sh " + dir.getTempInDir() + '/' + input_files[0] + " " + dir.getTempOutDir() + '/' + output_files[0];
    EXPECT_EQ(1,dir.executeExternalCommand(input_files,cmd,output_files));

    FREE_ARGV(11);
}

/** 
 * @brief Convenient function: converting from a table of ints to a vector of strings
 * 
 * @param tab 
 * @param s 
 * 
 * @return 
 */
vector_of_strings int2strings(int* tab,size_t s) {
    
    vector_of_strings rvl;
    ostringstream tmp;
    for (size_t i=0; i<s; i++) {
        tmp.str("");
        if (tab[i]>=0) {
            tmp << tab[i];
        };
        rvl.push_back(tmp.str());
    }
    return rvl;
}

void initFinfo(list<Finfo> & f_i, size_t s) {
    ostringstream tmp;
    off_t sze=5000;
    for (int i=s-1; i>-1; --i) {
        tmp.str("");
        tmp << i;
        f_i.push_back(Finfo(tmp.str(),sze));
        sze += 100;
    }
}

// testing buildBlocks
//         distribution in blocks size 5, from a sorted array, size 40
TEST_P(TestCase1,SortFiles1) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[13];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"5");
    INIT_ARGV(9,"--tmp-dir");
    INIT_ARGV(10,GetParam()->getTmpDir().c_str());
    INIT_ARGV(11,"--out-files");
    INIT_ARGV(12,"%out-dir%/%path%");

    Parameters prms(13,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);
    dir.setRank(0,5);

    vector_of_strings files;
    list<Finfo> f_info;
    initFinfo(f_info,40);
    dir.buildBlocks(f_info,files);

    int tmp[] = {0,4,8,12,16,1,5,9,13,17,2,6,10,14,18,3,7,11,15,19,20,24,28,32,36,21,25,29,33,37,22,26,30,34,38,23,27,31,35,39};
    vector_of_strings expected_files = int2strings(tmp,40);
    EXPECT_EQ(expected_files,files);

    FREE_ARGV(13);
}

// testing buildBlocks
//         distribution in blocks size 1, from a sorted array, size 16
TEST_P(TestCase1,SortFiles3) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[13];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"1");
    INIT_ARGV(9,"--tmp-dir");
    INIT_ARGV(10,GetParam()->getTmpDir().c_str());
    INIT_ARGV(11,"--out-files");
    INIT_ARGV(12,"%out-dir%/%path%");
    
    Parameters prms(13,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);
    dir.setRank(0,5);

    vector_of_strings files;
    list<Finfo> f_info;
    initFinfo(f_info,13);
    dir.buildBlocks(f_info,files);

    int tmp[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,-1,-1,-1};
    vector_of_strings expected_files = int2strings(tmp,16);
    EXPECT_EQ(expected_files,files);

    FREE_ARGV(13);
}

// testing buildBlocks
//         distribution in blocks size 5, from a sorted array, size 33
TEST_P(TestCase1,SortFiles2) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[13];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"5");
    INIT_ARGV(9,"--tmp-dir");
    INIT_ARGV(10,GetParam()->getTmpDir().c_str());
    INIT_ARGV(11,"--out-files");
    INIT_ARGV(12,"%out-dir%/%path%");

    Parameters prms(13,argv);
    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();
//    UsingFs dir(prms);
    dir.setRank(0,5);

    vector_of_strings files;
    list<Finfo> f_info;
    initFinfo(f_info,33);
    dir.buildBlocks(f_info,files);

    int tmp[] = {0,4,8,12,16,1,5,9,13,17,2,6,10,14,18,3,7,11,15,19,20,24,28,32,-1,21,25,29,-1,-1,22,26,30,-1,-1,23,27,31,-1,-1};
    vector_of_strings expected_files = int2strings(tmp,40);
    EXPECT_EQ(expected_files,files);

    FREE_ARGV(13);
}

// testing file type dir
// testing buildBlocks
//         distribution in blocks size 5, from a sorted array, size 33
TEST_P(TestCase4,filetypedir) {
    cout << GetParam()->getDescription() << '\n';

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"directories_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"touch out.txt");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"dir");
    INIT_ARGV(7,"--tmp-dir");
    INIT_ARGV(8,GetParam()->getTmpDir().c_str());
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");

    Parameters prms(11,argv);

    unique_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
    Directories& dir = *aptr_dir.get();

    if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
        EXPECT_THROW(dir.setRank(0,2),runtime_error);
    } else {
        EXPECT_NO_THROW(dir.setRank(0,2));
    
        vector_of_strings files = dir.getFiles();
        vector_of_strings expected_input_files;
        
        expected_input_files.push_back("0.dir");
        expected_input_files.push_back("0.dir/1.dir");
        expected_input_files.push_back("3.dir");
        expected_input_files.push_back("4.dir");
        expected_input_files.push_back("C/2.dir");
        EXPECT_EQ(expected_input_files,files);
    }
    
    FREE_ARGV(11);
}

// Calling "none" for tmp-dir is just a trick: "none" does not exist, so there is NO tmp-dir
// If you do NOT specify --tmp-dir, you get the default tmpdir, which may - or not - be defined
// See parameters.cpp
// If you create "none" with "mkdir none" you are no more "NoTmp" and the test fails !

unique_ptr<ChdbTestsWithParamsUsingFs> test_case_Fs_notmp   (new ChdbTestsWithParamsUsingFs("none"));
unique_ptr<ChdbTestsWithParamsUsingFs> test_case_Fs_withtmp (new ChdbTestsWithParamsUsingFs("."));

INSTANTIATE_TEST_CASE_P(
    tmpOrNotSeveralDirectories,
    TestCase1,
    Values(test_case_Fs_withtmp.get(),test_case_Fs_notmp.get())
);

// REMOVED BECAUSE bdbh WAS REMOVED
#if false
unique_ptr<ChdbTestsWithParamsUsingBdbh> test_case_Bdbh_withtmp (new ChdbTestsWithParamsUsingBdbh("."));

INSTANTIATE_TEST_CASE_P(
    tmpOrNotSeveralDirectories,
    TestCase1,
    Values(test_case_Fs_withtmp.get(),test_case_Bdbh_withtmp.get(),test_case_Bdbh_withtmp.get())
//    Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get(),test_case_Bdbh_withtmp.get())
);
INSTANTIATE_TEST_CASE_P(
    tmpOrNotSeveralDirectories,
    TestCase4,
    Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get(),test_case_Bdbh_withtmp.get())
    //Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get(),test_case_Bdbh_withtmp.get());
);
#endif
