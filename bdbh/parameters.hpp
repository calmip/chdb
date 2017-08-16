
#ifndef BDBH_PARAMETERS_H
#define BDBH_PARAMETERS_H
#include <vector>
#include <string>
using namespace std;

#include "exception.hpp"
#include "SimpleOpt.h"

namespace bdbh {

// We store ONLY files less than 4 Mb
#define MAX_FILE_SIZE 4*1024*1024

// We refuse to put in memory database files larger than 4 Gb
#define MAX_INMEMORY_SIZE 4*1024*1024*1024L

/** The legal commands, with an integer representation
 */
// WARNING - BDBH_CREATE should be 0 (cf. bdbh.cpp)
#define BDBH_CREATE  0          
#define BDBH_CAT     1
#define BDBH_EXTRACT 2
#define BDBH_ADD     3
#define BDBH_PUT     4
#define BDBH_RM      5
#define BDBH_LS      6
#define BDBH_MV      7
#define BDBH_INFO    8
#define BDBH_CHMOD   9
#define BDBH_MKDIR  10
#define BDBH_SHELL  11
#define BDBH_QUIT   12
#define BDBH_SYNC   13
#define BDBH_MERGE  14
#define BDBH_HELP   15
#define BDBH_CONVERT  16
#define BDBH_LISTKEYS 17

#define BDBH_MAX_COMMAND 17

/** The corresponding legal commands, with an char* representation
*/
#define LEGAL_COMMANDS "create","cat","extract","add","put","rm","ls","mv","info","chmod","mkdir","shell","q","sync","merge","help","convert","lk"

/** some commands need files, other need keys 
*/
#define COMMANDS_NEED_FILES "add","merge"
#define COMMANDS_NEED_FILES_MAX 1
#define COMMANDS_NEED_KEYS "cat","rm","put","mkdir","mv"
#define COMMANDS_NEED_KEYS_MAX 4

/** The legal switches, used only for readline completion - Thus we skip here database, compress, stamp
*/

#define LEGAL_SWITCHES "--root","-t","--directory","-C","--recursive","-r","--overwrite","-o", "--verbose","-v","--long_list","-l","-m","--level","-L","--mode","--value"
#define BDBH_MAX_SWITCH 20


    /** \brief Current Working Directory */
	class Cwd {
	public:
		Cwd();
		const string& operator()(){return cwd_string;};
		
	private:
		string cwd_string;
	};
	
    /** Retrieve the dirname 
		\param path The path
		\return The dirname
	*/
	string sdirname(const string&);
	
    /** Retrieve the basename 
		\param path The path
		\return The basename
	*/
	string  sbasename(const string&);
	
    /** Strip the leading slash, if any 
		\param[in,out] s The string to strip
	*/
	void  StripLeadingSlash(string&);
	
    /** Add a leading slash, if none 
		\param[in,out] s The string to add a / to
	*/
	void  AddLeadingSlash(string&);

   /** Add a trailing slash, if none 
		\param[in,out] s The string to add a / to
	*/
	void  AddTrailingSlash(string&);
	
    /** Strip the trailing slash, if any 
		\param[in,out] s The string to strip
	*/
	void  StripTrailingSlash(string&);
	
    /** Strip the leading string, if any
		\param root The root to detect and to strip if present
		\param[in,out] s The string to strip
		\return true if something was stripped or if root was empty
    */
	bool  StripLeadingString(const string& root, string& s);
	
    /** Strip the leading string and the slash, if any
		\param root The root to detect and to strip if present, PLUS the slash
		\param[in,out] s The string to strip
		\return true if something was stripped or if root was empty
	*/
	bool  StripLeadingStringSlash(const string& root, string& s);
	
    /** count the number of / ( = the level in the hierarchy)
		\param[in] s The string
		\return int The level
	*/
	int  CountLevel(const char * s);
	
    /**  \brief from file names to keys and vice-versa 
	 */
	class Fkey {
    public:
		// The constructor: give him a file name or a key name, the other will be computed */
		Fkey(const string& name, bool is_leaf, bool is_recurs=false);

		// Go up in the hierarchy... if possible !
		bool Up();

		// Go down in the hierarchy, always possible but we need a parameter
		void Down(const string &);

		const string& GetFileName() const {return file_name;};
		const string& GetKey() const {return key;};
		bool IsLeaf() const {return leaf;};
		bool IsRecurs() const {return recurs;};
		void SetRecurs() { recurs = true; };
		static string root;
		static string directory;
		static bool sort_by_f;

    private:
		string file_name;
		string key;
		bool leaf;
		bool recurs;
	};
	
    /** Useful to sort the Keys (used by a set) */
	inline bool operator< (const Fkey& fk1, const Fkey& fk2)
	{
		if (Fkey::sort_by_f) {
			return fk1.GetFileName() < fk2.GetFileName();
		} else {
			return fk1.GetKey() < fk2.GetKey();
		}
	}
	
//	/** Operator== we test only the file name */
//	inline bool operator==(const Fkey& fk1, const Fkey& fk2) {
//		return fk1.GetFileName()==fk2.GetFileName();
//	}
	
    /** \brief This class parses and keeps in memory the command line 
	 */
	class Parameters {
    public:
		// The default constructor
		Parameters();

		// The constructor for bdbh
		Parameters(int, char**, void(*)());

		// A constructor with const char*
		Parameters(int, const char*[]);

		// A constructor with a vector of strings
		Parameters(const vector<string>&);

		/*Parameters(const string& db, int command);*/
		void ReadNewCommand(int, char**);
		const string& GetDatabase() const { return database;};
		const string& GetRoot() const { return root;};
		int GetRootLevel() const { return root_level;};
		bool GetCompress() const { return compress;};
		bool GetVerbose() const {return verbose; };
		bool GetOverWrite() const { return overwrite;};
		bool GetInmemory() const { return inmemory; };
		bool GetSorted() const { return size_sort; };
		bool GetSortedReverse() const { return reverse_sort; };
		int GetCommand() const { return command;};
		bool GetLongList() const { return long_list;};
		int GetMaxFileSize() const {return MAX_FILE_SIZE;}; // hum. Not too good
		int GetLevel() const {return level;};
		const string& GetValue() const {return val;};
		bool IsValueAvailable() const {return val_available;};
		const string& GetMode() const {return mode;};
		const string& GetStamp() const {return stamp;};
		const vector<Fkey>& GetFkeys() const { return fkeys;};
		
		void Log(const string&, ostream&) const;

		// The default for the API is to AVOID strict checking
		// strict checking is useful for use with bdbh himself
		static void SetStrictCheckMode() { strict_check_mode = true; };
		static void UnsetStrictCheckMode() { strict_check_mode = false; };

		void __Init (int argc, char** argv, void(*help)());
		void __InitFkeys(char ** files, unsigned int file_count, bool recursive);
		void __InitFkeysLeaf (char ** files, unsigned int file_count, bool recursive);
		void __InitFkeysIntermediateDir();

		bool __CommandNeedsFiles() const;
		bool __CommandNeedsKeys() const;

		char** __ArgvAlloc(int& argc, const char** argv);
		void __ArgvFree(int argc, char** argv);

		vector<Fkey> fkeys;
		string database;
		bool compress;
		string root;
		int root_level;
		int command;
		bool overwrite;
		bool inmemory;
		bool verbose;
		bool long_list;
		bool size_sort;
		bool reverse_sort;
		int level;
		string mode;
		string val;
		bool val_available;
		string stamp;
		bool shell_mode;
		static bool strict_check_mode;
	};
}
#endif
