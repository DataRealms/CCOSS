#ifndef _RTEINCLUDES_
#define _RTEINCLUDES_

// This restores features removed from the C++17 standard (auto_ptr and some other stuff). This is needed for LuaBind to work because it relies on it heavily.
#define _HAS_AUTO_PTR_ETC 1
#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

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
#include <atomic>
#include <execution>

namespace std {

	/// <summary>
	/// Custom std::hash specialization to allow using std::array as key in hash table based containers.
	/// </summary>
	template <typename Type, size_t Size> struct hash<array<Type, Size>> {
		size_t operator()(const array<Type, Size> &arr) const {
			hash<Type> hasher;
			size_t outHash = 0;
			for (size_t i = 0; i < Size; ++i) {
				// Stolen from java.lang.String.hashCode. It seems to be a popular number, because it is prime, and 31 * x can be implemented quite efficiently as (x << 5) - x.
				outHash = outHash * 31 + hasher(arr[i]);
			}
			return outHash;
		}
	};
}
#endif
