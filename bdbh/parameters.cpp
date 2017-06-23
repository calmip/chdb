
#include <iostream>
#include <iterator>
#include <set>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
using namespace std;

#include "command.hpp"
#include "parameters.hpp"
#include "exception.hpp"

using bdbh::Parameters;
using bdbh::Cwd;
using bdbh::BdbhException;
using bdbh::Fkey;

#include "SimpleOpt.h"

const char* legal_commands[]={LEGAL_COMMANDS};
const char* commands_need_files[]={COMMANDS_NEED_FILES};
const char* commands_need_keys[]={COMMANDS_NEED_KEYS};
const char* legal_commands_switches[]={LEGAL_COMMANDS,LEGAL_SWITCHES};

/**
  Cwd initialization
  */
  
Cwd::Cwd()
{
    int size = 100;
    char* bfr = (char*) malloc(size+1);
    if (bfr==NULL)
        throw(BdbhException("Not enough memory"));
    
    char* nbfr = NULL;
    do
    {
        nbfr = getcwd(bfr,size);
        if (nbfr==NULL)
        {
            if (errno == ERANGE)
            {
                size *= 2;
                bfr = (char*) realloc(bfr,size+1);
            }
            else
            {
                throw(BdbhException("",errno));
            }
        }
    } while (nbfr == NULL);
    cwd_string = (string) bfr;
    free(bfr);
}

/**
    Wrapping dirname by a more c++ly correct function
    
    \param path The path
    \return The directory part of the path
*/
string bdbh::sdirname(const string & path)
{
    char* dup_path = strdup(path.c_str());
    string dir = (string) dirname(dup_path);
    free(dup_path);
    return dir;
}

/**
    Wrapping basename by a more c++ly correct function
    
    \param path The path
    \return The basename (dirname stripped, extension stripped also) part of the path
*/
string bdbh::sbasename(const string & path)
{
    char* dup_path = strdup(path.c_str());
    string base = (string) basename(dup_path);
    free(dup_path);
    return base;
}

/**
    If the parameter starts with a /, erase it
	If the parameter starts with "./", erase it too
    
    \param[in,out] s The string to strip
*/
void bdbh::StripLeadingSlash(string& s)
{
    if (s.length()==0)
        return;
    if (s[0]=='/')
        s.erase(0,1);
	if (s[0]=='.' && s.length()>=2 && s[1]=='/')
		s.erase(0,2);
}

/**
    If the parameter end with a /, erase it
    
    \param[in,out] s The string to strip
*/
void bdbh::StripTrailingSlash(string& s)
{
    if (s.length()==0)
        return;
    if (s[s.length()-1]=='/')
        s.erase(s.length()-1,1);
}

/**
    If the parameter does not starts with a /, add it
    
    \param[in,out] s The string to complete
*/
void bdbh::AddLeadingSlash(string& s)
{
    if (s.length()==0)
        return;
    if (s[0]!='/')
        s = "/" + s;
}

/**
    If the parameter does not end with a /, add it
    
    \param[in,out] s The string to strip
*/
void bdbh::AddTrailingSlash(string& s)
{
    if (s.length()==0)
        return;
    if (s[s.length()-1]!='/')
        s += '/';
}

/**
    count the number of /
    
    \param s The char* to count
    \return The number of /
*/
int bdbh::CountLevel(const char * s)
{
    int level=0;
    while((s=index(s,'/'))!=NULL)
    {
        level++;
        s++;
    };
    return level;
}    

/**
    If s starts with root, erase root from s
    
    example: StripLeadingString("/home/manu","/home/manu/some_file.txt" returns true, s becomes some_file.txt
    \param[in] root The starting string to find, should NOT end with /
    \param[in,out] s The string to strip
    \return bool true if root was found (or if root is empty)
*/
bool bdbh::StripLeadingString(const string& root, string& s)
{
    // If root empty, return true
    if (root.length()==0)
        return true;
    
    // if s starts with root, erase root and return true, else return false
    if (s.find(root) == 0) 
    {
        s.erase(0,root.size());
        return true;
    }
    else
        return false;
}

/**
    If s starts with root/, erase root from s

    example: StripLeadingStringSlash("/home/manu","/home/manu.txt" returns false    
    \param[in] root The starting string to find
    \param[in,out] s The string to strip - It is at least stripped from the leading slash, may be from the string root
    \return bool true if root was found (or if root is empty)
*/
bool bdbh::StripLeadingStringSlash(const string& root, string& s)
{
    // If root empty, return true
    if (root.length()==0)
    {
        StripLeadingSlash(s);
        return true;
    }
    // if s starts with root/, erase root + 1 chr and return true, else return false
    if (s.find(root)==0 && s[root.size()]=='/')   // if s starts with root
    {
        s.erase(0,root.size()+1);
        return true;
    }
    else
    {
        StripLeadingSlash(s);
        return false;
    }
}

/** The Fkey keeps some information about the objects we'll store/ to the database or retrieve from

\param name The file name or key name
\param is_file_name If true, name names a file, if false name names a key
\param is_leaf If true, it is a leaf, ie something (key or file) specified on the command line
If false, it is a directory which was on the path (in /home/manu/file, file is a leaf, home and manu are not)
\param is_recurs If true (only directory), explore the hierarchy. If false, do not 

*/

Fkey::Fkey(const string& name, bool is_leaf, bool is_recurs):leaf(is_leaf),recurs(is_recurs)
{
	string n = name;
	StripLeadingSlash(n);
	key = Fkey::root + n;
	if (name.size()>0 && name[0] != '/') {
		file_name = Fkey::directory + name;
	} else {
		file_name = name;
	}
}

string Fkey::root="";
string Fkey::directory="";
bool   Fkey::sort_by_f=true;

/**
   \brief Down in the keys and files hierarchy
          leaf is set to false, recurs unchanged

   \param d A directory (string) to go down to

*/
void Fkey::Down(const string& d) {
	leaf = true;
	AddTrailingSlash(file_name);
	file_name += d;
	AddTrailingSlash(key);
	key += d;
}

/**
   \brief Up in the keys and files hierarchy
          leaf and recurs are set to false, as we are NO MORE a leaf !
          If at top of keys/files, the key/file field to ""

   \return true if at top_of_files AND at top_of_keys
*/

bool Fkey::Up() {
	bool top_of_files= false;
	bool top_of_keys = false;
	leaf = false;
	recurs = false;
	if (file_name.length()!=0) {
		file_name = sdirname(file_name);
		if (file_name.length()==1 && (file_name[0]=='.'||file_name[0]=='/')) {
			file_name="";
			top_of_files = true;
		}
	} else {
		top_of_files = true;
	}
	if (key.length()!=0) {
		key = sdirname(key);
		if (key.length()==1 && key[0]=='.') {
			key="";
			top_of_keys = true;
		}
	} else {
		top_of_keys = true;
	}
	return top_of_keys && top_of_files;
}

/** The constructor of Parameters

	\param argc   passed to main by the system
	\param argv   passed to main by the system
	\param help   The help function

*/
Parameters::Parameters(int argc, char** argv, void(*help)())  throw(BdbhUsageException,BdbhException): 
database(""),
compress(false),
shell_mode(false)
{
    __Init(argc, argv, help);
    if (GetCommand() == BDBH_SHELL)
	shell_mode = true;
}

/** The default constructor
	\brief No parameter, cannot be used when strict_check_mode is activated
           Useful for applications using the API (but NOT bdbh)
 */
Parameters::Parameters() :
	database(""),
	compress(false),
	shell_mode(false)
{
	int argc = 0;
	const char* argv[] = {""};
	char ** wr_argv = __ArgvAlloc(argc,argv);
	try {
		__Init(argc, wr_argv, NULL);
	} catch(exception& e) {
		cerr << e.what() << '\n';
		__ArgvFree(argc,wr_argv);
		throw(e);
	}
	if (GetCommand() == BDBH_SHELL)
		shell_mode = true;
	
	// Free memory
	__ArgvFree(argc,wr_argv);
}
	
	
/** Another constructor
	\brief You may pass to this constructor a const char**, ie: {"--database","db","ls","-r"}
	Useful to be called from some other application than bdbh (ex chdb)
	\param argc   argv size
	\param argv   An array of const char*

*/
Parameters::Parameters(int argc, const char* argv[])  throw(BdbhUsageException,BdbhException): 
			database(""),
			compress(false),
			shell_mode(false)
{

	// Allocate a correct argc,argv, then call __Init
	char ** wr_argv = __ArgvAlloc(argc,argv);
	// Call __Init
	try {
		__Init(argc, wr_argv, NULL);
	} catch(exception& e) {
		cerr << e.what() << '\n';
		__ArgvFree(argc,wr_argv);
		throw(e);
	}
	if (GetCommand() == BDBH_SHELL)
		shell_mode = true;
	
	// Free memory
	__ArgvFree(argc,wr_argv);
}

/** Yet Another constructor
	\brief You may pass to this constructor a vector of strings
	\param argc   passed to main by the system
	\param argv   passed to main by the system

*/
Parameters::Parameters(const vector<string>& args )  throw(BdbhUsageException,BdbhException): 
			database(""),
			compress(false),
			shell_mode(false)
{
	// Allocate a const char** argv
	const char** argv = (const char**) malloc(args.size()*sizeof(char*));

	// Init the char* from args
	for (size_t i=0; i<args.size(); ++i) {
		argv[i] = args[i].c_str();
	}

	// Allocate a correct argc,argv, then call __Init
	int argc = (int) args.size();
	char ** wr_argv = __ArgvAlloc(argc,argv);

	// Call __Init
	try {
		__Init(argc, wr_argv, NULL);
	} catch(exception& e) {
		cerr << e.what() << '\n';
		__ArgvFree(argc,wr_argv);
		throw(e);
	}
	if (GetCommand() == BDBH_SHELL)
		shell_mode = true;
	
	// Free memory
	__ArgvFree(argc,wr_argv);
	free(argv);
}
/** 
 * @brief Copy argv (const) to a NON CONST argv
 * 
 * @param argc The number of arguments (will be increased by 1, shoud NOT be a const)
 * @param argv The array of const char*
 * 
 * @return The base address of wr argv (WARNING argc has been increased by 1)
 */
char** Parameters::__ArgvAlloc(int& argc, const char** argv) {

	// Add a pseudo-argv[0]
	argc+=1;
	const char* argv0="FROM THE API";

	char** wr_argv = (char**) malloc(argc*sizeof(char*));
	if (wr_argv==NULL) {
		throw(BdbhException("Not enough memory !"));
	}

	for (int i=0; i<argc; ++i) {
		const char* a=(i==0)?argv0:argv[i-1];
		wr_argv[i] = (char*) malloc((strlen(a)+1)*sizeof(char));
		if (wr_argv[i]==NULL) {
			throw(BdbhException("Not enough memory !"));
		}
		strcpy(wr_argv[i],a);
	}
	return wr_argv;
}

/** 
 * @brief Free memory allocated by __ArgvAlloc
 * 
 * @param argc 
 * @param wr_argv 
 * 
 * @return 
 */	
void Parameters:: __ArgvFree(int argc, char** wr_argv) {
	for (int i=0; i<argc; ++i) {
		free(wr_argv[i]);
	}
	free(wr_argv);
}

// Init to false, bdbh will change this with the SetStrictCheckMode() function
bool bdbh::Parameters::strict_check_mode = false;

/** Parse a new command line and re-init the object

It is not possible to change the database through this function, and the help function is not called
If something is wrong with the new command, it is just ignored.
\param argc   The argument count
\param argv The arguments

*/
void Parameters::ReadNewCommand(int argc, char**argv)  throw(BdbhUsageException,BdbhException)
{
	__Init(argc,argv,NULL);
}

/**
   \brief If the command needs files parameters, return true. Called by __Init
   \pre   argc,argv must have been read
   \todo  refaire ca avec une structure de donnees plus intelligente !
*/
bool Parameters::__CommandNeedsFiles() const {
    int tmp = -1;
    for (int i=0; i <= COMMANDS_NEED_FILES_MAX; i++)
    {
        if (legal_commands[command] == commands_need_files[i])
        {
            tmp = i;
            break;
        }
    }
	return (tmp!=-1);
}

/**
   \brief If the command needs keys parameters, return true. Called by __Init
   \pre   argc,argv must have been read
   \todo  refaire ca avec une structure de donnees plus intelligente !
*/
bool Parameters::__CommandNeedsKeys() const {
    int tmp = -1;
    for (int i=0; i <= COMMANDS_NEED_KEYS_MAX; i++)
    {
        if (legal_commands[command] == commands_need_keys[i])
        {
            tmp = i;
            break;
        }
    }
	return (tmp!=-1);
}
 	
/** Object initialization (from the command line)

\param argc   passed to main by the system
\param argv   passed to main by the system
\param help   The help function (may be NULL)

*/
void Parameters::__Init(int argc, char** argv, void(*help)())  throw(BdbhUsageException,BdbhException)
{ 	
    // define the ID values to identify the option
    enum { 
	// From 0 to BDBH_MAX_COMMAND, the options id is the same as the commands identification
	OPT_HELP=BDBH_MAX_COMMAND + 1,  // -h|--help
	OPT_SEPARATOR,  // NOT USED - Higher then OPT_CMDx, lower than other OPT_
	OPT_DB,         // -d|--database arg
	OPT_COMPRESS,   // -c|--compress
	OPT_ROOT,       // -t|--root arg
	OPT_DIRECTORY,  // -C|--directory arg
	OPT_RECURSIVE,  // -r|--recursive,-o|--overwrite,
	OPT_OVERWRITE,  // -o|--overwrite
	OPT_INMEMORY,   // -m|--in_memory
	OPT_VERBOSE,    // -v|--verbose
	OPT_LONG_LIST,  // -l|--long_list
	OPT_SSORT_LIST, // -S|--size_sort
	OPT_RSORT_LIST, // -R|--reverse_sort
	OPT_LEVEL,      // -L|--level arg
	OPT_MODE,       // --mode arg
	OPT_VALUE,      // --value arg
	OPT_STAMP,      // --stamp arg,
	OPT_CLUSTER,    // --cluster
	OPT_CMD         // create,info,add,extract,put,mkdir,cat,ls,rm,chmod,shell,sync,q
    };
    
    // declare a table of CSimpleOpt::SOption structures. See the SimpleOpt.h header
    // for details of each entry in this structure. In summary they are:
    //  1. ID for this option. This will be returned from OptionId() during processing.
    //     It may be anything >= 0 and may contain duplicates.
    //  2. Option as it should be written on the command line
    //  3. Type of the option. See the header file for details of all possible types.
    //     The SO_REQ_SEP type means an argument is required and must be supplied
    //     separately, e.g. "-f FILE"
    //  4. The last entry must be SO_END_OF_OPTIONS.
    //
    CSimpleOpt::SOption options[] = {
	{ BDBH_CREATE,   (char *)"create",     SO_NONE    },
	{ BDBH_CAT,      (char *)"cat",        SO_NONE    },
	{ BDBH_EXTRACT,  (char *)"extract",    SO_NONE    },
	{ BDBH_ADD,      (char *)"add",        SO_NONE    },
	{ BDBH_PUT,      (char *)"put",        SO_NONE    },
	{ BDBH_RM,       (char *)"rm",         SO_NONE    },
	{ BDBH_LS,       (char *)"ls",         SO_NONE    },
	{ BDBH_INFO,     (char *)"info",       SO_NONE    },
	{ BDBH_CHMOD,    (char *)"chmod",      SO_NONE    },
	{ BDBH_MKDIR,    (char *)"mkdir",      SO_NONE    },
	{ BDBH_SHELL,    (char *)"shell",      SO_NONE    },
	{ BDBH_QUIT,     (char *)"q",          SO_NONE    },
	{ BDBH_SYNC,     (char *)"sync",       SO_NONE    },
	{ BDBH_MERGE,    (char *)"merge",      SO_NONE    },
	{ BDBH_HELP,     (char *)"help",       SO_NONE    },
	{ BDBH_CONVERT,  (char *)"convert",    SO_NONE    },
	{ BDBH_LISTKEYS, (char *)"lk",         SO_NONE    },
	{ OPT_HELP,      (char *)"-h",          SO_NONE    },
	{ OPT_HELP,      (char *)"--help",      SO_NONE    },
	{ OPT_DB,        (char *)"-d",          SO_REQ_SEP },
	{ OPT_DB,        (char *)"--database",  SO_REQ_SEP },
	{ OPT_COMPRESS,  (char *)"-c",          SO_NONE    },
	{ OPT_COMPRESS,  (char *)"--compress",  SO_NONE    },
	{ OPT_ROOT,      (char *)"-t",          SO_REQ_SEP },
	{ OPT_ROOT,      (char *)"--root",      SO_REQ_SEP },
	{ OPT_DIRECTORY, (char *)"-C",          SO_REQ_SEP },
	{ OPT_DIRECTORY, (char *)"--directory", SO_REQ_SEP },
	{ OPT_RECURSIVE, (char *)"-r",          SO_NONE    },
	{ OPT_RECURSIVE, (char *)"--recursive", SO_NONE    },
	{ OPT_INMEMORY,  (char *)"-m",          SO_NONE    },
	{ OPT_INMEMORY,  (char *)"--in_memory", SO_NONE    },
	{ OPT_OVERWRITE, (char *)"-o",          SO_NONE    },
	{ OPT_OVERWRITE, (char *)"--overwrite", SO_NONE    },
	{ OPT_VERBOSE,   (char *)"-v",          SO_NONE    },
	{ OPT_VERBOSE,   (char *)"--verbose",   SO_NONE    },
	{ OPT_LONG_LIST, (char *)"-l",          SO_NONE    },
	{ OPT_LONG_LIST, (char *)"--long_list", SO_NONE    },
	{ OPT_SSORT_LIST,(char *)"-S",          SO_NONE    },
	{ OPT_SSORT_LIST,(char *)"--size_sort", SO_NONE    },
	{ OPT_RSORT_LIST,(char *)"-R",          SO_NONE    },
	{ OPT_RSORT_LIST,(char *)"--reverse_sort", SO_NONE },
	{ OPT_LEVEL,     (char *)"-L",          SO_REQ_SEP },
	{ OPT_LEVEL,     (char *)"--level",     SO_REQ_SEP },
	{ OPT_MODE,      (char *)"--mode",      SO_REQ_SEP },
	{ OPT_VALUE,     (char *)"--value",     SO_REQ_SEP },
	{ OPT_STAMP,     (char *)"--stamp",     SO_REQ_SEP },
	{ OPT_CLUSTER,   (char *)"--cluster",   SO_REQ_SEP },
	SO_END_OF_OPTIONS                       // END
    };

    // re-initializing some private members
    root          = "";
    root_level    = 0;
    command       = -1;
    overwrite     = false;
    verbose       = false;
    inmemory      = false;
    long_list     = false;
    size_sort     = false;
    reverse_sort  = false;
    cluster       = false;
    level         = -1;
    mode          = "";
    stamp         = "";
    val           = "";
    val_available = false;
		
    CSimpleOpt arguments(argc, argv, options);

    string directory="";
    bool recursive = false;

    while (arguments.Next()) {
		//cout << "coucou " << arguments.OptionId() << " " << arguments.OptionText() << "\n";
        if (arguments.LastError() == SO_SUCCESS) {
            if (arguments.OptionId() <= BDBH_MAX_COMMAND) {
		command = arguments.OptionId();
	    }
	    else if (arguments.OptionId() == OPT_HELP) {
                if (help != NULL)
		    help();
		    throw( BdbhUsageException() );
		}
	    else if (arguments.OptionId() == OPT_DB) {
		if (shell_mode)
		    throw (BdbhException("--database not allowed in shell mode"));
		database = arguments.OptionArg();
	    }
	    else if (arguments.OptionId() == OPT_COMPRESS) {
		if (shell_mode)
		    throw (BdbhException("--compress not allowed in shell mode"));
		compress = true;
	    }
	    else if (arguments.OptionId() == OPT_ROOT) {
		root = arguments.OptionArg();
	    }
	    else if (arguments.OptionId() == OPT_DIRECTORY) {
		directory = arguments.OptionArg();
	    }
	    else if (arguments.OptionId() == OPT_RECURSIVE) {
		recursive = true;
	    }
	    else if (arguments.OptionId() == OPT_OVERWRITE) {
		overwrite = true;
	    }
	    else if (arguments.OptionId() == OPT_INMEMORY) {
		inmemory = true;
	    }
	    else if (arguments.OptionId() == OPT_VERBOSE) {
		verbose = true;
	    }
	    else if (arguments.OptionId() == OPT_LONG_LIST) {
		long_list = true;
	    }
	    else if (arguments.OptionId() == OPT_SSORT_LIST) {
		size_sort = true;
	    }
	    else if (arguments.OptionId() == OPT_RSORT_LIST) {
		reverse_sort = true;
	    }
	    else if (arguments.OptionId() == OPT_LEVEL) {
		level = atoi(arguments.OptionArg());
	    }
	    else if (arguments.OptionId() == OPT_MODE) {
		mode = arguments.OptionArg();
	    }
	    else if (arguments.OptionId() == OPT_VALUE) {
		val = arguments.OptionArg();
	    }
	    else if (arguments.OptionId() == OPT_STAMP) {
		stamp = arguments.OptionArg();
	    }
	    else if (arguments.OptionId() == OPT_CLUSTER) {
		cluster = true;
	    }
	}
    }

    // check the validity of the command	
    if (command == -1) 
    {
	if (strict_check_mode && !shell_mode) {
	    cerr << "WARNING - No command recognized - switching to shell mode, exit with q\n";
	    command = BDBH_SHELL;
	}
    }
    
    if (shell_mode)
    {
	if ( command==BDBH_CREATE || command==BDBH_SHELL )
	    throw (BdbhException("Command Not allowed in shell mode"));
    }
    else
    {
	if ( command==BDBH_SYNC )
	    throw (BdbhException("Command allowed ONLY in shell mode"));
    }
		
    // root: useful for the commands extract, ls, add/write
    //       if not specified, an environment variable is used
    if (root == "")
    {
        char* t;
        if (command==BDBH_EXTRACT || command==BDBH_LS) {
            t = getenv("BDBH_ROOTX");
	} else if (command==BDBH_ADD) {
            t = getenv("BDBH_ROOTA");
	}
        else {
            t = NULL;
	}

        if (t!=NULL) {
            root = (string) t;
	}
    } else {
	// root Should NOT start with a '/'
	if (root[0] != '/') {
	    StripLeadingSlash(root);
	}

	// append a /
	if (root.length()>0) {
	    AddTrailingSlash(root);
	}
    }

    AddTrailingSlash(root);
    StripLeadingSlash(root);
    Fkey::root = root;

    root_level = CountLevel(root.c_str());

    if ( directory=="/" )
	throw BdbhException("Sorry, --directory cannot reference /");
    AddTrailingSlash(directory);
    Fkey::directory = directory;

    // The database specification: required (in strict_check_mode only), but may come from the environment
    if (database == "")
    {
        char* db = getenv("BDBH_DATABASE");
        if (db==NULL && strict_check_mode==true)
            throw (BdbhException("Database not specified - please set BDBH_DATABASE env variable or specify --database"));
	if (db != NULL) {
	    database = (string) db;
	}
    };

    // The --value is ignored if the command is not put
    if (command==BDBH_PUT && val != "")
    {
        val_available=true;
    }

    // recursive
    if (level>0)
	recursive = true;

    // Some flags are incompatible with some commands
    if (command==BDBH_CAT)
    {
        if (recursive)
            throw (BdbhException("recursive cannot be specified with the command cat"));
    }
    if (command!=BDBH_LS)
    {
	if (inmemory)
	    throw (BdbhException("inmemory can be specified ONLY WITH ls"));
    }

    // The --level switch is used ONLY with the commands: extract, ls, chmod
    if (command!=BDBH_EXTRACT && command!=BDBH_LS && command!=BDBH_CHMOD && level!=-1)
    {
        throw (BdbhException("--level can be specified only with ls, chmod or extract"));
    }
	
    __InitFkeys (arguments.Files(),arguments.FileCount(), recursive);
}

/** Initialize the vector: fkeys

\brief __InitFkeys, initialize the vector fkeys

\pre A lot of other fields must be already initialized
\param files An array of char * (the file names)
\todo  works ONLY for several template instantiations on SimpleOpt
\param file_count The number of files
\param recursive true if recursivity is wanted

*/
void Parameters::__InitFkeys(char ** files, unsigned int file_count, bool recursive) throw(BdbhUsageException,BdbhException)
{
    fkeys.clear();

	// The command needs files as parameters
    if (__CommandNeedsFiles())
    {
        if (file_count==0)
        {
            throw (BdbhException("Files or directories not specified - "));
        }
        else
        {
			Fkey::sort_by_f = false;
			__InitFkeysLeaf(files,file_count, recursive);
			__InitFkeysIntermediateDir();
		}
	}

	// The command needs keys as parameters
    else if (__CommandNeedsKeys())
    {
        if (file_count==0)
        {
            throw (BdbhException("Keys not specified - "));
        }
		else
		{
			Fkey::sort_by_f = true;
			__InitFkeysLeaf(files,file_count,recursive);
			__InitFkeysIntermediateDir();
		}
    }

	// The command may be run without parameters, if there are they are keys
	// \todo houlala pas beau
	else
	{
		Fkey::sort_by_f = true;
		__InitFkeysLeaf(files,file_count,recursive);
		__InitFkeysIntermediateDir();
	}

/*
	cout << fkeys.size() << " FKEYS - ROOT=" << Fkey::root << " DIRECTORY=" << Fkey::directory << " SORT BY F= " << Fkey::sort_by_f << '\n';
	for (size_t i=0; i<fkeys.size();i++) {
		cout << "FKEY K=" << fkeys[i].GetKey() << " F=" << fkeys[i].GetFileName() << " L=" << fkeys[i].IsLeaf() << "\n";
	}
*/
}

/**
   \brief For each file in the parameter list, generate an fkey (marked as a 'leaf')
          For the parameters, see __InitFkeys
*/
void Parameters::__InitFkeysLeaf(char** files, unsigned int file_count, bool recursive)
{
	for (unsigned int i=0; i < file_count; ++i)
	{
		if ((string) files[i] == "/")
			throw BdbhException("Sorry, you cannot specify / as a directory or key name");
		
		fkeys.push_back(Fkey(files[i],true,recursive));
	}

	if (fkeys.size()==0) {
		// \todo houlala pas beau !!!
		switch(command) {
		case BDBH_EXTRACT:
		case BDBH_LS: fkeys.push_back(Fkey("*",true,true));
		}
	}
}

/**
   \brief mark for insertion in the database every intermediate directory
          The fkeys generated are not leaves 
*/

void Parameters::__InitFkeysIntermediateDir()
{
 
	//            We use a set, to avoid any duplicate
            
	set<Fkey> set_fkeys;
	for (vector<Fkey>::iterator i = fkeys.begin(); i != fkeys.end(); ++i)
	{
		string k = i->GetKey();
		string f = i->GetFileName();
		//cout << f << " " << k << " " << i->IsLeaf() << "\n";

		bool end_of_loop=false;
		while(!end_of_loop) {
			set_fkeys.insert(set_fkeys.begin(),*i);
			end_of_loop = i->Up();
		}
	}
		
	// Now, copy data from the set to the vector
	fkeys.clear();
	fkeys.insert(fkeys.begin(),set_fkeys.begin(),set_fkeys.end());
}

/** Log something, only if verbose
\param msg The message to log
\param os The ostream to log to
*/

void Parameters::Log(const string& msg, ostream& os) const
{
    if (verbose)
        os << "INFO - " + msg + "\n";
}
