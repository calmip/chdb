/**
 * @file   parameters.cpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 *
 * This file is part of chdb software
 * chdb helps users to run embarrassingly parallel jobs on a supercomputer
 *
 * chdb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2015-2018 Emmanuel Courcelle
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

#include <iostream>
//#include <iterator>
//#include <set>
#include <string>
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

Parameters::Parameters(int argc,char* argv[]):
	sleep_time(0),
    tmp_directory(DEFAULT_TMP_DIRECTORY),
	is_bdbh(false),
	is_in_memory(false),
	is_size_sort(DEFAULT_SIZE_SORT),
	is_verbose(DEFAULT_VERBOSE),
	block_size(DEFAULT_BLOCK_SIZE) {
{
// define the ID values to identify the option
enum { 
	OPT_HELP=1,     // -h|--help
	OPT_INDIR,      // --in-dir
	OPT_INFILE,     // --in-file
	OPT_OUTDIR,     // --out-dir
	OPT_WORKDIR,    // --work-dir
	OPT_SLEEPTIME,	// --sleep
	OPT_ENV_SNIPPET,// --create-environment
	OPT_TMPDIR,     // --tmp-dir
	OPT_OUTFILES,   // --out-files
	OPT_BLOCK_SIZE, // --block-size
	OPT_IN_MEMORY,  // --in-memory
	OPT_SORT_BY_SIZE,  // --sort-by-size
	OPT_VERBOSE,       // --verbose
	OPT_ON_ERROR,      // --on-error
	OPT_REPORT,        // --report
	OPT_IN_TYPE,       // --in-type
	OPT_MPI_SLAVES,    // --mpi-slaves
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
	{ OPT_SLEEPTIME,	 "--sleep-time",   SO_REQ_SEP },
	{ OPT_ENV_SNIPPET,   "--create-environment", SO_REQ_SEP },
	{ OPT_TMPDIR,        "--tmp-dir",      SO_REQ_SEP },
	{ OPT_OUTFILES,      "--out-files",    SO_REQ_SEP },
	{ OPT_BLOCK_SIZE,    "--block-size",   SO_REQ_SEP },
	{ OPT_IN_MEMORY,     "--in-memory",    SO_NONE    },
	{ OPT_SORT_BY_SIZE,  "--sort-by-size", SO_NONE    },
	{ OPT_VERBOSE,       "--verbose",      SO_NONE    },
	{ OPT_ON_ERROR,      "--on-error",     SO_REQ_SEP },
	{ OPT_REPORT,        "--report",       SO_REQ_SEP },
	{ OPT_IN_TYPE,       "--in-type",      SO_REQ_SEP },
	{ OPT_MPI_SLAVES,    "--mpi-slaves",   SO_REQ_SEP },
	{ OPT_CMD,           "--command-line", SO_REQ_SEP },
	SO_END_OF_OPTIONS   // END
};

	// Searching the --command-line argument
	CSimpleOpt arguments(argc, argv, options);
    while (arguments.Next()) {
		//cerr << arguments.LastError() << "  " << arguments.OptionId() << " " << arguments.OptionText() << '\n';
		if (arguments.LastError() != SO_SUCCESS) {
			string msg = "ERROR: ";
			msg += arguments.OptionText();
			msg += " ";
			msg += getLastErrorText(arguments);
			throw runtime_error(msg);
		}
		else
		{
			switch (arguments.OptionId()) {
			case OPT_HELP:
				usage();
				break;
			case OPT_INDIR:
				input_directory = arguments.OptionArg();
				if (isEndingWith(input_directory,".db")) {
					is_bdbh = true;
				}
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
			case OPT_SLEEPTIME:
				sleep_time = atoi(arguments.OptionArg());
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
			case OPT_IN_MEMORY:
				is_in_memory = true;
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
				//file_type = arguments.OptionArg();
				setInputType(arguments.OptionArg());
				//is_type_dir = (file_type == "dir");
				break;
			case OPT_MPI_SLAVES:
				mpi_slaves = arguments.OptionArg();
				break;
			case OPT_CMD:
				external_command = arguments.OptionArg();
				break;
			}
		}
	}

	// complete parameters if necessary:
	//    - Default value for work directory if file type is "dir"
	//    - Default value for output directory UNLESS file type is "dir"
	//    - output_directory_db_free is used ONLY for bdbh, as we distinguish between:
	//        * data container name  (ie output_directory, input.out.db) and
	//        * top directory inside the container (ie output_directory_db_free, ie input.out)
	//    - No default value for output directory if file type is "iter"
	if ( isTypeDir() ) {
		if ( getWorkDir() == "") {
			work_directory = "%in-dir%/%path%";
		}
	};

	if ( isTypeFile() && output_directory == "" ) {
		if (isBdBh()) {
			output_directory = input_directory.substr(0,input_directory.length()-3);
			output_directory         += ".out";
			output_directory_db_free = output_directory;
			output_directory         += ".db";
		} else {
			output_directory         = input_directory + ".out";
			output_directory_db_free = output_directory;
		}
	}

	// check everything is ok
	checkParameters();
}
}

/****
  \brief Set members related to input type

  \param cft The type read from the command line
*********/
void Parameters::setInputType(const string& cft) {
	file_type = cft;
	if (file_type == "dir") {
		is_type_dir = true;
	} else {
		is_type_dir = false;
	}
	
	try {
		if ( cft.find(' ')==string::npos ) {
			is_type_iter = false;
			is_type_file = true;
		} else {
			size_t idx;
			unsigned int start=0,end=0,step=1;
			start = stoul(cft,&idx);
			// idx = number of processed characters
			if (idx != cft.length()) {
				string cft1 = cft.substr(idx);
				end = stoul(cft1, &idx);
				if (idx != cft1.length()) {
					string cft2 = cft1.substr(idx);
					step = stoul(cft2, nullptr);
				}
			}
			iter_start = start;
			iter_end   = end;
			iter_step  = step;
			is_type_iter = true;
			is_type_file = false;
		};
	}
	// As we do not check, stoul may throw an invalid_argument
	// But we want a runtime_error to be thrown...
	catch (const invalid_argument& e) {
		throw(runtime_error(e.what()));
	}
}
    

/**
   \brief throw a runtime_error is there is something wrong with the parameters
   \note  Those checks are generic - Other checks are done in Directories.
*/

void Parameters::checkParameters() {
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
	if ( file_type=="") {
		throw runtime_error("ERROR - The parameter --in-type is required");
	}
	if ( external_command == "" ) {
		throw runtime_error("ERROR - The parameter --command-line is required");
	}
	if ( input_directory=="" && ! isTypeIter()) {
		throw runtime_error("ERROR - The parameter --in-dir is required");
	}
}
void Parameters::checkInputDirectory() {
	struct stat bfr;
	int rvl;
	
	// If input directory not specified (thus is_type_iter), all ok
	if (input_directory=="") return;
	
	// Check input directory exists and is a directory
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
		//if ( S_ISREG(bfr.st_mode) ) {
		//	// TODO --> verifier que ca peut s'ouvrir avec bdbh !!!
		//	is_bdbh = true;
		//}
		if ( !S_ISDIR(bfr.st_mode) && !S_ISLNK(bfr.st_mode)) {
			string msg = "ERROR - ";
			msg += input_directory;
			msg += " should be a directory, or a symlink";
			throw runtime_error(runtime_error(msg));
		}
	}
}

/*
void Parameters::checkOutputDirectory() {
	if (isTypeIter() && output_directory=="") {
		throw runtime_error("ERROR - Output directory should be specified when asking for iterations");
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
	cerr << "  --in-type ext              : Only filenames terminating with this extension will be considered for input\n";
	cerr << "                               NOTES: The file type \"dir\" is a SPECIAL CASE:\n";
	cerr << "                                      1/ If you specify \"dir\", THE INPUT FILES SHOULD BE DIRECTORIES\n";
	cerr << "                                      2/ You CANNOT USE a \"dir\" file type while using the bdbh data cobntainer\n";
	cerr << "  --in-type 1 10             : Iteration mode: do not use input files, iterate from 1 to 10, executing the code 10 times\n";
	cerr << "  --in-type 1 10 2           : Iteration mode again, with a step = 2 (thus 5 executions only\n";
	cerr << "  --in-dir inputdir          : Input files are looked for in this directory.\n";
	cerr << "                               If dirname ends with .db (ie inputdir.db), it MUST be a bdbh data container\n";
	cerr << "                               In iteration mode, this parameter is optional\n";
	cerr << "  --command-line 'my_exe ...': The command line to be executed on each input file (see the allowed templates under)\n";
	cerr << "\n";
	cerr << "PARAMETERS REQUIRED ONLY WITH BDBH:\n";
	cerr << "  --out-files file1,file2,...: A list of output files created by the command-line (see the allowed templates under)\n";
	cerr << "                               NOTE: if several output files are generated for each command, the names must be separated by a comma (,)\n";
	cerr << "                               this parameter is required with bdbh, because output files will be stored inside the output data container\n";
	cerr << "\n";
	cerr << "OPTIONAL PARAMETERS:\n";
	cerr << "  --out-dir outdir           : All output will be written to this directory. Default = \"inputdir.out\"\n";
	cerr << "                               NOTES:\n";
	cerr << "                                      - The default name is \"inputdir.out\"\n";
	cerr << "                                      - If using bdbh data container as input, the default output name is \"inputdir.out.db\" (a bdbh data container)\n";
	cerr << "                                      - If using the \"dir\" file type, there is NO default output name.\n";
	cerr << "                                      - The output directory should NOT exist when chdb is started, and it will be created by chdb.\n";
	cerr << "  --work-dir workdir         : Change to this directory before executing command\n";
	cerr << "                               WARNING ! \n";
	cerr << "                                  - a RELATIVE path specified from --command will be treated FROM THE WORK DIRECTORY\n";
	cerr << "                                  - a RELATIVE PATH specified from ANY OTHER SWITCH will be treated FROM THE INITIAL LAUNCH DIRECTORY\n";
	cerr << "                               The default is: \"Do not change directory\", EXCEPT for the type: \"dir\", \n";
	cerr << "                               where the default is = \"change to %in-dir%/%path%\"";
	cerr << "  --create-environment       : A snippet containing some shell commands to create a working environment inside the work directory \n";
	cerr << "                               Will be executed AFTER chdir workdir and BEFORE the command itself\n";
	cerr << "                               EXAMPLE:\n";
	cerr << "                                  --create-environment 'cp ~/DATA/*.inp .; cp ~/DATA/*.conf .'\n";
	cerr << "  --block-size 1             : The higher the block-size, the less mpi communications, but you may get\n";
	cerr << "                               load-balancing issues (default = 1)\n";
	cerr << "  --sleep-time <T>           : Before starting process, each slave sleeps T * rank seconds. This is to desynchronize I/O calls when\n";
	cerr << "                               chdb is used to launch I/O intensive programs, as this could stress the shared filesystem\n";
	cerr << "  --on-error errors.txt      : When the command returns something different from 0, the status and the file path \n";
	cerr << "                               are stored in this file for later reference and execution\n";
	cerr << "                               NOTE: The default is to INTERRUPT chdb when the return status is not 0\n";
	cerr << "  --in-files file.txt        : Only files whose path is inside file.txt are considered for input\n";
	cerr << "                               Format: One path per line\n";
	cerr << "                               NOTE: A generated errors.txt (cf. --on-error) may be specified as in-files parameter \n";
	cerr << "  --report report.txt        : Generate a report with some timing info about the command (use only for debug !)\n";
	cerr << "  --mpi-slaves <s>           : The command (launched by the slaves) is an mpi program, s is the number of mpi processes/slave, placement is left to the system\n";
	cerr << "                                   -you CANNOT launch with chdb mpi programs using more than one node\n";
	cerr << "                                   -you CAN launch with chdb several slaves per node\n";
	cerr << "  --mpi-slaves <S:s:c>       : The command (launched by the slaves) is an mpi program, and chdb tries to control the placement:\n";
	cerr << "                             :     S is the number of slaves per node\n";
	cerr << "                                   s is the number of mpi processes/slave\n";
	cerr << "                                   c is the number of threads/mpi process\n";
	cerr << "                               NOTE: It is your reponsability to:\n";
	cerr << "                                   - Reserve the resources needed\n";
	cerr << "                                   - Arrange for S * s * c to be less or equal to the number of cores in each node\n";
	cerr << "                                   - Export correct environment variables, for example: --command-line 'env OMP_NUM_THREADS=s my_exe'\n";
	cerr << "\n";
	cerr << "OPTIONAL SWITCHES:\n";
	cerr << "  --sort-by-size             : Sort the input files the bigger first, may be less load balancing issues\n";
	cerr << "  --in-memory                : Read the whole database in memory. May help if you get poor performance when starting chdb with a lot of files and using bdbh.\n";
	cerr << "  --verbose                  : Some messages are printed\n";
	cerr << "  --help                     : Print this screen and leave\n";
	cerr << "\n";
	cerr << "TEMPLATES ALLOWED IN FILE NAME SPECIFICATIONS (--command, --out-files, --work-dir):\n";
	cerr << "  The following templates are allowed in filenames specified in parameters command-line and out-files.\n";
	cerr << "  They are expanded using the real input file. We suppose that the input file is inputdir/A/B/toto.txt:\n";
	cerr << "\n";
	cerr << "    %in-dir%       The input directory  (inputdir)\n";
	cerr << "    %out-dir%      The output directory (inputdir.out)\n";
	cerr << "    %path%         The input file complete path (relative to the input directory: A/B/toto.txt)\n";
	cerr << "    %name%         The file name with the extension (toto.txt)\n";
	cerr << "    %basename%     The file name without the extension (toto)\n";
	cerr << "    %dirname%      The directory name relative to the input or output directory (A/B)\n";
	cerr << "\n";
	cerr << "ENVIRONMENT VARIABLES:\n";
	cerr << "  The following environment variables are available in the command launched by chdb:\n";
	cerr << "    $CHDB_RANK      : The mpi rank of this slave\n";
	cerr << "    $CHDB_COMM_SIZE : The size of the chdb mpi communicator (ie nb of mpi processes, or nb of slaves + 1)\n";
	cerr << "\n\n";
	
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
