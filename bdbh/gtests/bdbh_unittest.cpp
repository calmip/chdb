
/**
   unit tests for the create bdbh api functions
*/


#include "constypes_unittest.hpp"
#include <memory>
#include "../create.hpp"
#include "../put.hpp"
#include "../write.hpp"
#include "../read.hpp"
#include "../merge.hpp"

#include "../exception.hpp"
#include <stdexcept>

#include "gtest/gtest.h"
using ::testing::TestWithParam;
using ::testing::Values;

// Create a database whose name is passed by parameter
void CreateDb(const string& database, bool compress) {
	removeFile(database);
	bdbh::BerkeleyDb bdb(database.c_str(),BDBH_OCREATE);
	vector<string> args;
	if (compress) args.push_back("--compress");
	bdbh::Parameters prms_create(args);
	bdbh::Create create(prms_create,bdb);
	create.Exec();
}

// Create, not compressed, permission denied
// NOTE - CreateDb is not used here
TEST_F(BdbhTest,create_ko) {
	// Declare bdb-related objects
	auto_ptr<bdbh::BerkeleyDb> bdb;
	auto_ptr<bdbh::Parameters> prms;
	auto_ptr<bdbh::Create> cmd;
	
	// Create and open an output BerkeleyDb
	string db_dir = "/";
	db_dir += getDbDir();
	ASSERT_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OCREATE),exception);
}

// Fill a database whose name is passed by parameter
// (should be already created)
void FillDb(const string& database, const string& root="") {
	bdbh::BerkeleyDb bdb(database.c_str(),BDBH_OWRITE);
	vector<string> args;
	if (root != "") {
		args.push_back("--root");
		args.push_back(root);
	}
	args.push_back("--recursive");
	args.push_back(INPUTDIR1);
	bdbh::Parameters prms_add(args);
	bdbh::Write write(prms_add,bdb);
	write.Exec();
}

// Extract a database whose name is passed by parameter to a directory passed by parameter also
void ExtractDb(const string& database, const string& directory) {
	bdbh::BerkeleyDb bdb(database.c_str(),BDBH_OREAD);
	vector<string> args;
	args.push_back("--recursive");
	args.push_back("--directory");
	args.push_back(directory);
	args.push_back("*");
	bdbh::Parameters prms_read(args);
	bdbh::Read read(prms_read,bdb);
	read.Exec();
}

// Create and Info, no compression
// NOTE - CreateDb is not used here
TEST_F(BdbhTest,create_ok) {

	// Declare bdb-related objects
	auto_ptr<bdbh::BerkeleyDb> bdb;
	auto_ptr<bdbh::Parameters> prms;
	auto_ptr<bdbh::Parameters> prms_info;
	auto_ptr<bdbh::Create> create;
	auto_ptr<bdbh::Info> info;
	auto_ptr<bdbh::Mkdir> mkdir;
	
	
	// Create a BerkeleyDb
	string db_dir = getDbDir();
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OCREATE));

	// Create a parameters object
	vector<string> args;
	ASSERT_NO_THROW(prms = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));

	// Create and execute the command
	ASSERT_NO_THROW(create = (auto_ptr<bdbh::Create>) new bdbh::Create(*prms.get(),*bdb.get()));

	// execute the command
	ASSERT_NO_THROW(create.get()->Exec());
	ASSERT_EQ(0,create.get()->GetExitStatus());
	bdb.reset();
	create.reset();

	// The file database should exist
	ASSERT_EQ(true,existsFile(db_dir+"/database"));

	// Should not be able to open the database in BDBH_OCREATE, as it already exists
	ASSERT_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OCREATE),exception);

	// Should not be able to create a database if opened in WRITE mode
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OWRITE));
	ASSERT_THROW(create = (auto_ptr<bdbh::Create>) new bdbh::Create(*prms.get(),*bdb.get()),logic_error);

	// Get the info, should be OK in WRITE mode
	ASSERT_NO_THROW(prms_info = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters());
	ASSERT_NO_THROW(info = (auto_ptr<bdbh::Info>) new bdbh::Info(*prms_info.get(),*bdb.get()));
	info->InhibitPrinting();

	// get some info -->  thrown (too early)
	ASSERT_THROW(info.get()->GetInfoData(),logic_error);

	// Execute Info and get some information
	ASSERT_NO_THROW(info.get()->Exec());
	ASSERT_EQ(0,info.get()->GetExitStatus());

	bdbh::InfoData info_data = info.get()->GetInfoData();
	ASSERT_EQ(false,info_data.data_compressed);
	ASSERT_EQ(info_data.date_created,info_data.date_modified);
	ASSERT_EQ(0,info_data.max_data_size_uncompressed);
	ASSERT_EQ(0,info_data.nb_of_files);
	ASSERT_EQ(0,info_data.nb_of_dir);

	// create a directory and check info again !
	bdb.reset();
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OWRITE));

	args.clear();
	args.push_back("TEST");

	ASSERT_NO_THROW(prms = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(mkdir = (auto_ptr<bdbh::Mkdir>) new bdbh::Mkdir(*prms.get(),*bdb.get()));
	ASSERT_NO_THROW(mkdir.get()->Exec());

	bdb.reset();
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OREAD));
	ASSERT_NO_THROW(info = (auto_ptr<bdbh::Info>) new bdbh::Info(*prms_info.get(),*bdb.get()));
	info->InhibitPrinting();
	ASSERT_NO_THROW(info.get()->Exec());
	info_data = info.get()->GetInfoData();	

	// info changed a little
	ASSERT_EQ(false,info_data.data_compressed);
	ASSERT_EQ(0,info_data.max_data_size_uncompressed);
	ASSERT_EQ(0,info_data.nb_of_files);
	ASSERT_EQ(1,info_data.nb_of_dir);
};

// Create and Info, compressed
// NOTE - CreateDb is not used here
TEST_F(BdbhTest,compressed) {

	// Declare bdb-related objects
	auto_ptr<bdbh::BerkeleyDb> bdb;
	auto_ptr<bdbh::Parameters> prms;
	auto_ptr<bdbh::Create>     create;
	auto_ptr<bdbh::Info>       info;
	
	// Create and open an output BerkeleyDb
	string db_dir = getDbDir();
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OCREATE));

	// Create a parameters object
	vector<string> args;
	args.push_back("--compress");
	ASSERT_NO_THROW(prms = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));

	// Create and execute the command
	ASSERT_NO_THROW(create = (auto_ptr<bdbh::Create>) new bdbh::Create(*prms.get(),*bdb.get()));
	ASSERT_NO_THROW(create->Exec());
	ASSERT_EQ(0,create->GetExitStatus());

	// The file database should exist
	ASSERT_EQ(true,existsFile(db_dir+"/database"));

	// close and repoen the database
	bdb.reset();
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OREAD));

	// Create and execute the Info command
	args.clear();
	ASSERT_NO_THROW(prms = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));

	// Create the command
	ASSERT_NO_THROW(info = (auto_ptr<bdbh::Info>) new bdbh::Info(*prms.get(),*bdb.get()));
	info->InhibitPrinting();

	// Execute Info and get some information
	ASSERT_NO_THROW(info->Exec());
	ASSERT_EQ(0,info->GetExitStatus());

	bdbh::InfoData info_data = info.get()->GetInfoData();
	ASSERT_EQ(true,info_data.data_compressed);
};

// Add
// NOTE - FillDb is NOT used here
TEST_P(TestCase1,add) {

	// Declare bdb-related objects
	auto_ptr<bdbh::BerkeleyDb> bdb;
	auto_ptr<bdbh::Parameters> prms_add;
	auto_ptr<bdbh::Write>      write;
	string db_dir = getDbDir();
	vector<string> args;
	bdbh::InfoData info_data;

	// add inputdir1 to the database
	// cout << "TESTING - add " << (GetParam()?"COMPRESSED":"NOT COMPRESSED") << "\n";
	CreateDb(db_dir,GetParam());
	args.clear();
	args.push_back(INPUTDIR1);
	ASSERT_NO_THROW(prms_add = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OWRITE));
	ASSERT_NO_THROW(write = (auto_ptr<bdbh::Write>) new bdbh::Write(*prms_add.get(),*bdb.get()));
	ASSERT_NO_THROW(write->Exec());
	ASSERT_EQ(0,write->GetExitStatus());

	// The second time, it still works, because it is just a directory, and --recursive not specified
	ASSERT_NO_THROW(write->Exec());
	ASSERT_EQ(0,write->GetExitStatus());
	
	bdb.reset();

	// Check result with info
	info_data = bdbh::GetInfo(db_dir);
	ASSERT_EQ(GetParam(),info_data.data_compressed);
	ASSERT_EQ(1,info_data.nb_of_dir);
	ASSERT_EQ(0,info_data.nb_of_files);

	// add inputdir, with recursivity enabled
	removeFile(db_dir);
	CreateDb(db_dir,GetParam());

	args.clear();
	args.push_back("--recursive");
	args.push_back(INPUTDIR1);
	ASSERT_NO_THROW(prms_add = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OWRITE));
	ASSERT_NO_THROW(write = (auto_ptr<bdbh::Write>) new bdbh::Write(*prms_add.get(),*bdb.get()));
	ASSERT_NO_THROW(write->Exec());
	ASSERT_EQ(0,write->GetExitStatus());

	// The second time, it should return 10 (files not overloaded)
	ASSERT_NO_THROW(write->Exec());
	ASSERT_EQ(10,write->GetExitStatus());

	// Try again with --overwrite
	args.push_back("--overwrite");
	ASSERT_NO_THROW(prms_add = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(write = (auto_ptr<bdbh::Write>) new bdbh::Write(*prms_add.get(),*bdb.get()));
	ASSERT_NO_THROW(write->Exec());
	ASSERT_EQ(0,write->GetExitStatus());

	// add something which does not exist: we get a BdbhException
	args.clear();
	args.push_back("THIS_FILE_DOES_NOT_EXIST");
	ASSERT_NO_THROW(prms_add = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(write = (auto_ptr<bdbh::Write>) new bdbh::Write(*prms_add.get(),*bdb.get()));
	ASSERT_THROW(write->Exec(),bdbh::BdbhException);

	// Check that a BdbhException is ALSO a runtime_error exception
	ASSERT_THROW(write->Exec(),runtime_error);

	bdb.reset();

	// Check result with info
	info_data = bdbh::GetInfo(db_dir);
	ASSERT_EQ(GetParam(),info_data.data_compressed);
	ASSERT_EQ(4,info_data.nb_of_dir);
	ASSERT_EQ(9,info_data.nb_of_files);

	// add only a subset, in the SAME database
	// add -r C --directory inputdir1
	args.clear();
	args.push_back("--recursive");
	args.push_back("--directory");
	args.push_back(INPUTDIR1);
	args.push_back("C");
	ASSERT_NO_THROW(prms_add = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OWRITE));
	ASSERT_NO_THROW(write = (auto_ptr<bdbh::Write>) new bdbh::Write(*prms_add.get(),*bdb.get()));
	ASSERT_NO_THROW(write->Exec());
	ASSERT_EQ(0,write->GetExitStatus());
	bdb.reset();

	// Check result with info
	info_data = bdbh::GetInfo(db_dir);
	ASSERT_EQ(GetParam(),info_data.data_compressed);
	ASSERT_EQ(6,info_data.nb_of_dir);
	ASSERT_EQ(11,info_data.nb_of_files);

	// add the same subset, in the SAME database, with a root
	// add -r C --directory inputdir1 --root C1
	args.clear();
	args.push_back("--recursive");
	args.push_back("C");
	args.push_back("--directory");
	args.push_back(INPUTDIR1);
	args.push_back("--root");
	args.push_back("C1");
	ASSERT_NO_THROW(prms_add = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OWRITE));
	ASSERT_NO_THROW(write = (auto_ptr<bdbh::Write>) new bdbh::Write(*prms_add.get(),*bdb.get()));
	ASSERT_NO_THROW(write->Exec());
	// GetExitStatus
	bdb.reset();

	// Check result with info
	info_data = bdbh::GetInfo(db_dir);
	ASSERT_EQ(GetParam(),info_data.data_compressed);
	ASSERT_EQ(9,info_data.nb_of_dir);
	ASSERT_EQ(13,info_data.nb_of_files);
};

// Extract
// NOTE - ExtractDb is not used here
TEST_P(TestCase1,extract) {
	// Declare bdb-related objects
	auto_ptr<bdbh::BerkeleyDb> bdb;
	auto_ptr<bdbh::Parameters> prms_read;
	auto_ptr<bdbh::Read>      read;
	string db_dir = getDbDir();
	vector<string> args;
	bdbh::InfoData info_data;

	// add inputdir1 to the database
	CreateDb(db_dir,GetParam());
	FillDb(db_dir);

	// Check compression status with info
	info_data = bdbh::GetInfo(db_dir);
	ASSERT_EQ(GetParam(),info_data.data_compressed);

	args.clear();
	args.push_back("--directory");
	string dir = INPUTDIR1;
	dir += ".extracted";
	args.push_back(dir);
	args.push_back("--root");
	args.push_back(INPUTDIR1);
	args.push_back("--recursive");
	args.push_back("*");
	ASSERT_NO_THROW(prms_read = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(bdb = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OREAD));
	ASSERT_NO_THROW(read = (auto_ptr<bdbh::Read>) new bdbh::Read(*prms_read.get(),*bdb.get()));
	ASSERT_NO_THROW(read->Exec());
	ASSERT_EQ(0,read->GetExitStatus());

	// Try to extract a key which does not exist
	args.clear();
	args.push_back("THIS_KEY_DOES_NOT_EXIST");
	ASSERT_NO_THROW(prms_read = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
	ASSERT_NO_THROW(read = (auto_ptr<bdbh::Read>) new bdbh::Read(*prms_read.get(),*bdb.get()));
	ASSERT_NO_THROW(read->Exec());
	ASSERT_EQ(12,read->GetExitStatus());

	dir += '/';
	for (vector<naco>::iterator i=files.begin(); i!= files.end(); ++i) {
		string name = i->name;
		string content = i->content;
		ASSERT_EQ(content,readFile(dir+name));
	}
};

// Merge
TEST_P(TestCase1,merge) {
	// Declare bdb-related objects
	auto_ptr<bdbh::BerkeleyDb> bdb;
	auto_ptr<bdbh::Parameters> prms_merge;
	auto_ptr<bdbh::Merge>       merge;
	string db_dir = getDbDir();
	string db_name = db_dir+".tomerge";
	string db_extracted = INPUTDIR1;
	db_extracted += ".extracted";
	vector<string> args;
	bdbh::InfoData info_data_1;
	bdbh::InfoData info_data_2;
	bdbh::InfoData info_data_merged;

	// add inputdir1 to the database
	CreateDb(db_dir,GetParam());
	FillDb(db_dir);
	info_data_1 = bdbh::GetInfo(db_dir);

	// Try to merge a database compressed and NOT compressed
	for (int i=0; i<2; i++) {
		bool comp = i;
		// TODO - merge de deux bases, l'une compressed pas l'autre, on a se message:
		// ERROR - All databases must have the same compression status
		if (comp != GetParam()) continue;

		string db_name_1 = db_name + (comp?".COMP":".NOCOMP");
		string db_extracted_1 = db_extracted + (comp?".COMP":".NOCOMP");

		// add inputdir1 in a root to some other database
		CreateDb(db_name_1,comp);
		FillDb(db_name_1,"BIS");
		info_data_2 = bdbh::GetInfo(db_name_1);

		// Merge both databases
		args.clear();
		args.push_back(db_name_1);
		ASSERT_NO_THROW(prms_merge = (auto_ptr<bdbh::Parameters>) new bdbh::Parameters(args));
		ASSERT_NO_THROW(bdb   = (auto_ptr<bdbh::BerkeleyDb>) new bdbh::BerkeleyDb(db_dir.c_str(),BDBH_OWRITE));
		ASSERT_NO_THROW(merge = (auto_ptr<bdbh::Merge>) new bdbh::Merge(*prms_merge.get(),*bdb.get()));
		ASSERT_NO_THROW(merge->Exec());
		// GetExitStatus
		bdb.reset();
		info_data_merged = bdbh::GetInfo(db_dir);

		ASSERT_EQ(info_data_1.nb_of_dir+info_data_2.nb_of_dir,info_data_merged.nb_of_dir);
		ASSERT_EQ(info_data_1.nb_of_files+info_data_2.nb_of_files,info_data_merged.nb_of_files);
		ASSERT_EQ(info_data_merged.data_size_uncompressed,info_data_1.data_size_uncompressed+info_data_2.data_size_uncompressed);

		// Extract the merged database and check content
		ExtractDb(db_dir,db_extracted_1);

		string dir1 = db_extracted_1 + "/";
		dir1 += INPUTDIR1;
		dir1 += "/";
		string dir2 = db_extracted_1 + "/BIS/";
		dir2 += INPUTDIR1;
		dir2 += "/";
		for (vector<naco>::iterator i=files.begin(); i!= files.end(); ++i) {
			string name = i->name;
			string content = i->content;
			ASSERT_EQ(content,readFile(dir1+name));
			ASSERT_EQ(content,readFile(dir2+name));
		}
	}
};

INSTANTIATE_TEST_CASE_P(
	compress,
	TestCase1,
	Values(false,true);
);

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
