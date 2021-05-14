#ifndef _RTEINCLUDES_
#define _RTEINCLUDES_

// This restores features removed from the C++17 standard (auto_ptr and some other stuff). This is needed for LuaBind to work because it relies on it heavily.
#define _HAS_AUTO_PTR_ETC 1

// Disable a bunch of unneeded crap in WinAPI (on top of WIN32_LEAN_AND_MEAN)
#ifdef _WIN32
#define NOMINMAX
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOMETAFILE
#define NOSCROLL
#define NOTEXTMETRIC
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NODRAWTEXT
#define NOCOLOR
#define NOCTLMGR
#define NOMSG
#define NOOPENFILE
#define NOSERVICE
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#endif

// Inclusion of relevant C++ Standard Library headers.
#include <cstdlib>
#include <cstdarg>
#include <cstddef>
#include <chrono>
#include <functional>
#include <thread>
#include <mutex>
#include <cctype>
#include <string>
#include <cstring>
#include <vector>
#include <stack>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>
#include <regex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cerrno>
#include <cassert>
#include <limits>
#include <random>
#include <array>
#include <filesystem>

// TODO: Get rid of these once alias qualifiers are added.
using std::string;
using std::list;
using std::pair;
using std::deque;
using std::map;
using std::set;
using std::vector;
using std::ios_base;
using std::array;
using std::make_pair;
using std::min;
using std::max;

#endif
