
#ifndef BDBH_CHMOD_H
#define BDBH_CHMOD_H

#include <vector>
#include <string>
using namespace std;
#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {
    /** \brief This object is used with the chmod command 
	 */
	class Chmod: public Command {
    public:
		Chmod(const Parameters& p, BerkeleyDb& d): Command(p,d) {};
		virtual void Exec();
		
    private:
		void __Exec(const string& key, bool is_recurs,mode_t f_sel, mode_t to_mode);
		void __ExecDir(const string& key, Mdata mdata, bool is_recurs, mode_t f_sel, mode_t to_mode);
		void __Mode(const string& key, Mdata mdata, mode_t f_sel, mode_t to_mode);
		
	};
}
#endif

