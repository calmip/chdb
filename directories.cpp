/**
 * @file   directories.cpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Mon Sep 29 11:04:09 2014
 * 
 */

//#include <iostream>
//#include <iterator>
//#include <set>
using namespace std;

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <list>
#include <cerrno>
//#include <libgen.h>

//#include "command.h"
#include "usingfs.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
//#include <libgen.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "system.hpp"

/** 
 * @brief set the protected member rank and comm_size
 * 
 * @param r 
 * @param s 
 *
 * @exception the rank and comm_size can be set only 1 time
 */

void Directories::setRank(int r, int s) {
	if (rank>=0) {
		throw(logic_error("ERROR - rank already set"));
	}
	rank=r;
	comm_size=s;
}

/**
 *  @brief Return the nextblok of file names
 *
 *  @pre The vector files should have been initialized
 *  @return A vector of names
 *          An empty vector if there are no more file names
*/

vector_of_strings Directories::nextBlock() {
	vector_of_strings blk;
	if ( blk_ptr != files.end() ) {

		int blk_size = prms.getBlockSize();
		vector_of_strings::iterator blk_next_ptr;
		if (files.end()-blk_ptr > blk_size) {
			blk_next_ptr = blk_ptr + blk_size;
		} else {
			blk_next_ptr = files.end();
		}
		blk.assign(blk_ptr,blk_next_ptr);
		blk_ptr = blk_next_ptr;
	}
	return blk;
}
/** 
 * @brief Replace some templates in the parameter, using the file_path:
 *        file pathes: #path#, #dirname#
 * 
 * @param[in]  p         Path used as a source for the template expansion
 * @param[out] text      String to expand (generally a command line or a file name)
 * @param[in]  force_out If true, output directory is forced at begininning of text
 */
void Directories::completeFilePath(const string& p, string& text, bool force_out) {

	// We must work with a readwrite copy !
	// p is the path (a/toto.txt), d the dirname (a), n the name (toto.txt), b the basename (toto), e the ext (txt)
	string d,n,b,e;
	parseFilePath(p,d,n,b,e);

	string id = prms.getInDir();
	string od = getTempOutDir();
	static string tmpl1="%in-dir%";
	static string tmpl2="%out-dir%";
	static string tmpl3="%path%";
	static string tmpl4="%basename%";
	static string tmpl5="%name%";
	static string tmpl6="%dirname%";
	
	replaceTmpl(tmpl1,id,text);
	replaceTmpl(tmpl2,od,text);
	replaceTmpl(tmpl3,p,text);
	replaceTmpl(tmpl4,b,text);
	replaceTmpl(tmpl5,n,text);
	replaceTmpl(tmpl6,d,text);
	if (force_out) {
		size_t od_index=text.find(od);
		// if not found, or not found at start
		if (od_index!=0) {
			text = od + '/' + text;
		}
	}
}

/** 
 * @brief Should be called by the subclasses executeExternalCommand. Build an mpi command,
 *        using the env variable CHDB_MPI_CMD and the switch mpi-slaves
 * 
 * @param[inout]  cmd    Command to run, returned unchanged if no mpi, or completed
 *                       with the mpirun calling string if mpi in slave wanted
 */
void Directories::buildMpiCommand(string& cmd) const {
	string mpi_slaves = prms.getMpiSlaves();
	if ( mpi_slaves != "" ) { 
		const char * mpi_cmd_c = getenv("CHDB_MPI_CMD");
		if (mpi_cmd_c != NULL) {
			string mpi_cmd = mpi_cmd_c;
			string h;
			getHostName(h);
			replaceTmpl("%MPI_SLAVES%", mpi_slaves, mpi_cmd);
			replaceTmpl("%HOSTNAME%", h, mpi_cmd);
			replaceTmpl("%COMMAND%", cmd, mpi_cmd);
			cmd = mpi_cmd;
		} else {
			throw runtime_error("ERROR -The env variable CHDB_MPI_CMD does not exists");
		}
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

