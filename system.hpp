

#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include "constypes.hpp"
using namespace std;

int callSystem(string cmd, bool err_flg=false);
void parseFilePath(const string& path, string& dir, string& name, string& base, string& ext);
vector_of_strings split(const string &);

#endif
