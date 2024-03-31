// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#pragma warning(disable:4100)
#pragma warning(disable:4456)
#pragma warning(disable:26495)

#include "targetver.h"
#define _RICHEDIT_VER 0x0500
#define _CONFIGURATOR_

#include <atlbase.h>
#include <atlapp.h>
#include <atluser.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atlctrls.h>
#include <atltypes.h>
#include <atlctrlx.h>
#include <atlframe.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlsplit.h>

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#undef min
#undef max

#define MAIN_HWND_NOT_NULL

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <memory>

#include <atomic>
#include <mutex>
#include <charconv>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <unordered_set>
#include <string>
#include <string_view>
#include <algorithm>
#include <codecvt>
#include <regex>
#include <math.h>
#include <filesystem>
#include <fstream>

#include <Scintilla.h>
#include <SciLexer.h>

#include <boost/mp11/mpl_tuple.hpp>
namespace mp11 = boost::mp11;

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/container/small_vector.hpp>

#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <npdb/db_serialization.h> // support for binary_archive_special


#include <boost/variant.hpp>

#include <boost/serialization/array.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/unordered_map.hpp>

#include "serialization_d2d.h"
#include "serialization_atl_rect.h"

#include "resource.h"
#include "ribbon.h"
#include "messages.h"

#include "iter.h"

#include <d3d11.h>
#include <dxgi1_6.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <d2d1_3helper.h>
#include <D2DErr.h>
#include <dwrite_3.h>
#include "ComAssert.h"

#include "math2d.h"

#include <wrl/wrappers/corewrappers.h>
#include <wrl/client.h>

#include <Ws2tcpip.h>

#include <atlcomcli.h>

namespace wrl = Microsoft::WRL;

extern HWND g_hMainWnd;
extern std::thread::id g_main_thread_id;

#include <nplib/utils/utf8.h>
using namespace nplib::utf8;

using oid_t = uint64_t;

#define ASSERT(expr) ATLASSERT(expr)

#if defined(DEBUG) || defined(_DEBUG) 
#	define VALIDATE(func) do { auto result = func; ASSERT(result); } while(0);
#else 
#	define VALIDATE(func) func;
#endif

template<size_t MaxCount = 256>
std::string GetWindowTextToStdStringW(HWND hWnd) {
	wchar_t buffer[MaxCount];
	int len = ::GetWindowTextW(hWnd, buffer, MaxCount);
	return narrow(buffer, len);
}

template<size_t MaxCount = 256>
std::string GetWindowTextToStdStringA(HWND hWnd) {
	char buffer[MaxCount];
	int len = ::GetWindowTextA(hWnd, buffer, MaxCount);
	if (len == 0) return std::string();
	return std::string(buffer, len);
}

#include <nplib/utils/format.h>
using namespace nplib::format;

using namespace std::string_view_literals;


inline std::ostream& operator << (std::ostream& os, const CPoint& pt) noexcept {
	os << '[' << pt.x << ", " << pt.y << ']';
	return os;
}

inline std::ostream& operator << (std::ostream& os, const CSize& sz) noexcept {
	os << '[' << sz.cx << ", " << sz.cy << ']';
	return os;
}

inline std::ostream& operator << (std::ostream& os, const CRect& rc) noexcept {
	os << '[' << rc.left << ", " << rc.top << ", " << rc.right << ", " << rc.bottom << ']';
	return os;
}