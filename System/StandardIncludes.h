#ifndef _RTEINCLUDES_
#define _RTEINCLUDES_

// This restores features removed from the C++17 standard (auto_ptr and some other stuff). This is needed for LuaBind to work because it relies on it heavily.
#define _HAS_AUTO_PTR_ETC 1

// Inclusion of relevant C++ Standard Library headers.
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Linux specific headers that MSVC uses implicitly
#ifdef __unix__
// Since GCC does not implement buffer safe functions until c++17
// This will be needed once c++17 is enabled
#define __STDC_WANT_LIB_EXT1__ 1
#endif

// TODO: Get rid of these once alias qualifiers are added.
using std::array;
using std::deque;
using std::ios_base;
using std::list;
using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::set;
using std::string;
using std::vector;

#endif