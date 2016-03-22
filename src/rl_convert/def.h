#pragma once

/*
KRKR PSBFILE Compiler/Decompiler

Author:201724
QQ:495159
EMail:number201724@me.com
*/

#include <iostream>
#include <fstream>
#include <string>
#include <set>

using namespace std;

#ifdef _WIN32

#define bzero(ptr, size) memset(ptr, 0, size)

#endif