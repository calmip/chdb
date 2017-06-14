
#ifndef BDBH_MERGE_H
#define BDBH_MERGE_H

#include <vector>
#include <string>
using namespace std;

#include "constypes.hpp"
#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {
/** \brief This object is used with the merge command
 */
	class Merge: public Command {
    public:
		Merge (const Parameters& p, BerkeleyDb& d, Merge* to=NULL) throw(DbException);
		virtual void Exec() throw(BdbhException,DbException);
    
    private:
		void __Exec(const Fkey&) throw(BdbhException,DbException);
		void __ExecFrom() throw(BdbhException,DbException);
		void __ExecFromKey(const string&) throw(BdbhException,DbException);
		void __ExecFromDir(const string&, Mdata) throw(BdbhException,DbException);
		void __ExecFromFile(const string&, Mdata&) throw(BdbhException);

		Merge* destination;
	};
}
#endif

