
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

