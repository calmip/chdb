/** This file is part of bdbh software
 * 
 * bdbh helps users to store little files in an embedded database
 *
 * bdbh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2010-2014    L I P M
 *  Copyright (C) 2015-2018    C A L M I P
 *  bdbh is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with bdbh.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:
 *        Emmanuel Courcelle - C.N.R.S. - UMS 3667 - CALMIP
 *        Nicolas Renon - Université Paul Sabatier - University of Toulouse)
 */

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

