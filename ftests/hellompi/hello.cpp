#include "mpi.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc==1) {
		cerr << "ERROR - You should pass at least ONE argument\n";
		exit(1);
	}
	string filename = argv[1];

	MPI_Init(NULL,NULL);
	int rank,size;
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	char* name = (char*) malloc(50);
	gethostname(name,50);
	cout << "Hello, running on " << name << " my rank is = " << rank << '/' << size << '\n';

	char* tmp = NULL;

	tmp = getenv("CHDB_RANK");
	int chdb_rank = tmp==NULL?-1:atoi(tmp);
	
	tmp = getenv("CHDB_SIZE");
	int chdb_size = tmp==NULL?-1:atoi(tmp);

	tmp = getenv("MA_VARIABLE_DENVIRONNEMENT");
	string other_variable = tmp==NULL?"none" : tmp;
		
	long int rand = random();
	unsigned int sleep_time = rand % 10;  // Sleeping from 0 to 10 s

	ifstream in(filename.c_str());
	if (!in) {
		cerr << "ERROR - File not found " << filename << "\n";
		exit(1);
	}
	
	cout << "Now sleeping " << sleep_time << "s\n";
	sleep(sleep_time);

	string from_file;
	in   >> from_file;
	cout << "CHDB_RANK           = "          << chdb_rank << '\n';
	cout << "CHDB_COMM_SIZE           = "          << chdb_size << '\n';
	cout << "MAVAR          = "          << other_variable << '\n';
	cout << "Read from file " << filename << " = " << from_file << '\n';
	MPI_Finalize();
}

