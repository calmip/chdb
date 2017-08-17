
#ifndef BDBH_READ_H
#define BDBH_READ_H

#include <vector>
#include <string>
using namespace std;

#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {
/** \brief (struct) Keep some (directory name,metadata) pairs in memory */
	struct DirectoryEntry
	{
		DirectoryEntry(const string& f, const Mdata& m):name(f),mdata(m){};
		string name;
		Mdata mdata;
	};

/** \brief this object is used when using the commands read or extract */
	class Read: public Command {
    public:
		Read(const Parameters& p, BerkeleyDb& d): Command(p,d){
			_AdjustBufferCapacity();
		};
		virtual void Exec();
    
    private:
		void __Exec(const Fkey& );
		void __ExecDir(const Fkey& fkey, Mdata mdata);
		void __ExecSymLink(const Fkey&);
		void __ExecFile(const Fkey& fkey, Mdata& mdata);

		void __RestoreTime(const string& file_name, const Mdata& mdata);
		vector<DirectoryEntry> directories;
	};
}

#endif

