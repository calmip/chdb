#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

int current_time_nanoseconds(){
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);
    return tm.tv_nsec;
}

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

    srandom(current_time_nanoseconds());    
    long int rand = random();
    unsigned int sleep_time = rand % 10;  // Sleeping from 0 to 10 s

    ifstream in(filename.c_str());
    if (!in) {
        cerr << "ERROR - File not found " << filename << "\n";
        exit(1);
    }
    
    //cout << "Now sleeping " << sleep_time << "s\n";
    sleep(sleep_time);

    string from_file;
    in   >> from_file;
    cout << "CHDB_RANK      = "          << chdb_rank << '\n';
    cout << "CHDB_SIZE      = "          << chdb_size << '\n';
    cout << "MAVAR          = "          << other_variable << '\n';
    cout << "COMPUTED RAND  = "          << rand << '\n';
    cout << "Read from file " << filename << " = " << from_file << '\n';
}

