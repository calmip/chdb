

#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
using namespace std;

int callSystem(string cmd, bool err_flg=false);
void parseFilePath(const string& path, string& dir, string& name, string& base, string& ext);

#endif
