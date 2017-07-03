
/*===========================================================================

    chdb - Calcul à Haut Débit couplé à une Base de données embarquée
	       high throughput Computing witH an embedded DataBase
	 
=============================================================================*/

//#define DEBUGPID

#ifdef DEBUGPID
#include <mpi.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <memory>
#endif

#include <iostream>
#include <stdexcept>
using namespace std;

#include "system.hpp"
#include "parameters.hpp"
#include "usingfs.hpp"
#include "usingbdbh.hpp"
#include "basicscheduler.hpp"

void printHeader(const Parameters& prms, Directories& dir, const Scheduler& sched ) {
	cout << "CHDB - VERSION " << CHDB_VERSION << " - ";
	system("date");
	cout << "---------\n";
	string host;
	getHostName(host);
	cout << "MASTER RUNNING ON " << host << " pid " << getpid() << '\n';
	cout << "----------\n";
	cout << "PARAMETERS" << '\n';
	cout << "----------" << '\n';
	cout << "INPUT  DIRECTORY   = " << prms.getInDir() << '\n';
	if (prms.isSizeSort()) {
	cout << "FILES SORTED BY SIZE\n";
	} else {
		cout << '\n';
	}
	cout << "OUTPUT DIRECTORY   = " << prms.getOutDir() << '\n';
	cout << "WORK DIRECTORY     = " << prms.getWorkDir() << '\n'; 
	cout << "FILE TYPE          = " << prms.getFileType() << '\n';
	cout << "ENV SNIPPET        = " << prms.getEnvSnippet() << '\n';
	cout << "COMMAND            = " << prms.getExternalCommand() << '\n';
	cout << "OUTPUT FILES       = ";
	vector_of_strings out_files = prms.getOutFiles();
	for (size_t i=0; i<out_files.size();++i) {
		cout << out_files[i] << ',';
	}
	cout << '\n';
	cout << "BLOCK SIZE         = " << prms.getBlockSize() << '\n';
	string in_files = prms.getInFile();
	if (in_files!="") {
		cout << "INPUT FILES IN     = " << in_files << '\n';
	}
	if (prms.isAbrtOnErr()) {
		cout << "ABORT ON ERROR     = YES" << '\n';
	}
	else
	{
		cout << "ABORT ON ERROR     = NO" << '\n';
		cout << "SEE FILE           = " << prms.getErrFile() << '\n';
	}
	if (prms.isReportMode()) {
		cout << "REPORT WALL TIMES  = " << prms.getReport() << '\n';
	}

	cout << "----------\n";
	cout << "NB OF INPUT FILES  = " << dir.getNbOfFiles() << '\n';
	cout << "NUMBER OF SLAVES   = " << sched.getNbOfSlaves() << '\n';
}

void printTrailer(Scheduler& sched) {
	cout << "ELAPSED TIME (s)   = " << sched.getTimer() << '\n';
	cout << "NB OF TREATED FILES= " << sched.getTreatedFiles() << '\n';
}

#ifdef DEBUGPID
void writePid() {
	ostringstream tmp;
	int rank;
	string host_name;
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	getHostName(host_name);

	tmp << "pid." << rank;

	ofstream p(tmp.str().c_str());
	p<<"h="<<host_name<<" p="<<getpid()<<endl;
}
#else
void writePid(){}
#endif

/************************
 * @brief Handling some signals: SIGINT (ctrl-c) and SIGTERM (kill) are handled by chdb and if necessary by bdbh code.
 ************************/

class SignalHandle {
    public:
        static class Initializer {
            public:
                Initializer() {
                    struct sigaction new_action;
                    sigemptyset (&new_action.sa_mask);
                    new_action.sa_flags = 0;
                    new_action.sa_handler = sig_handler;
                
                    sigaction (SIGINT,  &new_action, NULL);
                    sigaction (SIGTERM, &new_action, NULL);
                    cerr << "Signals programmed\n";
                }
        } Init;

        static void ConnectScheduler(Scheduler* s_ptr) { sched_ptr = s_ptr; };
        static void sig_handler(int s) {
			if (sched_ptr != NULL) {
				cerr << "signal received " << s << endl;
				if (sched_ptr != NULL) sched_ptr->SetSignal(s);
			}
		}
        
    private:
		static Scheduler* sched_ptr;
};

// Initialize SignalHandle, ie run the Initializer to connect the signal handler, and init cmd_ptr and sched_ptr
Scheduler* SignalHandle::sched_ptr = NULL;
SignalHandle::Initializer SignalHandle::Init;

/** 
 * @brief A factory to create to correct directory object, depending of the parameters
 * 
 * @param prms 
 * 
 * @return 
 */
Directories* dirFactory(Parameters& prms) {
	string name = prms.getInDir();
	if ( isEndingWith(name,".db") ) {
		return new UsingBdbh(prms);
	} else {
		return new UsingFs(prms);
	}
}
	
/*////////////// Main ///////////////*/
int main(int argc,char* argv[])
{
	Scheduler::init(argc,argv);
#ifdef DEBUGPID
	writePid();
#endif
	
	try {
		Parameters prms(argc,argv);
		auto_ptr<Directories> dir_aptr(dirFactory(prms));
		Directories& dir = *dir_aptr.get();

		BasicScheduler sched(prms,dir);
		SignalHandle::ConnectScheduler(&sched);

		if (sched.isMaster() && prms.isVerbose()) {
			sched.startTimer();
			printHeader(prms,dir,sched);
		}
		sched.mainLoop();
		if (sched.isMaster() && prms.isVerbose()) {
			printTrailer(sched);
		}
	}
	catch (ParametersHelp& e) {
	}
	catch (exception& e) {
		cerr << e.what() << '\n';
		Scheduler::abort();
	}

	Scheduler::finalize();
}
