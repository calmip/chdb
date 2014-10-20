

#ifndef CONSTYPES_H
#define CONSTYPES_H

#include <vector>
#include <string>
#include <stdexcept>
using namespace std;

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
