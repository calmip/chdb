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

#include "parameters.hpp"
#include "command.hpp"
#include "exception.hpp"
#include "merge.hpp"
using bdbh::Merge;

/**
   The constructor of the Merge object
   
   \param p Parameters
   \param d BerkeleyDb object 
   \param b The buffers
   \param to If not NULL, the merge is done from this to object pointed by to, if NULL, this is the merged database
             
*/

Merge::Merge(const Parameters& p, BerkeleyDb& d, Merge* to): Command(p,d),destination(to)
{
    bdb.IgnoreCompressionFlag();
}


/**
    If destination != NULL, call __ExecFrom, really merging the stuff
    If destination == NULL, call __Exec, for each fkey specified

*/
void Merge::Exec()
{
    if (destination != NULL)
    {
        __ExecFrom();
    }
    else
    {
        const vector<Fkey>& fkeys = prm.GetFkeys();
        
        for (unsigned int i=0; i < fkeys.size(); ++i)
        {
            if (!fkeys[i].IsLeaf())
                continue;                  // Ignore the other keys, if any
            __Exec(fkeys[i]);
            if (_IsSignalReceived())
            {
                cerr << "Interrupted by user\n";
                break;
            }
            
        }
    }
}

/** Open in read mode the db whose file_name is in the fkey passed by parameter
    

\param fkey The fkey encapsulation the database file to merge from

*/
void Merge::__Exec(const Fkey& fkey)
{
    string from_db_name = fkey.GetFileName();
    
    bool in_memory = prm.GetInmemory();
    BerkeleyDb from_db(from_db_name.c_str(),BDBH_OREAD,false,in_memory);
    if (from_db.GetCompressionFlag() != bdb.GetCompressionFlag())
        throw (BdbhException("All databases must have the same compression status"));
    
    from_db.IgnoreCompressionFlag();
    
    // Create a new Merge object, 
    Merge from(prm,from_db,this);
    
    // Merge the 2 databases
    from.Exec();
}



/** Called by Exec() when destination != NULL, ie when this is the from database

    Copy every key to the destination database
    
*/
void Merge::__ExecFrom()
{
    // expand the wilcards, getting the toplevel list of files/directories
    vector<string> k_exp = _ExpandWildcard("*");

    // For each key found, copy it to the destination database
    for (unsigned int j=0; j<k_exp.size();++j)
    {
        __ExecFromKey(k_exp[j]);
        if (_IsSignalReceived())
            break;
        
    }
}

/** Called by __ExecFrom() for each key: copy the (key,value) pair to the destination

\param key The key to copy

*/
void Merge::__ExecFromKey(const string& key)
{
    

    Mdata mdata;
    string nkey;

    // Read the key and the data
    int rvl=_ReadKeyDataCursor(key,nkey,mdata,0,true,true);
    if (rvl==DB_NOTFOUND)
    {
        throw(BdbhException("ERROR - Pb in the database " + prm.GetDatabase()));
    }
    else
    {
        if (S_ISDIR(mdata.mode))
        {
            __ExecFromDir(key,mdata);
        }
        else if (S_ISREG(mdata.mode) || S_ISLNK(mdata.mode))
        {
            __ExecFromFile(key,mdata);
        }
        else
        {
            throw(BdbhException("ERROR - unsupported data type (may be data are corrupted ?)"));
        }
    }
}

/** recursively copy the directory passed by parameter

    When __ExecFromDir is called, the key has already been read and should point to a directory.
    mkdir is called and the directory is kept in a stack (for the further chmod)
    If recursivity is asked (see fkey), and if it is not too deep (--level), 
    __ExecFile, __ExecSysmLink or __ExecDir is called

\param key The fkey to extract (a string)
\param mdata The corresponding metadata
        
*/
void Merge::__ExecFromDir(const string& key, Mdata mdata)
{

   int rvl=0;    
    
    // The directories are NEVER overwritten
    if (key!="" && !destination->_IsInDb(key.c_str()))
    {
        // Write the key, with 0-sized data buffer, to the destination
        GetDataBfr().SetSize(0);
        mdata.ino = destination->_NextInode();
        destination->_WriteKeyData(key,mdata);
        
        // Update the destination's global information
        destination->_UpdateDbSize(0,0,key.size(),0,1,0);
        
        // Create also the marker
        mdata.mode = S_IFREG | 0666;
        string marker = key + DIRECTORY_MARKER;
        destination->_WriteKeyData(marker,mdata);
        
        // Do not count the file, only the key size
        destination->_UpdateDbSize(0,0,marker.size(),0,0,0);          
        
        prm.Log("key " + key + " merged (directory)",cerr);
    }
    
    // Explore this directory
    int lvl = CountLevel(key.c_str())+1;
    string nkey;
    string marker = key+DIRECTORY_MARKER;
    rvl=_ReadKeyDataCursor(marker,nkey,mdata,lvl,true);
    if (rvl==DB_NOTFOUND)
    {
        string msg = "Problem in the database: the special file #" + marker +"# does not exsit"; 
        throw(BdbhException(msg));
    }
    
    while(rvl!=DB_NOTFOUND)
    {
        rvl=_ReadKeyDataCursor(key,nkey,mdata,lvl,false,true);
        if (rvl != DB_NOTFOUND)
        {
            if (S_ISDIR(mdata.mode))
            {
                __ExecFromDir(nkey,mdata);
            }
            else if (S_ISREG(mdata.mode) || S_ISLNK(mdata.mode))
            {
                __ExecFromFile(nkey, mdata);
            } 
            else
            {
                throw(BdbhException("ERROR - unsupported data type (may be data are corrupted ?)"));
            }
        }
        if (_IsSignalReceived())
            break;
    }
    return;
}

/** Copy the file passed by parameter

    When __ExecFile is called, the key has already been read and should point to a regular file
    The data area is written to the file
    If the key is already present in the destination database, nothing happens because there is NO overwrite mode with the merge command

\param key The key to extract (a string)
\param mdata The corresponding metadata

*/
void Merge::__ExecFromFile(const string& key, Mdata& mdata)
{
    if (!destination->_IsInDb(key.c_str()))
    {
        
        // Write data to the destination
        // Ask for a new inode
        mdata.ino = destination->_NextInode();
        destination->_WriteKeyData(key,mdata);
        
        // Update the destination's global information
        destination->_UpdateDbSize(mdata.size,mdata.csize,key.size(),1,0,0);
        
        // message in verbose mode
        destination->prm.Log("key " + key + " merged (regular file or symlink)",cerr);
    }
    else
    {
        destination->prm.Log("could not merge key " + key + ": key is already present in the destination database",cerr);
    }
}

