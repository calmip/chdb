
#ifndef BDBH_WRITE_H
#define BDBH_WRITE_H

#include <vector>
#include <string>
using namespace std;

#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {

/** \brief This object is used with the add command 
 */
	class Write: public Command {
    public:
		Write(const Parameters& p, BerkeleyDb& d): Command(p,d){};
		virtual void Exec();
    
    private:
		void __ExecSymLink(const Fkey&);
		void __ExecDir(const Fkey& fkey, Mdata & mdata);
		void __Exec(const Fkey& fkey);    
	};
}
#endif

