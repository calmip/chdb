
/*===========================================================================

    chdb - Calcul à Haut Débit couplé à une Base de données embarquée
	       high throughput Computing witH an embedded DataBase
	 
=============================================================================*/

#include <iostream>
#include <stdexcept>
using namespace std;

#include "parameters.hpp"
#include "usingfs.hpp"
#include "basicscheduler.hpp"

/*////////////// Main ///////////////*/
int main(int argc,char* argv[])
{
	Scheduler::init(argc,argv);
	
	try {
		Parameters prms(argc,argv);
		UsingFs dir(prms);
		BasicScheduler sched(prms,dir);
		if (sched.isMaster()) {
			sched.startTimer();
		}
		sched.mainLoop();
		if (sched.isMaster() && prms.isVerbose()) {
			cout << "NUMBER OF SLAVES = " << sched.getCommSize()-1 << '\n';
			cout << "ELAPSED TIME     = " << sched.getTimer() << '\n';
		}
	}
	catch (exception& e) {
		cerr << e.what() << '\n';
		Scheduler::abort();
	}

	Scheduler::finalize();
}
