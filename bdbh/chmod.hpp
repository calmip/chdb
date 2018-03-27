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

