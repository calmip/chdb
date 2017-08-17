#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;

int main(int argc, char* argv[]) {
	if (argc==1) {
		cerr << "ERROR - You should pass at least ONE argument\n";
		exit(1);
	}
	string filename = argv[1];

	char* tmp = NULL;

	tmp = getenv("CHDB_RANK");
	int chdb_rank = tmp==NULL?-1:atoi(tmp);
	
	tmp = getenv("CHDB_COMM_SIZE");
	int chdb_size = tmp==NULL?-1:atoi(tmp);

	tmp = getenv("MA_VARIABLE_DENVIRONNEMENT");
	string other_variable = tmp==NULL?"none" : tmp;
		
	ifstream in(filename.c_str());
	if (!in) {
		cerr << "ERROR - File not found " << filename << "\n";
		exit(1);
	}
	
	time_t now;
	char time_fmt[32];
	time(&now);
	tm* timeInfo = localtime(&now);
	strftime(time_fmt, 32, "%H-%M-%S", timeInfo);

	// Important to sleep from 0 to 100s to simulate heaving computations
	// If you don't, the FIRST slace if given the whole work because it is the first to start !
	long int rand = random();
	unsigned int sleep_time = rand % 100;
	sleep(sleep_time);


	string from_file;
	in   >> from_file;
	cout << "RANK           = "          << chdb_rank << '\n';
	cout << "SIZE           = "          << chdb_size << '\n';
	cout << "MAVAR          = "          << other_variable << '\n';
	cout << "Time now       = " << time_fmt << "\n";
	cout << "Read from file " << filename << " = " << from_file << '\n';
}

