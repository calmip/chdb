

#ifndef CONSTYPES_H
#define CONSTYPES_H

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <sstream>
using namespace std;

#define CHDB_VERSION 0.61

/** 
 * @brief Some predicates
 * 
 * @param s 
 * 
 * @todo should work with the same name, but we get the error "unresolved function type"
 * @return 
 */

inline bool isNotNullStr (const string& s) { return (s.size()>0); }
inline bool isNotNull(int i) { return (i!=0); }

/* 
   NOTMP => The CALMIP policy is not to use /tmp on the nodes
            Comment out the following line to regain tmpdir functionality
*/
#define NOTMP

/*
   comment out to avoid creating an output directory per slave
*/
//#define OUTDIRPERSLAVE 1

/*
  number of retries executing the external command if there is an error in system()
*/
#define NUMBER_OF_RETRIES 5

/**
   \brief To have a class non copyable, you just have to private derive from this class
*/

class NonCopyable {
public:
	NonCopyable(){};

private:
	NonCopyable(const NonCopyable&) {};
	void operator=(const NonCopyable&) {};
};

/* Some typedefs */
typedef vector<string> vector_of_strings;
typedef vector<int> vector_of_int;
typedef vector<double> vector_of_double;


#endif
