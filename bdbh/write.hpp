
#ifndef BDBH_WRITE_H
#define BDBH_WRITE_H

#include <vector>
#include <string>
using namespace std;

#include "constypes.hpp"
#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {

/** \brief This object is used with the add command 
 */
	class Write: public Command {
    public:
		Write(const Parameters& p, BerkeleyDb& d) throw(DbException): Command(p,d){};
		virtual void Exec() throw(BdbhException,DbException);
    
    private:
		void __ExecSymLink(const Fkey&);
		void __ExecDir(const Fkey& fkey, Mdata & mdata);
		void __Exec(const Fkey& fkey);    
	};
}
#endif

