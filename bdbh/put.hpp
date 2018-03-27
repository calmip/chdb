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

