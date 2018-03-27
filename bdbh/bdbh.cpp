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


/*===========================================================================

    bdbh - Manage a hierarchy of files inside a BerkeleyDb database
    
	 
=============================================================================*/

using namespace std;

#include <iostream>
#include <fstream>
#include <iomanip>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "parameters.hpp"
#include "exception.hpp"
#include "command.hpp"
#include "create.hpp"
#include "read.hpp"
#include "write.hpp"
#include "ls.hpp"
#include "rm.hpp"
#include "mv.hpp"
#include "put.hpp"
#include "chmod.hpp"
#include "merge.hpp"

using namespace bdbh;

bool Parse (const char* s, vector<string>& v);

typedef auto_ptr<Command> Command_aptr;
typedef auto_ptr<BerkeleyDb> BerkeleyDb_aptr;

void ExecCommand(const Parameters& params, BerkeleyDb_aptr& bdb, TriBuff& bfr3, Command_aptr& cmd) throw(DbException,BdbhException);
void bdbh::Usage();
char * command_generator (const char* text, int state);

void ListKeys(const string& );
char* StampPrompt(const string&, const string&,int,char*,size_t );

extern const char* legal_commands_switches[];


/************************
 * Handling some signals
 ************************/


class SignalHandle {
    public:
        static class Initializer {
            public:
                Initializer() {
                    struct sigaction new_action;
                    sigemptyset (&new_action.sa_mask);
                    new_action.sa_flags = 0;
                    new_action.sa_handler = sig_handler;
                
                    sigaction (SIGINT,  &new_action, NULL);
                    sigaction (SIGTERM, &new_action, NULL);
                }
        } Init;

        static void Connect(Command* c_ptr) { cmd_ptr = c_ptr; };
        static void sig_handler(int s) {
            if (cmd_ptr != NULL) {
                cerr << "signal received " << s << endl;
                cmd_ptr->SetSignal(s);
            }
        }
        
    private:
        static Command* cmd_ptr;
};

// Initialize SignalHandle, ie run the Initializer to connect the signal handler, and init cmd_ptr
Command* SignalHandle::cmd_ptr = NULL;
SignalHandle::Initializer SignalHandle::Init;

/*////////////// Main ///////////////*/
int main(int argc,char* argv[])
{
    
	bdbh::Initialize();
	Parameters::SetStrictCheckMode();

    Command_aptr command;
    BerkeleyDb_aptr bdb;
    
    
    rl_completion_entry_function = command_generator;
    
    string prompt="bdbh>";
    try
    {
        Parameters params(argc,argv,Usage);
        if (params.GetCommand()==BDBH_LISTKEYS)
        {
            ListKeys(params.GetDatabase());
            return 0;
        }
        
        TriBuff bfr3;
        
        // Command shell ==> GO TO INTERACTIVE MODE
        if (params.GetCommand()==BDBH_SHELL)
        {
            // Try to open the database read-write, if not possible switch to read mode
            int flg = BDBH_OSHELL;
            while(1)
            {
                try
                {
                    bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),flg,params.GetVerbose());
                    break;
                } 
                catch (DbException& e)
                {
                    if (flg==BDBH_OSHELL)
                        flg = BDBH_OREAD;
                    else
                        throw(e);
                };
            }            
              
            // line is used to read the commands
            char* line = NULL;
            vector<string> parsed;
            
            // Print some text about bdbh
            cout << "This is bdbh release " << V_MAJOR << "." << V_MINOR << "($Rev: 1033 $)\n";
            cout << "Useful commands are:\n";
            cout << "     help (printing the program usage)\n";
            cout << "     q    (quitting the program)\n";
            cout << "     !    (escaping to a shell)\n";

            int rvl=0;
            while(!cin.eof())
            {
                // Display the prompt and ask for the answer
                line =StampPrompt(prompt,params.GetStamp(),rvl,line,params.GetMaxFileSize());
                if (line==NULL)
                    break;
          
                if (command.get() != NULL)
                    command.get()->ResetSignal();
                
                // !: fork a /bin/sh and wait for the end of the child
                if (line[0]=='!')
                {
                    pid_t child = fork();
                    if (child==0)
                    {
						char* const argv[] = {NULL};
                        execv("/bin/sh",argv);
                    }
                    else
                    {
                        int sts=0;
                        waitpid(child,&sts,0);
                    }
                    //line =StampPrompt(prompt,params.GetStamp(),0,line,params.GetMaxFileSize());
                    continue;
                }
                
                // Parse the line
                if (Parse(line,parsed))
                {
                    // transform a vector of strings into a char** 
                    char** alt_argv = (char**)malloc((parsed.size()+1)*sizeof(char**));
                    alt_argv[0] = strdup(argv[0]);
                    for (unsigned int i=1; i<=parsed.size();i++)
                    {
                        alt_argv[i] = strdup(parsed[i-1].c_str());
                    }

                    // Catch some exceptions
                    rvl=0;
                    try
                    {
                        // Re-init params
                        params.ReadNewCommand(parsed.size()+1,alt_argv);
                        
                        // Special commands, there is no need to create a new Command object for them
                        if (params.GetCommand()==BDBH_QUIT)
                            break;
                        else if (params.GetCommand()==BDBH_SYNC)
                        {
                            bdb.get()->Sync(true);
                            //line = StampPrompt(prompt,params.GetStamp(),0,line,params.GetMaxFileSize());
                            continue;
                        }
                        
                        // Other command: create a Command object, and call Exec 
                        else
                            ExecCommand(params,bdb,bfr3,command);
                    }
                    
                    // Usage exception: output the error code
                    catch(BdbhUsageException& e) {  
                        rvl = 1;
                    }
                    
                    // DbException: throw it again, program will terminate
                    catch (DbException& e)
                    {
                        throw(e);
                    }
                    
                    // Other exception: just print a message
                    //catch (error& e) 
                    //{
                    //    cerr << "ERROR - "<< e.what() << "\n";
                    //    rvl = 1;
                    //}
                    catch (exception &e) {
                        cerr << "ERROR - " << e.what() << "\n";
                        rvl = 2;
                    }
                    
                    // Free temporary memory
                    free (alt_argv);
                }
                else
                {
                    cerr << "ERROR - Parse error\n";
                    cout << " 1\n";
                    cout.flush();
                }
            }
            free(line);
        }
        
        // NOT INTERACTIVE MODE: Just call ExecCommand
        else
        {
            ExecCommand(params,bdb,bfr3,command);
        };
    } 
    catch(BdbhUsageException& e) {   // exit with code 1
        exit(1);
    } 
    catch(DbException& e)
    {
        cerr << "ERROR - " << e.what() << "\n";
        return 3;
    }
    //catch (error& e) 
    //{
    //   cerr << "ERROR - "<< e.what() << "\n";
    //    return 1;
    //}
    catch (exception &e) {
        cerr << "ERROR - " << e.what() << "\n";
        return 2;
    }
    if (command.get() != NULL)
        return (command.get()->GetExitStatus());
    else
        return 0;

	bdbh::Terminate();

}

/** Print a stamp (may be empty string) adn wait for the answer
    This function is used only in interactive mode
    If no stamp parameter provided, we use readline
    If a stamp parameter was provided, we do not used readline because we are probably working from a script
    
    \param prompt The prompt tp print (used only if stamp == "")
    \param stamp The stamp
    \param rvl The status of the previous command (will be printed)
    \param line A buffer to copy the data read, not used if using readline
    \param lne_size The size of line, unused if using readline
    
    \return A char*, pointing to the entered commands
*/

char* StampPrompt(const string& prompt, const string& stamp, int rvl, char* line, size_t lne_size)
{
    // If no stamp, print the prompt through readline
    if (stamp=="")
    {
        // If line not null, it was already used throu readline: free memory
        if (line != NULL)
            free(line);
        cout << "status " << " " << rvl << "\n";
        char* line = readline(prompt.c_str());
        add_history(line);
        return line;
    }
    
    // If stamp, we are using through a script: do not use readline, and do not print the prompt
    else
    {
        
        // If line null, we must allocate some memory
        if (line == NULL)
            line = (char*) calloc(lne_size,sizeof(char));
        cout << " " << stamp << " " << rvl << "\n";
        cin.getline(line,lne_size);
        cout.flush();
        return line;
    }
}


/** This function is used for readline, for comand completion
    
\param text The starting text of the command
\param state =0 when we start at the top of the list
\return The next command or switch matching with text, NULL if none matches 

*/

char * command_generator (const char* text, int state)
{
  static int list_index, len;
  const char* name;

  /* If this is a new word to complete, initialize now.  This
     includes saving the length of TEXT for efficiency, and
     initializing the index variable to 0. 
  */
  if (!state)
  {
      list_index = 1;   // Skipping the create command
      len = strlen (text);
  }

  /* Return the next name which partially matches from the
     command list. 
  */
  while ((name=legal_commands_switches[list_index]) && list_index <= BDBH_MAX_COMMAND+BDBH_MAX_SWITCH)
  {
      list_index++;
      
      if (strncmp (name, text, len) == 0)
          return (strdup(name));
  }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}


/** Execute a single command
 \pre params and TriBuff must be initialized
 
 \param params The parameters
 \param bdb A BerkeleyDb*, may be initialized or not
 \param bfr3 a TriBuff, already allocated
 \param command a Command*, will be re-allocated if already allocated 

*/
void ExecCommand(const Parameters& params, BerkeleyDb_aptr& bdb, TriBuff& bfr3, Command_aptr& command) throw(DbException,BdbhException)
{
	bool verb=params.GetVerbose();
    bool inmemory=params.GetInmemory();
    
    switch (params.GetCommand())
    {
        case BDBH_CREATE:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OCREATE,verb);
            command = (Command_aptr) new Create(params,*bdb.get());
            break;
        }
        /*
        case BDBH_CONVERT:
        {
            if (bdb.get()==NULL)
            {
                // Try to open the file to convert in write mode, then close the file
                {
                    ofstream tmp_f (params.GetDatabase().c_str(),ios_base::app);
                    if (!tmp_f)
                    {
                        string msg = "Cannot convert ";
                        msg += params.GetDatabase();
                        msg += " because the file is write protected";
                        throw(BdbhException(msg));
                    }
                    tmp_f.close();
                }
                
                // Open in read only the database to convert, just to be sure it is a correct file
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OREAD,verb);
                
                // Close the database
                bdb = (BerkeleyDb_aptr) NULL;

                // Move this file to a temporary name
                string tmp_path = params.GetDatabase() + "_TMP";
                int err = link(params.GetDatabase().c_str(), tmp_path.c_str());
                if (err==-1)
                {
                    string msg = "ERROR - ";
                    msg += strerror(errno);
                    throw(BdbhException(msg));
                };
                err = unlink(params.GetDatabase().c_str());
                if (err==-1)
                {
                    string msg = "ERROR - ";
                    msg += strerror(errno);
                    throw(BdbhException(msg));
                };

                // Create a new database
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OCREATE,verb);
                
                // close the created database
                bdb = (BerkeleyDb_aptr)NULL;

                // Remove the file "database" (it is empty), and use the old file instead
                string new_path = params.GetDatabase() + "/database";
                err = unlink(new_path.c_str());
                if (err==-1)
                {
                    string msg = "ERROR - ";
                    msg += strerror(errno);
                    msg += " (bdbh internal error)";
                    throw(BdbhException(msg));
                };
                
                err = link(tmp_path.c_str(),new_path.c_str());
                if (err==-1)
                {
                    string msg = "ERROR - ";
                    msg += strerror(errno);
                    msg += " (bdbh internal error)";
                    throw(BdbhException(msg));
                };
                 
                unlink(tmp_path.c_str());
                if (err==-1)
                {
                    string msg = "ERROR - ";
                    msg += strerror(errno);
                    msg += " (bdbh internal error)";
                    throw(BdbhException(msg));
                };
                
                // Reopen the database, in OCONVERT mode
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OCONVERT,verb);
            }
            command = (Command_aptr) new Convert(params,*bdb.get());
            break;
        }
        */
        case BDBH_CAT:
        case BDBH_EXTRACT:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OREAD,verb);
            command = (Command_aptr) new Read(params,*bdb.get());
            break;
        }
        case BDBH_ADD:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OWRITE,verb);
            command = (Command_aptr) new Write(params,*bdb.get());
            break;
        }
        case BDBH_PUT:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OWRITE,verb);
            command = (Command_aptr) new Put(params,*bdb.get());
            break;
        }
        case BDBH_MKDIR:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OWRITE,verb);
            command = (Command_aptr) new Mkdir(params,*bdb.get());
            break;
        }
        case BDBH_RM:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OWRITE,verb);
            command = (Command_aptr) new Rm(params,*bdb.get());
            break;
        }
        case BDBH_LS:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OREAD,verb,inmemory);
            command = (Command_aptr) new Ls(params,*bdb.get());
			// Attach the LsPrint observer
			LsPrint ls_print_obs(params, cout);
			Ls* ls_cmd = (Ls*) command.get();
			ls_cmd->AttachObserver(ls_print_obs);
            break;
        }
        case BDBH_MV:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OWRITE,verb);
            command = (Command_aptr) new Mv(params,*bdb.get());
            break;
        }
        case BDBH_INFO:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OINFO,verb);
            command = (Command_aptr) new Info(params,*bdb.get());
            break;
        }
        case BDBH_CHMOD:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OWRITE);
            command = (Command_aptr) new Chmod(params,*bdb.get());
            break;
        }
        case BDBH_MERGE:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OWRITE);
            command = (Command_aptr) new Merge(params,*bdb.get());
            break;
        }
        case BDBH_HELP:
        {
            if (bdb.get()==NULL)
                bdb = (BerkeleyDb_aptr) new BerkeleyDb(params.GetDatabase().c_str(),BDBH_OREAD,verb);
            command = (Command_aptr) new Help(params,*bdb.get());
            break;
        }
        default:
        {
            throw(BdbhException("Not implemented"));
            break;
        }
    }
    
    // Connect the new command to the SignalHandle
    SignalHandle::Connect(command.get());
    
    // Execute command
    command.get()->Exec();
}

/** Parse parses the char* and initiliazes the vector of strings
    This function is used ony in interactive mode
    
\param c The char* to parse
\param v The vector of strings sotring the result
\return true if OK, false if parse error

*/

bool Parse (const char* c, vector<string>& v)
{
    v.clear();
    string s = (string) c;
    string::size_type bgn_max = s.size() - 1;
    const char def_sep  = ' ';
    const char alt_sep1 = '"';
    const char alt_sep2 = '\'';
    const char esc      = '\\';

    char sep = def_sep;
    string::size_type bgn = 0;
    while (bgn<=bgn_max)
    {
        // skip the leading spaces, return if end of string
        while(bgn<=bgn_max && s[bgn]==def_sep)
            bgn++;
        
        // finished - no error
        if (bgn > bgn_max)
            break;
        
        // detect and skip ' or "
        if (s[bgn]==alt_sep1)
        {
            sep = alt_sep1;
            if (bgn==bgn_max)
                return false;
            else
                bgn++;
        } 
        else if (s[bgn]==alt_sep2)
        {
            sep = alt_sep2;
            if (bgn==bgn_max)
                return false;
            else
                bgn++;
        };

        // search the next separator, ignore it if preceded with the escape character 
        string::size_type end=s.find(sep,bgn);
        while (end != string::npos && s[end-1]==esc)
        {
            s.erase(end-1,1);   // NOTE end point now to the char AFTER the separator.
            bgn_max--;
            end=s.find(sep,end);
        }
        
        // If alt_sep, we should find a separator at the end
        if (sep != def_sep && end==string::npos)
            return false;
        
        // Store the token found and return to default separator
        v.push_back(s.substr(bgn,end-bgn));
        if (end==string::npos)
            break;
        else
            bgn=end+1;
        sep=def_sep;
    };
    return true;
}
        
/** List all keys

    This very low-level function is only used for debugging bdbh
    The keys are listed. That's all

\pre none
 \arg database The FILE (ie data or metadata) to open

*/

bool isEndingWith(const string& name, const string& ext) {
        size_t ext_len = ext.length();
        size_t nme_len = name.length();
        if ( ext_len < nme_len ) {
            string nme_ext = name.substr(nme_len-ext_len);
            return ( nme_ext == ext );
        } else {
            return false;
        }
}

void ListKeys(const string& database)
{
    
    bool flg_data = true;
    if ( isEndingWith(database, METADATA_NAME )) {
        flg_data = false;
    }

    Db db(0,0);
    db.open(NULL,database.c_str(),NULL,DB_UNKNOWN,DB_RDONLY,0);
    Dbc* cursor;
    db.cursor(0,&cursor,0);
    
    Dbt dbt_key;
    Dbt dbt_val;
    
    void * key_area = calloc(1024*1024,1);    // 1 Mb
    void * val_area = calloc(1024*1024,10);

    dbt_key.set_flags(DB_DBT_USERMEM);
    dbt_key.set_data(key_area);
    dbt_key.set_ulen(1024*1024);

    dbt_val.set_flags(DB_DBT_USERMEM);
    dbt_val.set_data(val_area);
    dbt_val.set_ulen(1024*1024);
    //dbt_val.set_dlen(1024*1024);
    //dbt_val.set_doff(0);
    
    int rvl=0;
    cout.setf(ios::left,ios::adjustfield);  
    while(rvl==0)
    {
        rvl = cursor->get(&dbt_key,&dbt_val,DB_NEXT);
        if (rvl==0) {
            if (!flg_data) {
                Mdata* mdata = (Mdata*) dbt_val.get_data();
                bdbh_ino_t inode = mdata->ino;
                cout << setw(132) << setfill('-') << (char*) dbt_key.get_data() << " " << inode << '\n';
            } else {
                bdbh_ino_t* inode = (bdbh_ino_t*) dbt_key.get_data();
                cout << setw(10) << setfill(' ') << *inode;
                char* val = (char*) dbt_val.get_data();
                //val += sizeof(Mdata);
                if (strlen(val) > 122) {
                    val[122]='\0';
                }
                cout << setw(122) << setfill('-') << val << '\n';
            }
        }
    }
    free(key_area);
    free(val_area);
}

    



