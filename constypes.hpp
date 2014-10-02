

#ifndef CONSTYPES_H
#define CONSTYPES_H

#include <vector>
#include <string>
#include <stdexcept>
using namespace std;


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

#endif
