#pragma once
#include <iostream>
#include <Windows.h>
#include <string>

class InitLoader
{
public:

	static InitLoader* createNew(const wchar_t* FullPath = L"");
	//static InitLoader* createNew(const char* FullPath = "");

	std::string Load(const wchar_t* Section, const wchar_t* Key);
	std::string Load(const char* Section, const char* Key);

	~InitLoader();

private:

	InitLoader(const wchar_t* iniName);

	//TCHAR FullPath[2048];
	TCHAR IniPath[2048];
	std::string MFullPath;
};

//// Read an INI file into easy-to-access name/value pairs.
//
//// SPDX-License-Identifier: BSD-3-Clause
//
//// Copyright (C) 2009-2019, Ben Hoyt
//
//// inih and INIReader are released under the New BSD license (see LICENSE.txt).
//// Go to the project home page for more info:
////
//// https://github.com/benhoyt/inih
//
//#ifndef __INIREADER_H__
//#define __INIREADER_H__
//
//#include <map>
//#include <string>
//
//// Read an INI file into easy-to-access name/value pairs. (Note that I've gone
//// for simplicity here rather than speed, but it should be pretty decent.)
//class INIReader
//{
//public:
//	// Construct INIReader and parse given filename. See ini.h for more info
//	// about the parsing.
//	explicit INIReader(const std::string& filename);
//
//	// Return the result of ini_parse(), i.e., 0 on success, line number of
//	// first error on parse error, or -1 on file open error.
//	int ParseError() const;
//
//	// Get a string value from INI file, returning default_value if not found.
//	std::string Get(const std::string& section, const std::string& name,
//		const std::string& default_value) const;
//
//	// Get a string value from INI file, returning default_value if not found,
//	// empty, or contains only whitespace.
//	std::string GetString(const std::string& section, const std::string& name,
//		const std::string& default_value) const;
//
//	// Get an integer (long) value from INI file, returning default_value if
//	// not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
//	long GetInteger(const std::string& section, const std::string& name, long default_value) const;
//
//	// Get a real (floating point double) value from INI file, returning
//	// default_value if not found or not a valid floating point value
//	// according to strtod().
//	double GetReal(const std::string& section, const std::string& name, double default_value) const;
//
//	// Get a boolean value from INI file, returning default_value if not found or if
//	// not a valid true/false value. Valid true values are "true", "yes", "on", "1",
//	// and valid false values are "false", "no", "off", "0" (not case sensitive).
//	bool GetBoolean(const std::string& section, const std::string& name, bool default_value) const;
//
//	// Return true if the given section exists (section must contain at least
//	// one name=value pair).
//	bool HasSection(const std::string& section) const;
//
//	// Return true if a value exists with the given section and field names.
//	bool HasValue(const std::string& section, const std::string& name) const;
//
//private:
//	int _error;
//	std::map<std::string, std::string> _values;
//	static std::string MakeKey(const std::string& section, const std::string& name);
//	static int ValueHandler(void* user, const char* section, const char* name,
//		const char* value);
//};
//
//#endif  // __INIREADER_H__
//
//#ifndef __INI_H__
//#define __INI_H__
//
///* Make this header file easier to include in C++ code */
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//#include <stdio.h>
//
//	/* Nonzero if ini_handler callback should accept lineno parameter. */
//#ifndef INI_HANDLER_LINENO
//#define INI_HANDLER_LINENO 0
//#endif
//
///* Typedef for prototype of handler function. */
//#if INI_HANDLER_LINENO
//	typedef int(*ini_handler)(void* user, const char* section,
//		const char* name, const char* value,
//		int lineno);
//#else
//	typedef int(*ini_handler)(void* user, const char* section,
//		const char* name, const char* value);
//#endif
//
//	/* Typedef for prototype of fgets-style reader function. */
//	typedef char* (*ini_reader)(char* str, int num, void* stream);
//
//	/* Parse given INI-style file. May have [section]s, name=value pairs
//	   (whitespace stripped), and comments starting with ';' (semicolon). Section
//	   is "" if name=value pair parsed before any section heading. name:value
//	   pairs are also supported as a concession to Python's configparser.
//
//	   For each name=value pair parsed, call handler function with given user
//	   pointer as well as section, name, and value (data only valid for duration
//	   of handler call). Handler should return nonzero on success, zero on error.
//
//	   Returns 0 on success, line number of first error on parse error (doesn't
//	   stop on first error), -1 on file open error, or -2 on memory allocation
//	   error (only when INI_USE_STACK is zero).
//	*/
//	int ini_parse(const char* filename, ini_handler handler, void* user);
//
//	/* Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
//	   close the file when it's finished -- the caller must do that. */
//	int ini_parse_file(FILE* file, ini_handler handler, void* user);
//
//	/* Same as ini_parse(), but takes an ini_reader function pointer instead of
//	   filename. Used for implementing custom or string-based I/O (see also
//	   ini_parse_string). */
//	int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
//		void* user);
//
//	/* Same as ini_parse(), but takes a zero-terminated string with the INI data
//	instead of a file. Useful for parsing INI data from a network socket or
//	already in memory. */
//	int ini_parse_string(const char* string, ini_handler handler, void* user);
//
//	/* Nonzero to allow multi-line value parsing, in the style of Python's
//	   configparser. If allowed, ini_parse() will call the handler with the same
//	   name for each subsequent line parsed. */
//#ifndef INI_ALLOW_MULTILINE
//#define INI_ALLOW_MULTILINE 1
//#endif
//
//	   /* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
//		  the file. See https://github.com/benhoyt/inih/issues/21 */
//#ifndef INI_ALLOW_BOM
//#define INI_ALLOW_BOM 1
//#endif
//
//		  /* Chars that begin a start-of-line comment. Per Python configparser, allow
//			 both ; and # comments at the start of a line by default. */
//#ifndef INI_START_COMMENT_PREFIXES
//#define INI_START_COMMENT_PREFIXES ";#"
//#endif
//
//			 /* Nonzero to allow inline comments (with valid inline comment characters
//				specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
//				Python 3.2+ configparser behaviour. */
//#ifndef INI_ALLOW_INLINE_COMMENTS
//#define INI_ALLOW_INLINE_COMMENTS 1
//#endif
//#ifndef INI_INLINE_COMMENT_PREFIXES
//#define INI_INLINE_COMMENT_PREFIXES ";"
//#endif
//
//				/* Nonzero to use stack for line buffer, zero to use heap (malloc/free). */
//#ifndef INI_USE_STACK
//#define INI_USE_STACK 1
//#endif
//
///* Maximum line length for any line in INI file (stack or heap). Note that
//   this must be 3 more than the longest line (due to '\r', '\n', and '\0'). */
//#ifndef INI_MAX_LINE
//#define INI_MAX_LINE 200
//#endif
//
//   /* Nonzero to allow heap line buffer to grow via realloc(), zero for a
//	  fixed-size buffer of INI_MAX_LINE bytes. Only applies if INI_USE_STACK is
//	  zero. */
//#ifndef INI_ALLOW_REALLOC
//#define INI_ALLOW_REALLOC 0
//#endif
//
//	  /* Initial size in bytes for heap line buffer. Only applies if INI_USE_STACK
//		 is zero. */
//#ifndef INI_INITIAL_ALLOC
//#define INI_INITIAL_ALLOC 200
//#endif
//
//		 /* Stop parsing on first error (default is to keep parsing). */
//#ifndef INI_STOP_ON_FIRST_ERROR
//#define INI_STOP_ON_FIRST_ERROR 0
//#endif
//
///* Nonzero to call the handler at the start of each new section (with
//   name and value NULL). Default is to only call the handler on
//   each name=value pair. */
//#ifndef INI_CALL_HANDLER_ON_NEW_SECTION
//#define INI_CALL_HANDLER_ON_NEW_SECTION 0
//#endif
//
//#ifdef __cplusplus
//}
//#endif
//
//#endif /* __INI_H__ */