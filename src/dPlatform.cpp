// ===========================================================
//     _/_/_/   _/_/_/  _/_/_/_/    _/_/_/_/  _/_/_/   _/_/_/
//      _/    _/       _/             _/    _/    _/   _/   _/
//     _/    _/       _/_/_/_/       _/    _/    _/   _/_/_/
//    _/    _/       _/             _/    _/    _/   _/
// _/_/_/   _/_/_/  _/_/_/_/_/     _/     _/_/_/   _/_/
// ===========================================================
//
// dPlatform.cpp: Functions for independent platforms
//
// Copyright (C) 2007 - 2014	Xiuwen Zheng
//
// This file is part of CoreArray.
//
// CoreArray is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License Version 3 as
// published by the Free Software Foundation.
//
// CoreArray is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with CoreArray.
// If not, see <http://www.gnu.org/licenses/>.

#include <CoreDEF.h>
#include <dPlatform.h>
#include <math.h>
#include <time.h>

#include <float.h>
#include <sys/stat.h>

#if defined(COREARRAY_CC_GNU_MINGW32)
#   include <process.h>
#endif

#ifdef COREARRAY_PLATFORM_UNIX

	#include <cerrno>
	#include <fcntl.h>
	#include <unistd.h>
	#include <langinfo.h>
	#include <sys/types.h>

	#if defined(COREARRAY_PLATFORM_BSD) || defined(COREARRAY_PLATFORM_MACOS)
	#  include <sys/sysctl.h>
	#endif

	#if !defined(COREARRAY_CYGWIN) && !defined(COREARRAY_PLATFORM_MACOS)
	#  include <sys/sysinfo.h>
	#endif

#endif

#if defined(COREARRAY_USING_R)
#   include <Rinternals.h>
#endif


using namespace std;
using namespace CoreArray;
using namespace CoreArray::_INTERNAL;


// Math Operation

const double CoreArray::NaN =
	std::numeric_limits<double>::quiet_NaN();
const double CoreArray::Infinity =
	std::numeric_limits<double>::infinity();
const double CoreArray::NegInfinity =
	-std::numeric_limits<double>::infinity();

//

#if defined(COREARRAY_PLATFORM_WINDOWS)
	const char *const CoreArray::sLineBreak = "\r\n";
	const char *const CoreArray::sFileSep = "\\";
#else
	const char *const CoreArray::sLineBreak = "\n";
	const char *const CoreArray::sFileSep = "/";
#endif

#if defined(COREARRAY_PLATFORM_WINDOWS)
	const TSysHandle CoreArray::NullSysHandle = NULL;
#else
	const TSysHandle CoreArray::NullSysHandle = 0;
#endif



TFPClass CoreArray::FloatClassify(const float val)
{
	return FloatClassify((double)val);
}

TFPClass CoreArray::FloatClassify(const double val)
{
	if (ISNAN(val))
		return fpNaN;
	else if (R_FINITE(val))
		return fpFinite;
	else if (val == R_PosInf)
		return fpPosInf;
	else if (val == R_NegInf)
		return fpNegInf;
	else
		return fpNaN;
}

TFPClass CoreArray::FloatClassify(const long double val)
{
	return FloatClassify((double)val);
}

bool CoreArray::IsFinite(const float val)
{
	return (R_FINITE(val) != 0);
}

bool CoreArray::IsFinite(const double val)
{
	return (R_FINITE(val) != 0);
}

bool CoreArray::IsFinite(const long double val)
{
	return (R_FINITE(val) != 0);
}

bool CoreArray::IsNaN(const float val)
{
	return (ISNAN(val) != 0);
}

bool CoreArray::IsNaN(const double val)
{
	return (ISNAN(val) != 0);
}

bool CoreArray::IsNaN(const long double val)
{
	return (ISNAN(val) != 0);
}

bool CoreArray::IsInf(const float val)
{
	return (FloatClassify(val) == fpPosInf);
}

bool CoreArray::IsInf(const double val)
{
	return (FloatClassify(val) == fpPosInf);
}

bool CoreArray::IsInf(const long double val)
{
	return (FloatClassify(val) == fpPosInf);
}

bool CoreArray::IsNegInf(const float val)
{
	return (FloatClassify(val) == fpNegInf);
}

bool CoreArray::IsNegInf(const double val)
{
	return (FloatClassify(val) == fpNegInf);
}

bool CoreArray::IsNegInf(const long double val)
{
	return (FloatClassify(val) == fpNegInf);
}

bool CoreArray::EqaulFloat(const float v1, const float v2)
{
	if (!IsNaN(v1))
	{
		if (!IsNaN(v2))
			return (v1 == v2);
		else
			return false;
	} else
		return IsNaN(v2);
}

bool CoreArray::EqaulFloat(const double v1, const double v2)
{
	if (!IsNaN(v1))
	{
		if (!IsNaN(v2))
			return (v1 == v2);
		else
			return false;
	} else
		return IsNaN(v2);
}

bool CoreArray::EqaulFloat(const long double v1, const long double v2)
{
	if (!IsNaN(v1))
	{
		if (!IsNaN(v2))
			return (v1 == v2);
		else
			return false;
	} else
		return IsNaN(v2);
}


// =========================================================================
// Format a string
// =========================================================================

static const char *errStrToInt = "Unable to convert string to integer.";
static const char *errStrToFloat = "Unable to convert string to double.";
static const char *errFormat = "Invalid parameter 'fmt' of Format.";

string CoreArray::Format(const char *fmt, ...)
{
	int L;
	char buf[4096];
	va_list args; va_start(args, fmt);
	L = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	if (L >= 0)
		return string(buf);
	else
		throw ErrConvert(errFormat);
}

string CoreArray::_FmtNum(const char *fmt, ...)
{
	int L;
	char buf[64];
	va_list args; va_start(args, fmt);
	L = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	if (L >= 0)
		return string(buf);
	else
		throw ErrConvert(errFormat);
}


// =========================================================================
// Floating number <--> string
// =========================================================================

static string STRING_INF("Inf");
static string STRING_POS_INF("+Inf");
static string STRING_NEG_INF("-Inf");
static string STRING_NAN("NaN");

string CoreArray::FloatToStr(const float val)
{
	switch (FloatClassify(val))
	{
		case fpPosInf: return STRING_POS_INF;
		case fpNegInf: return STRING_NEG_INF;
		case fpNaN: return STRING_NAN;
		default: return _FmtNum("%.7g", val);
	}
}

string CoreArray::FloatToStr(const double val)
{
	switch (FloatClassify(val))
	{
		case fpPosInf: return STRING_POS_INF;
		case fpNegInf: return STRING_NEG_INF;
		case fpNaN: return STRING_NAN;
		default: return _FmtNum("%.15g", val);
	}
}

string CoreArray::FloatToStr(const long double val)
{
	switch (FloatClassify(val))
	{
		case fpPosInf: return STRING_POS_INF;
		case fpNegInf: return STRING_NEG_INF;
		case fpNaN: return STRING_NAN;
		default: return _FmtNum("%.17g", val);
	}
}

double CoreArray::StrToFloat(char const* str)
{
	if ((STRING_POS_INF==str) || (STRING_INF==str))
		return Infinity;
	else if (STRING_NEG_INF == str)
		return NegInfinity;
	else {
		char *p;
		double rv = strtod(str, &p);
		while (*p==' ' || *p=='\t') ++p;
		if (*p)
			throw ErrConvert(errStrToFloat);
		else
			return rv;
    }
}

bool CoreArray::StrToFloat(char const* str, double *rv)
{
	if (STRING_POS_INF == str)
	{
    	*rv = Infinity;
		return true;
	} else if (STRING_NEG_INF == str)
	{
    	*rv = NegInfinity;
		return true;
	} else {
		char *p;
		double r = strtod(str, &p);
		while (*p==' ' || *p=='\t') ++p;
		if (*p) {
			*rv = NaN; return false;
		} else {
			if (rv) *rv = r;
			return true;
		}
	}
}

double CoreArray::StrToFloatDef(char const* str, const double Default)
{
	if (STRING_POS_INF == str)
		return Infinity;
	else if (STRING_NEG_INF == str)
		return NegInfinity;
	else {
		char *p;
		double r = strtod(str, &p);
		while (*p==' ' || *p=='\t') ++p;
		if (*p)
			return Default;
		else
			return r;
    }
}


// =========================================================================
// Integer <--> string
// =========================================================================

string CoreArray::IntToStr(const C_Int8 val)
{
	return _FmtNum("%d", val);
}

string CoreArray::IntToStr(const C_UInt8 val)
{
	return _FmtNum("%d", val);
}

string CoreArray::IntToStr(const C_Int16 val)
{
	return _FmtNum("%d", val);
}

string CoreArray::IntToStr(const C_UInt16 val)
{
	return _FmtNum("%d", val);
}

string CoreArray::IntToStr(const C_Int32 val)
{
	return _FmtNum("%d", val);
}

string CoreArray::IntToStr(const C_UInt32 val)
{
	return _FmtNum("%u", val);
}

string CoreArray::IntToStr(const C_Int64 val)
{
	return _FmtNum("%lld", val);
}

string CoreArray::IntToStr(const C_UInt64 val)
{
	return _FmtNum("%llu", val);
}

long CoreArray::StrToInt(char const* str)
{
	char *p;
	long rv = strtol(str, &p, 10);
	while (*p==' ' || *p=='\t') ++p;
	if (*p)
		throw ErrConvert(errStrToInt);
	else
		return rv;
}

bool CoreArray::StrToInt(char const* str, long *rv)
{
	char *p;
	long r = strtol(str, &p, 10);
	while (*p==' ' || *p=='\t') ++p;
	if (*p) {
    	*rv = -1; return false;
	} else {
		if (rv) *rv = r;
		return true;
    }
}

long CoreArray::StrToIntDef(char const* str, const long Default)
{
	char *p;
	long rv = strtol(str, &p, 10);
	while (*p==' ' || *p=='\t') ++p;
	if (*p)
    	return Default;
	else
		return rv;
}


// =========================================================================
// UTF string conversion
// =========================================================================

namespace CoreArray  
{  
	// UTF-32 to UTF-8
	COREARRAY_INLINE static size_t utf(C_UInt32 src, C_UInt8* des)  
	{  
		if (src == 0) return 0;

		static const C_UInt8 PREFIX[] = { 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
		static const C_UInt32 CODE_UP[] =
		{
			0x80,           // U+00000000 - U+0000007F
			0x800,          // U+00000080 - U+000007FF
			0x10000,        // U+00000800 - U+0000FFFF
			0x200000,       // U+00010000 - U+001FFFFF
			0x4000000,      // U+00200000 - U+03FFFFFF
			0x80000000      // U+04000000 - U+7FFFFFFF
		};

		size_t i, len = sizeof(CODE_UP) / sizeof(C_UInt32);
		for(i = 0; i < len; ++i)
			if (src < CODE_UP[i]) break;
		if (i == len) return 0; // the src is invalid

		len = i + 1;
		if (des)
		{
			for(; i > 0; i--)
			{
				des[i] = (C_UInt8)((src & 0x3F) | 0x80);
				src >>= 6;
			}
			des[0] = (C_UInt8)(src | PREFIX[len - 1]);
		}

		return len;  
	}
   
	// UTF-8 to UTF-32 
	COREARRAY_INLINE static size_t utf(const C_UInt8* src, C_UInt32& des)  
	{
		if (!src || (*src) == 0) return 0;

		C_UInt8 b = *(src++);
		if (b < 0x80)  
		{
			des = b; return 1;
		}
		if ((b < 0xC0) || (b > 0xFD)) return 0; // the src is invalid

		size_t len;
		if (b < 0xE0)
		{
			des = b & 0x1F; len = 2;
		} else if (b < 0xF0)
		{
			des = b & 0x0F; len = 3;
		} else if (b < 0xF8)
		{  
			des = b & 0x07; len = 4;  
		} else if (b < 0xFC)
		{  
			des = b & 0x03; len = 5;
		} else
		{  
			des = b & 0x01; len = 6;  
		}  

		for (size_t i=1; i < len; i++)
		{
			b = *(src++);
			if (b < 0x80 || b > 0xBF) return 0; // the src is invalid
			des = (des << 6) + (b & 0x3F);
		}
		return len;
	}  
   
	// UTF-32 to UTF-16 
	COREARRAY_INLINE static size_t utf(C_UInt32 src, C_UInt16* des)
	{
		if (src == 0) return 0;

		if (src <= 0xFFFF)
		{
			if (des) (*des) = (C_UInt16)src;
			return 1;  
		} else if (src <= 0xEFFFF)
		{  
			if (des)  
			{  
				// high
				des[0] = (C_UInt16)(0xD800 + (src >> 10) - 0x40);
				// low
				des[1] = (C_UInt16)(0xDC00 + (src & 0x03FF));
			}  
			return 2;  
		}  
		return 0;  
	}  
   
	// UTF-16 to UTF-32 
	COREARRAY_INLINE static size_t utf(const C_UInt16* src, C_UInt32& des)  
	{  
		if (!src || (*src) == 0) return 0;

		C_UInt16 w1 = src[0];
		if (w1 >= 0xD800 && w1 <= 0xDFFF)
		{
			if (w1 < 0xDC00)
			{
				C_UInt16 w2 = src[1];
				if (w2 >= 0xDC00 && w2 <= 0xDFFF)
				{
					des = (w2 & 0x03FF) + (((w1 & 0x03FF) + 0x40) << 10);
					return 2;
				}
			}
			return 0; // the src is invalid
		} else {
			des = w1;  
			return 1;  
		}  
	}  

	// UTF-16 to UTF-8
	COREARRAY_INLINE static size_t utf(C_UInt16 src, C_UInt8* des)
	{
		// make utf-16 to utf-32
		C_UInt32 tmp;
		if (utf(&src, tmp) != 1) return 0;
		// make utf-32 to utf-8
		return utf(tmp, des);
	}
   
	// UTF-8 to UTF-16
	COREARRAY_INLINE static size_t utf(const C_UInt8* src, C_UInt16& des)
	{
		// make utf-8 to utf-32
		C_UInt32 tmp;
		size_t len = utf(src, tmp);
		if (len == 0) return 0;
		// make utf-32 to utf-16
		if (utf(tmp, &des) != 1) return 0;
		return len;
	}  


	// UTF-X: string to string

	template <typename T>
	size_t utf(const C_UInt32* src, T* des)   // UTF-32 to UTF-X(8/16)
	{
		if (!src || (*src) == 0) return 0;
		size_t num = 0;
		for(; *src; ++src)
		{
			size_t len = utf(*src, des);
			if (len == 0) break;
			if (des) des += len;
			num += len;
		}
		if (des) (*des) = 0;
		return num;
	}  

	template <typename T>
	size_t utf(const T* src, C_UInt32* des)   // UTF-X(8/16) to UTF-32
	{
		if (!src || (*src) == 0) return 0;
		size_t num = 0;
		while(*src)
		{
			C_UInt32 tmp;
			size_t len = utf(src, tmp);
			if (len == 0) break;
			if (des)
			{
				(*des) = tmp;
				++des;
			}
			src += len;
			num += 1;
		}
		if (des) (*des) = 0;
		return num;
	}

	template <typename T, typename U>
	size_t utf(const T* src, U* des)    // UTF-X(8/16) to UTF-Y(16/8)
	{  
		if (!src || (*src) == 0) return 0;
		size_t num = 0;
		while(*src)
		{  
			// make utf-x to ucs4
			C_UInt32 tmp;
			size_t len = utf(src, tmp);
			if (len == 0) break;
			src += len;
			// make ucs4 to utf-y
			len = utf(tmp, des);
			if (len == 0) break;
			if (des) des += len;
			num += len;
		}
		if (des) (*des) = 0;
		return num;
	}  
}


UTF16String CoreArray::UTF8ToUTF16(const UTF8String &s)
{
	const C_UInt8 *p = (C_UInt8*)(s.c_str());
	UTF16String rv;
	rv.resize(utf(p, (C_UInt16*)0));
	utf(p, (C_UInt16*)(&rv[0]));
	return rv;
}

UTF32String CoreArray::UTF8ToUTF32(const UTF8String &s)
{
	const C_UInt8 *p = (C_UInt8*)(s.c_str());
	UTF32String rv;
	rv.resize(utf(p, (C_UInt32*)0));
	utf(p, (C_UInt32*)(&rv[0]));
	return rv;
}

UTF8String CoreArray::UTF16ToUTF8(const UTF16String &s)
{
	const C_UInt16 *p = (C_UInt16*)(s.c_str());
	UTF8String rv;
	rv.resize(utf(p, (C_UInt8*)0));
	utf(p, (C_UInt8*)(&rv[0]));
	return rv;
}

UTF32String CoreArray::UTF16ToUTF32(const UTF16String &s)
{
	const C_UInt16 *p = (C_UInt16*)(s.c_str());
	UTF32String rv;
	rv.resize(utf(p, (C_UInt32*)0));
	utf(p, (C_UInt32*)(&rv[0]));
	return rv;
}

UTF8String CoreArray::UTF32ToUTF8(const UTF32String &s)
{
	const C_UInt32 *p = (C_UInt32*)(s.c_str());
	UTF8String rv;
	rv.resize(utf(p, (C_UInt8*)0));
	utf(p, (C_UInt8*)(&rv[0]));
	return rv;
}

UTF16String CoreArray::UTF32ToUTF16(const UTF32String &s)
{
	const C_UInt32 *p = (C_UInt32*)(s.c_str());
	UTF16String rv;
	rv.resize(utf(p, (C_UInt16*)0));
	utf(p, (C_UInt16*)(&rv[0]));
	return rv;
}

RawString CoreArray::RawText(const string &s)
{
	return s;
}

// RawString CoreArray::RawText(const UTF8String &s)
//{
//	return RawString(s.begin(), s.end());
//}

RawString CoreArray::RawText(const UTF16String &s)
{
	UTF8String ss = UTF16ToUTF8(s);
	return RawString(ss.begin(), ss.end());
}

RawString CoreArray::RawText(const UTF32String &s)
{
	UTF8String ss = UTF32ToUTF8(s);
	return RawString(ss.begin(), ss.end());
}

UTF8String CoreArray::UTF8Text(const char *s)
{
	return UTF8String(s, s+strlen(s));
}

UTF8String CoreArray::UTF8Text(const string &s)
{
	return UTF8String(s.begin(), s.end());
}

// UTF8String CoreArray::UTF8Text(const UTF8String &s)
// {
//	return s;
// }

UTF8String CoreArray::UTF8Text(const UTF16String &s)
{
	return UTF16ToUTF8(s);
}

UTF8String CoreArray::UTF8Text(const UTF32String &s)
{
	return UTF32ToUTF8(s);
}

UTF16String CoreArray::UTF16Text(const char *s)
{
	return UTF8ToUTF16(UTF8String(s, s+strlen(s)));
}

UTF16String CoreArray::UTF16Text(const string &s)
{
	return UTF8ToUTF16(UTF8String(s.begin(), s.end()));
}

// UTF16String CoreArray::UTF16Text(const UTF8String &s)
// {
//	return UTF8ToUTF16(s);
// }

UTF16String CoreArray::UTF16Text(const UTF16String &s)
{
	return s;
}

UTF16String CoreArray::UTF16Text(const UTF32String &s)
{
	return UTF32ToUTF16(s);
}

UTF32String CoreArray::UTF32Text(const char *s)
{
	return UTF8ToUTF32(UTF8String(s, s+strlen(s)));
}

UTF32String CoreArray::UTF32Text(const string &s)
{
	return UTF8ToUTF32(UTF8String(s.begin(), s.end()));
}

// UTF32String CoreArray::UTF32Text(const UTF8String &s)
// {
//	return UTF8ToUTF32(s);
// }

UTF32String CoreArray::UTF32Text(const UTF16String &s)
{
	return UTF16ToUTF32(s);
}

UTF32String CoreArray::UTF32Text(const UTF32String &s)
{
	return s;
}




// =========================================================================
// Time Date --> String
// =========================================================================

string CoreArray::NowDateToStr()
{
	time_t tm;
	time(&tm);
	string rv(ctime(&tm));
	rv.erase(rv.size()-1, 1);
	return rv;
}


// =========================================================================
// File Functions
// =========================================================================

TSysHandle CoreArray::SysCreateFile(char const* const AFileName, C_UInt32 Mode)
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		TSysHandle H;
		H = CreateFile(AFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		return (H != INVALID_HANDLE_VALUE) ? H : NULL;
	#else
		TSysHandle H;
		#ifdef O_LARGEFILE
			int flag = O_LARGEFILE;
		#else
			int flag = 0;
		#endif
		H = open(AFileName, flag | O_CREAT | O_RDWR | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		return (H > 0) ? H : 0;
	#endif
}

#if defined(COREARRAY_PLATFORM_WINDOWS)
	static const DWORD AccessMode[3] =
		{ GENERIC_READ, GENERIC_WRITE, GENERIC_READ | GENERIC_WRITE };
	static const DWORD ShareMode[4] =
		{ 0, FILE_SHARE_READ, FILE_SHARE_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE};
#else
	static const int AccessMode[3] =
		{ O_RDONLY, O_WRONLY, O_RDWR };
	#ifdef O_SHARE_NONE
	static const int ShareMode[4] =
		{ O_SHARE_NONE, O_SHARE_RDONLY, O_SHARE_WRONLY, O_SHARE_RDWR };
	#else
	static const int ShareMode[4] =
		{ 0, 0, 0, 0 };
	#endif
#endif

TSysHandle CoreArray::SysOpenFile(char const* const AFileName,
	enum TSysOpenMode mode, enum TSysShareMode smode)
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		TSysHandle H;
		H = CreateFile(AFileName, AccessMode[mode], ShareMode[smode], NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		return (H != INVALID_HANDLE_VALUE) ? H : NULL;
	#else
		TSysHandle H;
		#ifdef O_LARGEFILE
			int flag = O_LARGEFILE;
		#else
			int flag = 0;
		#endif
		H = open(AFileName, flag | AccessMode[mode] |
			ShareMode[smode]);
		return (H > 0) ? H : 0;
	#endif
}

bool CoreArray::SysCloseHandle(TSysHandle Handle)
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		return CloseHandle(Handle);
	#else
		return close(Handle)==0;
	#endif
}

size_t CoreArray::SysHandleRead(TSysHandle Handle, void *Buffer, size_t Count)
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		unsigned long rv;
		if (ReadFile(Handle, Buffer, Count, &rv, NULL))
			return rv;
		else
			return 0;
	#else
		ssize_t rv = read(Handle, Buffer, Count);
		return (rv >= 0) ? rv : 0;
	#endif
}

size_t CoreArray::SysHandleWrite(TSysHandle Handle, const void* Buffer,
	size_t Count)
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		unsigned long rv;
		if (WriteFile(Handle, Buffer, Count, &rv, NULL))
			return rv;
		else
			return 0;
	#else
		ssize_t rv = write(Handle, Buffer, Count);
		return (rv >= 0) ? rv : 0;
	#endif
}

C_Int64 CoreArray::SysHandleSeek(TSysHandle Handle, C_Int64 Offset,
	enum TdSysSeekOrg sk)
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		C_Int64 p = Offset;
		DWORD *Lo = (DWORD*)&p;
		DWORD *Hi = ((DWORD*)&p) + 1;

		*Lo = SetFilePointer(Handle, *Lo, (long*)Hi, sk);
		if (*Lo==INVALID_SET_FILE_POINTER && GetLastError()!=0)
			return -1;
		else
			return p;
	#else
		#if defined(COREARRAY_CYGWIN) || defined(COREARRAY_PLATFORM_MACOS)
			return lseek(Handle, Offset, sk);
		#else
			return lseek64(Handle, Offset, sk);
		#endif
	#endif
}

bool CoreArray::SysHandleSetSize(TSysHandle Handle, C_Int64 NewSize)
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		if (SysHandleSeek(Handle, NewSize, soBeginning)>=0)
			return SetEndOfFile(Handle);
		else
			return false;
	#else
		#if defined(COREARRAY_CYGWIN) || defined(COREARRAY_PLATFORM_MACOS)
			return ftruncate(Handle, NewSize)==0;
		#else
			return ftruncate64(Handle, NewSize)==0;
		#endif
	#endif
}

string CoreArray::TempFileName(const char *prefix, const char *tempdir)
{
	char *fn = R_tmpnam(prefix, tempdir);
	string s(fn);
	free(fn);
	return s;
}

bool CoreArray::FileExists(const string &FileName)
{
	struct stat sb;
	return (stat(FileName.c_str(), &sb) == 0);
}


// =========================================================================
// System Error information
// =========================================================================

int CoreArray::GetLastOSError()
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		return GetLastError();
	#else
		return errno;
	#endif
}

string CoreArray::SysErrMessage(int err)
{
	#if defined(COREARRAY_PLATFORM_WINDOWS)
		char buf[1024];
		memset((void*)buf, 0, sizeof(buf));
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_ARGUMENT_ARRAY, NULL, err, 0,
			buf, sizeof(buf), NULL);
		return string(buf);
	#elif defined(COREARRAY_PLATFORM_UNIX)
		return string(strerror(err));
	#endif
}

string CoreArray::LastSysErrMsg()
{
	return SysErrMessage(GetLastOSError());
}


// =========================================================================
// CPU functions
// =========================================================================

int CoreArray::Mach::GetCPU_NumOfCores()
{
#if defined(COREARRAY_PLATFORM_WINDOWS)

	SYSTEM_INFO info;
	info.dwNumberOfProcessors = 0;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;

#elif defined(COREARRAY_CYGWIN)

	const char * p = getenv("NUMBER_OF_PROCESSORS");
	if (p)
	{
		int rv = atoi(p);
		if (rv < 0) rv = 0;
		return rv;
	} else
		return 0;

#elif defined(COREARRAY_PLATFORM_UNIX)

	#ifdef _SC_NPROCESSORS_ONLN
		int n = sysconf(_SC_NPROCESSORS_ONLN);
		if (n < 0) n = 0;
		return n;
	#else
		return 0;
	#endif

#else
	return 0;
#endif
}


#if defined(COREARRAY_PLATFORM_WINDOWS)

	enum MY_LOGICAL_PROCESSOR_RELATIONSHIP {
		My_RelationProcessorCore = 0,
		My_RelationNumaNode,
		My_RelationCache,
		My_RelationProcessorPackage
	};

	enum MY_PROCESSOR_CACHE_TYPE {
		My_CacheUnified = 0,
		My_CacheInstruction,
		My_CacheData,
		My_CacheTrace
	};

	struct MY_CACHE_DESCRIPTOR {
		BYTE   Level;
		BYTE   Associativity;
		WORD   LineSize;
		DWORD  Size;
		MY_PROCESSOR_CACHE_TYPE Type;
	};

	typedef struct {
		ULONG_PTR   ProcessorMask;
		MY_LOGICAL_PROCESSOR_RELATIONSHIP Relationship;
		union {
			struct {
				BYTE  Flags;
			} ProcessorCore;
			struct {
				DWORD NodeNumber;
			} NumaNode;
			MY_CACHE_DESCRIPTOR Cache;
			ULONGLONG  Reserved[2];
		} DUMMYUNIONNAME;
	} MY_SYSTEM_LOGICAL_PROCESSOR_INFORMATION,
	*MY_PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;

	typedef BOOL (WINAPI *LPFN_GLPI)(
		MY_PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

#endif


C_UInt64 CoreArray::Mach::GetCPU_LevelCache(int level)
{
	if (level < 0) return 0;

#if defined(COREARRAY_PLATFORM_WINDOWS)

	LPFN_GLPI glpi;
	BOOL done = FALSE;
	MY_PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	MY_PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	DWORD returnLength = 0;
	DWORD byteOffset = 0;
	DWORD rv = 0;

	glpi = (LPFN_GLPI)GetProcAddress(
		GetModuleHandle("kernel32"), "GetLogicalProcessorInformation");
	if (glpi == NULL) return 0;

	while (!done)
	{
		DWORD rc = glpi(buffer, &returnLength);
		if (rc == FALSE)
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (buffer) free(buffer);
				buffer = (MY_PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
				if (buffer == NULL) return 0;
			} else
				return 0;
		} else
			done = TRUE;
	}

	ptr = buffer;
	while (byteOffset + sizeof(MY_SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
	{
		if (ptr->Relationship == My_RelationCache)
		{
			// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache.
			if (ptr->Cache.Level == level)
			{
				if ((rv == 0) || (ptr->Cache.Size < rv))
					rv = ptr->Cache.Size;
			}
		}
		byteOffset += sizeof(MY_SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		ptr++;
	}

	free(buffer);
	return rv;

#elif defined(COREARRAY_PLATFORM_BSD) || defined(COREARRAY_PLATFORM_MACOS)

	string nm;
	switch (level)
	{
		case 0:  nm = "hw.l1icachesize"; break;
		case 1:  nm = "hw.l1dcachesize"; break;
		default: nm = Format("hw.l%dcachesize", level);
	}
	uint64_t count = 0;
	size_t size = sizeof(count);
	if (sysctlbyname(nm.c_str(), &count, &size, NULL, 0) == -1)
		return 0;
	else
		return count;

#elif defined(COREARRAY_PLATFORM_LINUX)

	FILE *f = fopen(Format(
		"/sys/devices/system/cpu/cpu0/cache/index%d/size", level).c_str(),
		"r");
	if (!f) return 0;
	int x = 0;
	if (fscanf(f, "%d", &x) != EOF)
	{
		char ch = 0;
		if (fscanf(f, "%c", &ch) != EOF)
		{
			if ((ch == 'K') || (ch == 'k'))
				return C_UInt64(x) * 1024;
			else if ((ch == 'M') || (ch == 'm'))
				return C_UInt64(x) * 1024 * 1024;
			else
				return 0;
		} else
			return x;
	} else
		return 0;

#else

	if (level != 2) return 0;

	char line[200];
	FILE *f = fopen("/proc/cpuinfo", "r");
	if (!f) return 0;
	int rv_cache = 0;
	memset(line, 0, sizeof(line));

	// analyze
	while (fgets(line, 200, f) != NULL)
	{
		char *s, *value;
		// NOTE: the ':' is the only character we can rely on
		if (!(value = strchr(line, ':')))
			continue;
		// terminate the valuename
		s = value - 1;
		while ((s >= line) && ((*s == ' ') || (*s == '\t'))) s--;
		*(s + 1) = '\0';
		// and strip leading spaces from value
		value += 1;
		while (*value == ' ') value++;
		if ((s = strchr(value,'\n'))) *s='\0';

		if (!strcasecmp(line, "cache size"))
		{
			// cache size ...
			int x;
			if (sscanf(value, "%d", &x))
				rv_cache = x*1024;
			else
				rv_cache = 0;
			break;
		}
	}
	fclose(f);
	return rv_cache;

#endif
}


TProcessID CoreArray::GetCurrentProcessID()
{
#if defined(COREARRAY_PLATFORM_WINDOWS)
	return GetCurrentProcessId();
#else
	return getpid();
#endif
}


// =========================================================================
// CdThreadMutex
// =========================================================================

CdThreadMutex::CdThreadMutex()
{
	#if defined(COREARRAY_POSIX_THREAD)
    	int rv = pthread_mutex_init(&mutex, NULL);
    	if (rv != 0)
    		throw ErrOSError("pthread_mutex_init returns an error code %d.", rv);
	#elif defined(COREARRAY_PLATFORM_WINDOWS)
		InitializeCriticalSection(&mutex);
		// mutex = CreateMutex(NULL, FALSE, NULL);
		// if (mutex == NULL)
			// RaiseLastOSError<ErrOSError>();
	#else
		XXXX: need portable pthread_mutex_init
	#endif
}

CdThreadMutex::~CdThreadMutex()
{
	#if defined(COREARRAY_POSIX_THREAD)
		int rv = pthread_mutex_destroy(&mutex);
	   	if (rv != 0)
    		throw ErrOSError("pthread_mutex_destroy returns an error code %d.", rv);
	#elif defined(COREARRAY_PLATFORM_WINDOWS)
		DeleteCriticalSection(&mutex);
		// if (CloseHandle(mutex) == 0)
			// RaiseLastOSError<ErrOSError>();
	#else
		XXXX: need portable pthread_mutex_destroy
	#endif
}

void CdThreadMutex::Lock()
{
	#if defined(COREARRAY_POSIX_THREAD)
		int rv = pthread_mutex_lock(&mutex);
	   	if (rv != 0)
    		throw ErrOSError("pthread_mutex_lock returns an error code %d.", rv);
	#elif defined(COREARRAY_PLATFORM_WINDOWS)
		EnterCriticalSection(&mutex);
		// if (WaitForSingleObject(mutex, INFINITE) == WAIT_FAILED)
			// RaiseLastOSError<ErrOSError>();
	#else
		XXXX: need portable pthread_mutex_lock
	#endif
}

void CdThreadMutex::Unlock()
{
	#if defined(COREARRAY_POSIX_THREAD)
		int rv = pthread_mutex_unlock(&mutex);
	   	if (rv != 0)
    		throw ErrOSError("pthread_mutex_unlock returns an error code %d.", rv);
	#elif defined(COREARRAY_PLATFORM_WINDOWS)
		LeaveCriticalSection(&mutex);
		// if (!ReleaseMutex(mutex))
			// RaiseLastOSError<ErrOSError>();
	#else
		XXXX: need portable pthread_mutex_unlock
	#endif
}

bool CdThreadMutex::TryLock()
{
	#if defined(COREARRAY_POSIX_THREAD)
		return pthread_mutex_trylock(&mutex) == 0;
	#elif defined(COREARRAY_PLATFORM_WINDOWS)
		return TryEnterCriticalSection(&mutex);
		// DWORD rv = WaitForSingleObject(mutex, INFINITE);
		// if (rv == WAIT_FAILED)
			// RaiseLastOSError<ErrOSError>();
		// return (rv == WAIT_OBJECT_0);
	#else
		XXXX: need portable pthread_mutex_trylock
	#endif
}


// =========================================================================
// CdThread
// =========================================================================

#if defined(COREARRAY_POSIX_THREAD)

	extern "C" void *COREARRAY_CALL_ALIGN16_ARG ThreadWrap1(
		void *lpThreadParameter)
	{
		CdThread *p = (CdThread*)lpThreadParameter;
		ssize_t rd = p->RunThreadSafe();
		return (void*)rd;
	}

	extern "C" void *COREARRAY_CALL_ALIGN16_ARG ThreadWrap2(
		void *lpThreadParameter)
	{
		TdThreadData *p = (TdThreadData*)lpThreadParameter;
		int &rv = p->thread->ExitCode();
		try {
			rv = (*p->proc)(p->thread, p->Data);
		}
		catch (exception &E) {
    		p->thread->ErrorInfo() = E.what();
			rv = -1;
		}
		catch (...) {
			rv = -1;
		}
		ssize_t rd = rv;
		return (void*)rd;
	}

#elif defined(COREARRAY_PLATFORM_WINDOWS)

	DWORD WINAPI COREARRAY_CALL_ALIGN16_ARG ThreadWrap1(
		LPVOID lpThreadParameter)
	{
		CdThread *p = (CdThread*)lpThreadParameter;
		return p->RunThreadSafe();
	}

	DWORD WINAPI COREARRAY_CALL_ALIGN16_ARG ThreadWrap2(
		LPVOID lpThreadParameter)
	{
		TdThreadData *p = (TdThreadData*)lpThreadParameter;
		int &rv = p->thread->ExitCode();
		try {
			rv = (*p->proc)(p->thread, p->Data);
		}
		catch (exception &E) {
	    	p->thread->ErrorInfo() = E.what();
			rv = -1;
		}
		catch (...) {
			rv = -1;
		}
		return rv;
	}

#else
	XXXX: need portable thread functions
#endif


CdThread::CdThread()
{
	terminated = false;
	fExitCode = 0;
	vPrivate = NULL;
	memset(&thread, 0, sizeof(thread));
}

CdThread::CdThread(TdThreadProc proc, void *Data)
{
	terminated = false;
	fExitCode = 0;
	vPrivate = NULL;
	memset(&thread, 0, sizeof(thread));
	vData.thread = this; vData.proc = proc; vData.Data = Data;
	_BeginThread();
}

CdThread::~CdThread()
{
	try {
		Terminate();
		EndThread();
	} catch (...) {
		Done(); throw;
	}
	Done();
	if (vPrivate) delete vPrivate;
}

void CdThread::Done()
{
#if defined(COREARRAY_POSIX_THREAD)
	if (thread)
	{
		pthread_detach(thread);
		thread = 0;
	}
#elif defined(COREARRAY_PLATFORM_WINDOWS)
	if (thread.Handle != NULL)
	{
		CloseHandle(thread.Handle);
		thread.Handle = NULL;
	}
#else
	XXXX: need portable pthread_detach
#endif
}

void CdThread::BeginThread()
{
#if defined(COREARRAY_POSIX_THREAD)
	if (!thread)
	{
		int v = pthread_create(&thread, NULL, ThreadWrap1, (void*)this);
		if (v != 0)
			throw ErrOSError("pthread_create exited with error id (%d).", v);
	} else
    	throw ErrThread("BeginThread");
#elif defined(COREARRAY_PLATFORM_WINDOWS)
	if (thread.Handle == NULL)
	{
		SECURITY_ATTRIBUTES attr;
			attr.nLength = sizeof(attr);
			attr.lpSecurityDescriptor = NULL;
			attr.bInheritHandle = true;
		thread.Handle = CreateThread(&attr, 0, ThreadWrap1, (void*)this,
			0, &thread.ThreadID);
		if (thread.Handle == NULL)
			RaiseLastOSError<ErrThread>();
	} else
    	throw ErrThread("BeginThread");
#else
	XXXX: need portable pthread_create
#endif
}

void CdThread::_BeginThread()
{
#if defined(COREARRAY_POSIX_THREAD)
	if (!thread)
	{
		int v = pthread_create(&thread, NULL, ThreadWrap2, (void*)&vData);
		if (v != 0)
           	throw ErrOSError("pthread_create exited with error id (%d).", v);
	} else
    	throw ErrThread("_BeginThread");
#elif defined(COREARRAY_PLATFORM_WINDOWS)
	if (thread.Handle == NULL)
	{
		SECURITY_ATTRIBUTES attr;
			attr.nLength = sizeof(attr);
			attr.lpSecurityDescriptor = NULL;
			attr.bInheritHandle = true;
		thread.Handle = CreateThread(&attr, 0, ThreadWrap2, (void*)&vData,
			0, &thread.ThreadID);
		if (thread.Handle == NULL)
			RaiseLastOSError<ErrThread>();
	} else
    	throw ErrThread("_BeginThread");
#else
	XXXX: need portable pthread_create
#endif
}

int CdThread::RunThreadSafe()
{
	try {
		fExitCode = RunThread();
	}
	catch (exception &E) {
    	fErrorInfo = E.what();
		fExitCode = -1;
	}
	catch (...) {
        fExitCode = -1;
    }
	return fExitCode;
}

int CdThread::RunThread()
{
	return 0;
}

void CdThread::Terminate()
{
	terminated = true;
}

int CdThread::EndThread()
{
#if defined(COREARRAY_POSIX_THREAD)
	if (thread)
	{
		int v = pthread_join(thread, NULL);
		if (v != 0)
			throw ErrOSError("pthread_join exited with error id (%d).", v);
		Done();
	}
#elif defined(COREARRAY_PLATFORM_WINDOWS)
	if (thread.Handle != NULL)
	{
		if (WaitForSingleObject(thread.Handle, INFINITE) == WAIT_FAILED)
			RaiseLastOSError<ErrThread>();
		Done();
	}
#else
	XXXX: need portable pthread_join
#endif
	return fExitCode;
}


// CdThreadSuspending

CdThreadsSuspending::CdThreadsSuspending()
{
#if defined(COREARRAY_POSIX_THREAD)
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&threshold, NULL);
#elif defined(COREARRAY_PLATFORM_WINDOWS)
	waiters_count_ = 0;
	wait_generation_count_ = 0;
	release_count_ = 0;
	// Create a critical section
	InitializeCriticalSection(&waiters_count_lock_);
	// Create a manual-reset event.
	event_ = CreateEvent(NULL,  // no security
						TRUE,  // manual-reset
						FALSE, // non-signaled initially
						NULL); // unnamed
#else
	XXXX: need portable pthread_mutex_init and pthread_cond_init
#endif
}

CdThreadsSuspending::~CdThreadsSuspending()
{
    WakeUp();
#if defined(COREARRAY_POSIX_THREAD)
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&threshold);
#elif defined(COREARRAY_PLATFORM_WINDOWS)
	DeleteCriticalSection(&waiters_count_lock_);
	CloseHandle(event_);
#else
	XXXX: need portable pthread_mutex_destroy and pthread_cond_destroy
#endif
}

void CdThreadsSuspending::Suspend()
{
#if defined(COREARRAY_POSIX_THREAD)

	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&threshold, &mutex);
	pthread_mutex_unlock(&mutex);

#elif defined(COREARRAY_PLATFORM_WINDOWS)

	// Avoid conditions
	EnterCriticalSection(&waiters_count_lock_);
	// Increment count of waiters
	waiters_count_ ++;
	// Store current generation in our activation record
	int my_generation = wait_generation_count_;
	// Leave conditions
	LeaveCriticalSection(&waiters_count_lock_);

	for (;;)
	{
		// Wait until the event is signaled.
		WaitForSingleObject(event_, INFINITE);
		EnterCriticalSection(&waiters_count_lock_);
		// Exit the loop when the <event_> is signaled and
		// there are still waiting threads from this <wait_generation>
		// that haven't been released from this wait yet.
		bool wait_done = (release_count_ > 0)
                    && (wait_generation_count_ != my_generation);
		LeaveCriticalSection(&waiters_count_lock_);
		if (wait_done) break;
	}

	EnterCriticalSection(&waiters_count_lock_);
	waiters_count_ --;
	release_count_ --;
	bool last_waiter = (release_count_ == 0);
	LeaveCriticalSection(&waiters_count_lock_);

	if (last_waiter)
	{
		// We're the last waiter to be notified, so reset the manual event.
		ResetEvent (event_);
	}

#else
	XXXX: need portable pthread_cond_wait
#endif
}

void CdThreadsSuspending::WakeUp()
{
#if defined(COREARRAY_POSIX_THREAD)

	pthread_mutex_lock(&mutex);
	pthread_cond_broadcast(&threshold);
	pthread_mutex_unlock(&mutex);

#elif defined(COREARRAY_PLATFORM_WINDOWS)

	EnterCriticalSection(&waiters_count_lock_);
	if (waiters_count_ > 0)
	{
		SetEvent(event_);
		// Release all the threads in this generation.
		release_count_ = waiters_count_;
		// Start a new generation.
		wait_generation_count_ ++;
	}
	LeaveCriticalSection(&waiters_count_lock_);

#else
	XXXX: need portable pthread_cond_broadcast
#endif
}


// ErrCoreArray

void ErrCoreArray::Init(const char *fmt, va_list arglist)
{
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, arglist);
	fMessage = buf;
}
