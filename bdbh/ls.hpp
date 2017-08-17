
#ifndef BDBH_LS_H
#define BDBH_LS_H

#include <vector>
#include <string>
using namespace std;

#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {

	/** This struct is used among other things for sorting the files before printing them - see __ExecDir */
	struct finfo {
		finfo(const string& n, const Mdata& m): name(n),mdata(m) {};
		string name;
		Mdata mdata;
	};
	inline int operator<(const finfo& a, const finfo& b) { return a.mdata.size < b.mdata.size; }


    /** \brief LsObserver implements a simplistic version of the observer pattern to the Ls object
		See http://www.oodesign.com/observer-pattern.html
		Here, attachObserver REPLACES the currently attached observer (there is only ONE) observer
	*/
	class LsObserver {
	public:
		// Update is passed a file or directory name, together with its metadata
		virtual void Update(string,const Mdata&) = 0;
	};

	/** \brief LsPrint is the default observer provided: print file name, that's it ! */
	class LsPrint: public LsObserver {
	public:
		LsPrint(const Parameters& p, ostream& out): prm(p), os(out) {};
		void Update(string ,const Mdata&);
	private:
		const Parameters& prm;
		ostream& os;
	};

    /** \brief This object is used with the ls command 
	 */
	class Ls: public Command {
    public:
		Ls(const Parameters& p, BerkeleyDb& d): Command(p,d), __obs(NULL) {};
		virtual void Exec();
		void AttachObserver(LsObserver& obs) { __obs = &obs; };

    private:
		LsObserver* __obs;
//		void __List(string,const Mdata&, ostream& os);
		void __Exec(const string&, bool);
		void __ExecDir(const string& key, Mdata mdata, bool is_recurs);
	};
}

#endif

