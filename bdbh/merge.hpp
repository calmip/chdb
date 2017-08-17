
#ifndef BDBH_MERGE_H
#define BDBH_MERGE_H

#include <vector>
#include <string>
using namespace std;

#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {
/** \brief This object is used with the merge command
 */
	class Merge: public Command {
    public:
		Merge (const Parameters& p, BerkeleyDb& d, Merge* to=NULL);
		virtual void Exec();
    
    private:
		void __Exec(const Fkey&);
		void __ExecFrom();
		void __ExecFromKey(const string&);
		void __ExecFromDir(const string&, Mdata);
		void __ExecFromFile(const string&, Mdata&);

		Merge* destination;
	};
}
#endif

