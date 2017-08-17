
#ifndef BDBH_RM_H
#define BDBH_RM_H

#include <vector>
#include <string>
using namespace std;

#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {
/** \brief This object is used with the remove command 
 */
	class Rm: public Command {
    public:
		Rm(const Parameters& p, BerkeleyDb& d): Command(p,d){};
		virtual void Exec();
    
    private:
		void __Exec(const string& key, bool is_recurs);
		void __ExecDir(const string& key, Mdata mdata, bool is_recurs);
		void __Remove(const string& key, Mdata* mdata_ptr, int lvl);
	};
}
#endif

