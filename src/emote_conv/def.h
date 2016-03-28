#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "../psb.hpp"

using namespace std;

#ifdef _WIN32

#define bzero(ptr, size) memset(ptr, 0, size)

#endif