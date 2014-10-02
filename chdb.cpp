
/*===========================================================================

    chdb - Calcul à Haut Débit couplé à une Base de données embarquée
	       high throughput Computing witH an embedded DataBase
	 
=============================================================================*/

#include <iostream>
using namespace std;

#include "parameters.hpp"

/*////////////// Main ///////////////*/
int main(int argc,char* argv[])
{
	for(int i=0; i<argc; i++) {
		cout << "arg " << i << " " << argv[i] << "\n";
	}

	Parameters prms(argc,argv);
	cout << "coucou " << prms.getExternalCommand() << "\n";
	for(int i=0; i<argc; i++) {
		cout << "arg " << i << " " << argv[i] << "\n";
	}


}
