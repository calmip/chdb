
#include <iostream>
//#include <iterator>
//#include <set>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

//#include "command.h"
#include "parameters.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
//#include <libgen.h>
//#include <stdlib.h>


/**
   \brief 
*/

// define the ID values to identify the option
enum { 
	OPT_HELP=1,     // -h|--help
	OPT_INDIR,      // --in-dir
	OPT_OUTDIR,     // --out-dir
	OPT_OUTFILES,   // --out-files
	OPT_BLOCK_SIZE, // --block-size,
	OPT_SORT_BY_SIZE,  // --sort-by-size
	OPT_VERBOSE,       // --verbose
	OPT_ON_ERROR,      // --on-error
	OPT_IN_TYPE,       // --in-type
	OPT_CMD            // --command-line
};

// declare a table of CSimpleOpt::SOption structures. See the SimpleOpt.h header
// for details of each entry in this structure. In summary they are:
//  1. ID for this option. This will be returned from OptionId() during processing.
//     It may be anything >= 0 and may contain duplicates.
//  2. Option as it should be written on the command line
//  3. Type of the option. See the header file for details of all possible types.
//     The SO_REQ_SEP type means an argument is required and must be supplied
//     separately, e.g. "-f FILE"
//  4. The last entry must be SO_END_OF_OPTIONS.
//
CSimpleOpt::SOption options[] = {
	{ OPT_HELP,          "--help",         SO_NONE    },
	{ OPT_INDIR,         "--in-dir",       SO_REQ_SEP },
	{ OPT_OUTDIR,        "--out-dir",      SO_REQ_SEP },
	{ OPT_OUTFILES,      "--out-files",    SO_REQ_SEP },
	{ OPT_BLOCK_SIZE,    "--block-size",   SO_REQ_SEP },
	{ OPT_SORT_BY_SIZE,  "--sort-by-size", SO_NONE    },
	{ OPT_VERBOSE,       "--verbose",      SO_NONE    },
	{ OPT_ON_ERROR,      "--on-error",     SO_REQ_SEP },
	{ OPT_IN_TYPE,       "--in-type",      SO_REQ_SEP },
	{ OPT_CMD,           "--command-line", SO_REQ_SEP },
	SO_END_OF_OPTIONS   // END
};

/** The constructor

	\param argc   passed to main by the system
	\param argv   passed to main by the system

*/

Parameters::Parameters(int argc, 
					   char* argv[]) throw(runtime_error) :
	is_bdbh(false),
	is_size_sort(false),
	is_verbose(false),
	block_size(1) {

	// Searching the --command-line argument
	CSimpleOpt arguments(argc, argv, options);
    while (arguments.Next()) {
//		cerr << arguments.LastError() << "  " << arguments.OptionId() << '\n';
		if (arguments.LastError() != SO_SUCCESS) {
			throw runtime_error(getLastErrorText(arguments));
		}
		else
		{
			switch (arguments.OptionId()) {
			case OPT_HELP:
				usage();
				break;
			case OPT_INDIR:
				input_directory = arguments.OptionArg();
				break;
			case OPT_OUTDIR:
				output_directory =  arguments.OptionArg();
				break;
			case OPT_OUTFILES:
				output_files.push_back(arguments.OptionArg());
				break;
			case OPT_BLOCK_SIZE:
				block_size = atoi(arguments.OptionArg());
				break;
			case OPT_SORT_BY_SIZE:
				is_size_sort = true;
				break;
			case OPT_VERBOSE:
				is_verbose = true;
				break;
			case OPT_ON_ERROR:
				on_error = arguments.OptionArg();
				break;
			case OPT_IN_TYPE:
				file_type = arguments.OptionArg();
				break;
			case OPT_CMD:
				external_command = arguments.OptionArg();
				break;
			}
		}
	}

	// complete parameters if necessary
	if ( output_directory == "" ) {
		output_directory = input_directory + ".out";
	}

	// check everything is ok
	check();
}

/**
   \brief throw a runtime_error is there is something wrong with the parameters
*/

void Parameters::check() {
	checkEmptyMembers();
	checkInputDirectory();
	checkOutputDirectory();
	checkBlockSize();
}
void Parameters::checkBlockSize() {
	if (getBlockSize() <= 0 ) {
		throw runtime_error("ERROR - The parameter --block-size hould be > 0");
	}
}
void Parameters::checkEmptyMembers() {
	if ( external_command == "" ) {
		throw runtime_error("ERROR - The parameter --command-line is required");
	}
	if ( input_directory=="" ) {
		throw runtime_error("ERROR - The parameter --in-dir is required");
	}
	if ( file_type=="") {
		throw runtime_error("ERROR - The parameter --in-type is required");
	}
}
void Parameters::checkInputDirectory() {
	struct stat bfr;
	int rvl;

	rvl = stat(input_directory.c_str(),&bfr);
	if (rvl != 0) {
		string msg = "ERROR - Cannot open file or directory ";
		msg += input_directory;
		msg += '\n';
		msg += strerror(errno);
		throw runtime_error(msg);
	}
	else
	{
		if ( S_ISREG(bfr.st_mode) ) {
			// TODO --> verifier que ca peut s'ouvrir avec bdbh !!!
			is_bdbh = true;
		}
		else if ( !S_ISDIR(bfr.st_mode) && !S_ISLNK(bfr.st_mode)) {
			string msg = "ERROR - ";
			msg += input_directory;
			msg += " should be a directory, a symlink, or a bdbh file";
			throw runtime_error(msg);
		}
	}
}
void Parameters::checkOutputDirectory() {
	struct stat bfr;
	int rvl;

	rvl = stat(output_directory.c_str(),&bfr);
	if (rvl == 0) {
		string msg = "ERROR - The output directory ";
		msg += output_directory;
		msg += " already exists !";
		throw runtime_error(msg);
	}
}
void Parameters::usage() {
	cerr << "Usage: chdb bla bla" << '\n';
	exit(1);
}

string Parameters::getLastErrorText(const CSimpleOpt& arg) {
	switch (arg.LastError()) {
	case SO_SUCCESS:            return "Success";
	case SO_OPT_INVALID:        return "Unrecognized option";
	case SO_OPT_MULTIPLE:       return "Option matched multiple strings";
	case SO_ARG_INVALID:        return "Option does not accept argument";
	case SO_ARG_INVALID_TYPE:   return "Invalid argument format";
	case SO_ARG_MISSING:        return "Required argument is missing";
	case SO_ARG_INVALID_DATA:   return "Invalid argument data";
	default:                    return "Unknown error";
	}
}

/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/
