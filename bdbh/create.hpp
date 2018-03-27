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

#ifndef BDBH_CREATE_H
#define BDBH_CREATE_H

#include <vector>
#include <string>
using namespace std;

#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {

/** \brief This object is used with the create command 
 */
	class Create: public Command {
    public:
		Create(const Parameters& p, BerkeleyDb& d): Command(p,d) {
			if (d.GetOpenMode() != BDBH_OCREATE) {
				throw(logic_error("The database should be opened in CREATE mode"));
			}
		};
		virtual void Exec();
	};

/** \brief This object is used with the convert command 
 */
	class Convert: public Command {
    public:
		Convert(const Parameters& p, BerkeleyDb& d): Command(p,d) {};
		virtual void Exec();
	};

/**
   \brief Two convenient functions to get info from the database
*/

	InfoData GetInfo(BerkeleyDb & bdb);
	InfoData GetInfo(const string& database);

/** \brief This object is used with the info command 
 */
	class Info: public Command {
    public:
		Info(const Parameters& p, BerkeleyDb& d): Command(p,d),no_print(false),data_ready(false) {};
		void InhibitPrinting()      { no_print = true; };
		void DoNotInhibitPrinting() { no_print = false; };
		InfoData GetInfoData()      { __DataReady(); return info_data; };
		virtual void Exec();
		friend InfoData GetInfo(BerkeleyDb & bdb);

	private:
		InfoData info_data;
		bool no_print;
		bool data_ready;
		void __DataReady() {
			if (data_ready) return;
			else throw (logic_error("Internal error: Call bdbh::Info::Exec() before this methode"));
		}
	};

/** \brief This object is used with the help command 
 */
	class Help: public Command {
    public:
		Help(const Parameters& p, BerkeleyDb& d): Command(p,d) {};
		virtual void Exec();
	};

	void Usage();

}

#endif

