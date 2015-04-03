
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
#include "system.hpp"
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
	OPT_INFILE,     // --in-file
	OPT_OUTDIR,     // --out-dir
	OPT_WORKDIR,    // --work-dir
	OPT_ENV_SNIPPET,// --create-environment
	OPT_TMPDIR,     // --tmp-dir
	OPT_OUTFILES,   // --out-files
	OPT_BLOCK_SIZE, // --block-size,
	OPT_SORT_BY_SIZE,  // --sort-by-size
	OPT_VERBOSE,       // --verbose
	OPT_ON_ERROR,      // --on-error
	OPT_REPORT,        // --report
	OPT_IN_TYPE,       // --in-type
	OPT_CMD_IS_MPI,    // --command-is-mpi
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
	{ OPT_INFILE,        "--in-files",     SO_REQ_SEP },
	{ OPT_OUTDIR,        "--out-dir",      SO_REQ_SEP },
	{ OPT_WORKDIR,       "--work-dir",     SO_REQ_SEP },
	{ OPT_ENV_SNIPPET,   "--create-environment", SO_REQ_SEP },
	{ OPT_TMPDIR,        "--tmp-dir",      SO_REQ_SEP },
	{ OPT_OUTFILES,      "--out-files",    SO_REQ_SEP },
	{ OPT_BLOCK_SIZE,    "--block-size",   SO_REQ_SEP },
	{ OPT_SORT_BY_SIZE,  "--sort-by-size", SO_NONE    },
	{ OPT_VERBOSE,       "--verbose",      SO_NONE    },
	{ OPT_ON_ERROR,      "--on-error",     SO_REQ_SEP },
	{ OPT_REPORT,        "--report",       SO_REQ_SEP },
	{ OPT_IN_TYPE,       "--in-type",      SO_REQ_SEP },
	{ OPT_CMD_IS_MPI,    "--command-is-mpi", SO_REQ_SEP },
	{ OPT_CMD,           "--command-line", SO_REQ_SEP },
	SO_END_OF_OPTIONS   // END
};

/*
  NOTMP support
  
  If the macro NOTMP is defined, it is not possible to modify the tmp value with the --tmp-dir switch
  However, this switch is still recognized, for compatibility reasons
*/

/* The default values */
#ifdef NOTMP
#define DEFAULT_TMP_DIRECTORY ""
#else
#define DEFAULT_TMP_DIRECTORY "."
#endif

#define DEFAULT_VERBOSE    false
#define DEFAULT_SIZE_SORT  false
#define DEFAULT_BLOCK_SIZE 1

/** The constructor

	\param argc   passed to main by the system
	\param argv   passed to main by the system

*/

Parameters::Parameters(int argc, 
					   char* argv[]) throw(runtime_error) :
    tmp_directory(DEFAULT_TMP_DIRECTORY),
	is_bdbh(false),
	is_size_sort(DEFAULT_SIZE_SORT),
	is_verbose(DEFAULT_VERBOSE),
	block_size(DEFAULT_BLOCK_SIZE) {

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
			case OPT_INFILE:
				input_file = arguments.OptionArg();
				break;
			case OPT_OUTDIR:
				output_directory =  arguments.OptionArg();
				break;
			case OPT_WORKDIR:
				work_directory = arguments.OptionArg();
				break;
			case OPT_ENV_SNIPPET:
				env_snippet = arguments.OptionArg();
				break;
			case OPT_TMPDIR:
#ifndef NOTMP
				tmp_directory = arguments.OptionArg();
#endif
				break;
			case OPT_OUTFILES:
				output_files = split(arguments.OptionArg());
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
			case OPT_REPORT:
				report = arguments.OptionArg();
				break;
			case OPT_IN_TYPE:
				file_type = arguments.OptionArg();
				break;
			case OPT_CMD_IS_MPI:
				cmd_is_mpi = arguments.OptionArg();
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
	//checkOutputDirectory();
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
		throw runtime_error(runtime_error(msg));
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
			throw runtime_error(runtime_error(msg));
		}
	}
}
/*
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
*/

void Parameters::usage() {
	cerr << "Calcul à Haut DéBit - version " << CHDB_VERSION << "\n";
	cerr << "Copyright license todo\n";

	if (isVerbose()) {
#ifdef NOTMP
		cerr << "DNOTMP ";
#else
		cerr << "UNOTMP ";
#endif
#ifdef OUTDIRPERSLAVE
		cerr << "DOUTDIRPERSLAVE\n";
#else
		cerr << "UOUTDIRPERSLAVE\n";
#endif
	}
	
	cerr << "chdb executes an external command for every file of a given file-type found in the input hierarchy.\n";
	cerr << "Results are saved to the output hierarchy.\n";
	cerr << "The work is distributed among the cores using the MPI library.\n\n";
	cerr << "Usage: mpirun -n N ... chdb parameters switches ..." << '\n';
	cerr << "\n";
	cerr << "REQUIRED PARAMETERS:\n";
	cerr << "  --in-dir inputdir          : Input files are looked for in this directory.\n";
	cerr << "  --in-type ext              : Only filenames terminating with this extension will be considered for input\n";
	cerr << "  --command-line '...'       : The command line to be executed on each input file (see the allowed templates under)\n";
	cerr << "  --out-files file1,file2,...: A list of output files created by the command-line (see the allowed templates under)\n";
	cerr << "                               NOTE: if several output files are generated for each command, the names must be separated by a comma (,)\n";
	cerr << "\n";
	cerr << "OPTIONAL PARAMETERS:\n";
	cerr << "  --out-dir outdir           : All output will be written to this directory. Default = inputdir.out\n";
	cerr << "  --work-dir workdir         : Change to this directory before executing command\n";
	cerr << "                               WARNING ! \n";
	cerr << "                                  - a RELATIVE path specified from --command will be treated FROM THIS DIRECTORY\n";
	cerr << "                                  - a RELATIVE PATH specified from ANY OTHER SWITCH will be treated FROM THE INITIAL LAUNCH DIRECTORY\n";
	cerr << "  --create-environment       : A snippet containing some shell commands to create a working environment inside the work directory \n";
	cerr << "                               Will be executed AFTER chdir workdir and BEFORE the command itself\n";
	cerr << "                               EXAMPLE:\n";
	cerr << "                                  --create-environment 'cp ~/DATA/*.inp .; cp ~/DATA/*.conf .'\n";
	cerr << "  --block-size 10            : The higher the block-size, the less mpi communications, but you may get\n";
	cerr << "                               load-balancing issues\n";
	cerr << "  --on-error errors.txt      : When the command returns something different from 0, the status and the file path \n";
	cerr << "                               are stored in this file for later reference and execution\n";
	cerr << "                               NOTE: The default is to INTERRUPT chdb when the return status is not 0\n";
	cerr << "  --in-files file.txt        : Only files whose path is inside file.txt are considered for input\n";
	cerr << "                               Format: One path per line\n";
	cerr << "                               NOTE: A generated errors.txt (cf. --on-error) may be specified as in-files parameter \n";
	cerr << "  --report report.txt        : Generate a report with some timing info about the command (use only for debug !)\n";
	cerr << "  --command_is_mpi n         : The command is itself an mpi program, n is the number of mpi processes\n";
	cerr << "\n";
	cerr << "OPTIONAL SWITCHES:\n";
	cerr << "  --sort-by-size             : Sort the input files the bigger first, may be less load balancing issues\n";
	cerr << "  --verbose                  : Some messages are printed\n";
	cerr << "  --help                     : Print this screen and leave\n";
	cerr << "\n";
	cerr << "TEMPLATES ALLOWED IN FILE NAMES (--command, --out-files, --work-dir):\n";
	cerr << "The following templates are allowed in filenames specified in parameters command-line and out-files.\n";
	cerr << "They are expanded using the real input file. We suppose that the input file is inputdir/A/B/toto.txt:\n";
	cerr << "\n";
	cerr << "  %in-dir%       The input directory  (inputdir)\n";
	cerr << "  %out-dir%      The output directory (inputdir.out)\n";
	cerr << "  %path%         The input file complete path (relative to the input directory: A/B/toto.txt)\n";
	cerr << "  %name%         The file name with the extension (toto.txt)\n";
	cerr << "  %basename%     The file name without the extension (toto)\n";
	cerr << "  %dirname%      The directory name relative to the input or output directory (A/B)\n";
	throw ParametersHelp();
}

string Parameters::getLastErrorText(const CSimpleOpt& arg) {
	char* r=arg.OptionArg();
	string rvl;
	if (r!=NULL) rvl=r;

	rvl += ' ';
	switch (arg.LastError()) {
	case SO_SUCCESS:            rvl="Success"; break;
	case SO_OPT_INVALID:        rvl += "Unrecognized option"; break;
	case SO_OPT_MULTIPLE:       rvl += "Option matched multiple strings"; break;
	case SO_ARG_INVALID:        rvl += "Option does not accept argument"; break;
	case SO_ARG_INVALID_TYPE:   rvl += "Invalid argument format"; break;
	case SO_ARG_MISSING:        rvl += "Required argument is missing"; break;
	case SO_ARG_INVALID_DATA:   rvl += "Invalid argument data"; break;
	default:                    rvl += "Unknown error"; break;
	}
	return rvl;
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
