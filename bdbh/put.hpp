
#ifndef BDBH_PUT_H
#define BDBH_PUT_H

#include <vector>
#include <string>
using namespace std;

#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {
/** \brief This object is used with the put command
 */
	class Put: public Command {
    public:
		Put(const Parameters& p, BerkeleyDb& d): Command(p,d){};
		virtual void Exec();
    
//    protected:
//		void __ExecDir(const Fkey& fkey, Mdata & mdata);
    
    private:
		virtual void __Exec(const Fkey& fkey);    
	};

/** \brief This object is used with the mkdir command
    Mkdir derives from Put, just to use the __ExecDir function. This is not really
    objectically correct, because we cannot say that Mkdir is an extension of a Put operation, but... it works
*/
	class Mkdir: public Put {
    public:
		Mkdir(const Parameters& p, BerkeleyDb& d): Put(p,d){};
		virtual void Exec();
    
//    private:
//		virtual void __Exec(const Fkey& fkey);    
	};
}

#endif

