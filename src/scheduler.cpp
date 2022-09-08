/**
 * @file   scheduler.cpp
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

#include <mpi.h>
#include <fstream>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <list>
#include <cerrno>
#include <functional>

#include "scheduler.hpp"
#include "system.hpp"
#include <sys/types.h>
#include <dirent.h>

/** 
 * @brief Build a Scheduler object
 *        throw an error if MPI not yet initialized or if no slave 
 *        Set the environment variables CHDB_RANK and CHDB_COMM_SIZE
 * 
 * @param p 
 * @param d 
 * 
 * @return 
 */


Scheduler::Scheduler(const Parameters& p, Directories& d, bool o=false) : prms(p), dir(d), only_testing(o), start_time(-1) {
    int flg;
    MPI_Initialized(&flg);
    if (flg==0) {
        throw logic_error("ERROR - MPI not yet initialized");
    };
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &comm_size);

    if (! only_testing && comm_size==1) {
        throw logic_error("ERROR - YOU SHOULD HAVE AT LEAST 1 SLAVE");
    }

    // Set two env variables
    ostringstream os_tmp;
    os_tmp << rank;
    int rvl1 = setenv("CHDB_RANK",os_tmp.str().c_str(),true);
    os_tmp.str("");

    os_tmp << comm_size;
    int rvl2 = setenv("CHDB_COMM_SIZE",os_tmp.str().c_str(),true);
    
    // @todo - Let the user modify herself the environment ???
    string chdb_env = "CHDB_RANK CHDB_COMM_SIZE";
    if (prms.isVerbose()) {
        setenv("CHDB_VERBOSE","1",true);
        chdb_env += " CHDB_VERBOSE";
    }
    int rvl3 = setenv("CHDB_ENVIRONMENT",chdb_env.c_str(), true);

    if (rvl1!=0 || rvl2!=0 || rvl3!=0)
        throw runtime_error("ERROR - COULD NOT setenv CHDB_RANK, CHDB_COMM_SIZE or CHDB_ENVIRONMENT");

    // Give some infos to dir
    dir.setRank(rank,comm_size);

    // Initialize hostname
    getHostName(hostname);

    // Initialize a second communicator
    // node_comm is a connector specific to each node
    // It is used to compute the "node_rank", ie the node of this process for one node
    hash<string> hasher;
    int color = (int) (hasher(hostname) >> 1);  // Must be positive integer

    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &node_comm);
    MPI_Comm_rank (node_comm, &node_rank);
}


/**
 * @brief Init the checkList, asking the Directory for the list of files
 * 
 * @pre The directory object must be initialized
 * 
 *****/ 
void Scheduler::_initCheckList() {
    const vector_of_strings& files = dir.getFiles();
    for (vector_of_strings::const_iterator s=files.begin(); s!= files.end(); ++s) {
        checkList[*s] = false;
    }
}

/**
 * @brief Check the items of the list, ie mark the files are "treated" unless they get an error
 * 
 * @pre The ckeckList must be already initialized
 * 
 * @param treated_files A list of treated (= used for computation) files
 * @param return_values A corresponding list of returned values, the file is checked only if value is 0 
 *
 * @exception Throw a logic_error if some file is not in the list
 * 
 *********/
void Scheduler::_checkListItems(const vector_of_strings& treated_files, const vector_of_int& return_values) {
    for (size_t i=0; i < treated_files.size(); ++i) {
        string f = treated_files[i];
        int    v = return_values[i];
        if (checkList.find(f) == checkList.end()) {
            string msg = "ERROR - THE FILE " + f + " IS NOT IN THE CHECK LIST !";
            throw logic_error(msg.c_str());
        }
        if (v==0) checkList[f] = true;
    }
}

    

/** 
 * @brief Call MPI_Abort
 * 
 */
void Scheduler::abort() {
    MPI_Abort(MPI_COMM_WORLD,1);
}

/** 
 * @brief call MPI_init 
 * 
 * @param argc 
 * @param argv 
 */

void Scheduler::init(int argc, char**argv) {
    MPI_Init(&argc,&argv);
}
    
void Scheduler::startTimer() {
    start_time = MPI_Wtime();
}

double Scheduler::getTimer() {
    if (start_time==-1) {
        throw(logic_error("ERROR - Timer was not started !"));
    }
    return MPI_Wtime()-start_time;
}

/** 
 * @brief Send a barrier to be sure everybody is synchronized, then finalize
 * 
 */
void Scheduler::finalize() {
    int flag = 0;
    MPI_Request request;
    MPI_Ibarrier(MPI_COMM_WORLD,&request);
    MPI_Status sts;
    do {
        MPI_Test(&request,&flag,&sts);
        sleepMs(SLEEP_TIME);
    } while(flag==0);
    MPI_Finalize();
}

/****
 * @brief Called by main to inform the scheduler that a signal was received !
 *        See the SignalHandle class
 *        If master, Save the state, wait 25 s and exit
 *        If slave, wait 25 s and exit
 * 
 * @param signal The signal received
 * 
 * @return If master, DOES NOT RETURN (call _exit)
 * 
 *****/
void Scheduler::SetSignal(int signal) {
    if (isMaster()) {
        cerr << "Scheduler rank=" << getRank() << " received a signal " << signal << " - Creating CHDB-INTERRUPTION.txt and exiting" << endl;
        ofstream ofs ("CHDB-INTERRUPTION.txt", ofstream::out);
        ofs << "# CHDB WAS INTERRUPTED - You may restart chdb using this file with the switch --in-files\n";
        ofs << dir.howToConsolidate() << endl;
        
        int j = 0;
        for (map<string,bool>::iterator i = checkList.begin(); i != checkList.end(); ++i) {
            if ( i->second == false) {
                j++;
                ofs << i->first << endl;
            }
        }
        
        ofs << "# Number of files not yet processed = " << j << endl;
        ofs.close();
        
        // Close open files, if necessary
        if (err_file.is_open())    err_file.close();
        if (report_file.is_open()) report_file.close();
    }
    else {
        cerr << "Scheduler rank=" << getRank() << " received a signal - " << signal << " - Sleeping 25 s" << endl;
    }
    sleep(25);
    _exit(0);
}

