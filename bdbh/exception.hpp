
#ifndef BDBH_EXCEPTION_H
#define BDBH_EXCEPTION_H

using namespace std;

#include "constypes.hpp"
#include <stdexcept>
#include <string.h>
#include <string>

namespace bdbh {
/** \brief The exceptions generated by this application
	The error message is passed to the constructor
*/
	class BdbhException: public runtime_error {
    public:
		BdbhException(const string& msg) throw(): runtime_error(msg),_message(msg) {};
		BdbhException(const string name, int err);
		virtual ~BdbhException() throw() {};
		virtual const char * what() const throw() { 
			return _message.c_str();
		};
    protected:
		string _message;
	};

/** \brief Derives from BdbhException. Called when a stupid switch is specified
 */
	class BdbhUsageException: public BdbhException {
    public:
		BdbhUsageException() throw(): BdbhException("") {};
		virtual ~BdbhUsageException() throw() {};
	};

/** An alternate constructor for BdbhException: give him a file/directory name and an errno, 
    the constructor builds the message from strerror
*/

	inline BdbhException::BdbhException(const string name, int err):runtime_error(name),_message(name + " - " + strerror(err)){}
}
#endif

