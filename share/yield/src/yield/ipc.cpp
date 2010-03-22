#include "yield/ipc.h"
using namespace yield::ipc;


// uriparser.h
/*
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2007, Weijia Song <songweijia@gmail.com>
 * Copyright (C) 2007, Sebastian Pipping <webmaster@hartwork.org>
 * All rights reserved.
 *
 * Redistribution  and use in source and binary forms, with or without
 * modification,  are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions   of  source  code  must  retain  the   above
 *       copyright  notice, this list of conditions and the  following
 *       disclaimer.
 *
 *     * Redistributions  in  binary  form must  reproduce  the  above
 *       copyright  notice, this list of conditions and the  following
 *       disclaimer   in  the  documentation  and/or  other  materials
 *       provided with the distribution.
 *
 *     * Neither  the name of the <ORGANIZATION> nor the names of  its
 *       contributors  may  be  used to endorse  or  promote  products
 *       derived  from  this software without specific  prior  written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT  NOT
 * LIMITED  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS
 * FOR  A  PARTICULAR  PURPOSE ARE DISCLAIMED. IN NO EVENT  SHALL  THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL,    SPECIAL,   EXEMPLARY,   OR   CONSEQUENTIAL   DAMAGES
 * (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES;  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT  LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE  OR  OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file UriDefsConfig.h
 * Adjusts the internal configuration after processing external definitions.
 */

#ifndef URIPARSER_H
#define URIPARSER_H

# include <stdio.h> /* For NULL, snprintf */
# include <ctype.h> /* For wchar_t */
# include <string.h> /* For strlen, memset, memcpy */
# include <stdlib.h> /* For malloc */


/* Header: UriDefsAnsi.h */
#undef URI_CHAR
#define URI_CHAR char

#undef _UT
#define _UT(x) x

#undef URI_FUNC
#define URI_FUNC(x) uri##x##A

#undef URI_TYPE
#define URI_TYPE(x) Uri##x##A



#undef URI_STRLEN
#define URI_STRLEN strlen
#undef URI_STRCPY
#define URI_STRCPY strcpy
#undef URI_STRCMP
#define URI_STRCMP strcmp
#undef URI_STRNCMP
#define URI_STRNCMP strncmp

/* TODO Remove on next source-compatibility break */
#undef URI_SNPRINTF
#if (defined(__WIN32__) || defined(_WIN32) || defined(WIN32))
# define URI_SNPRINTF _snprintf
#else
# define URI_SNPRINTF snprintf
#endif


/* Header: UriBase.h */

/* Version helper macro */
#define URI_ANSI_TO_UNICODE(x) L##x



/* Version */
#define URI_VER_MAJOR           0
#define URI_VER_MINOR           7
#define URI_VER_RELEASE         5
#define URI_VER_SUFFIX_ANSI     ""
#define URI_VER_SUFFIX_UNICODE  URI_ANSI_TO_UNICODE(URI_VER_SUFFIX_ANSI)



/* More version helper macros */
#define URI_INT_TO_ANSI_HELPER(x) #x
#define URI_INT_TO_ANSI(x) URI_INT_TO_ANSI_HELPER(x)

#define URI_INT_TO_UNICODE_HELPER(x) URI_ANSI_TO_UNICODE(#x)
#define URI_INT_TO_UNICODE(x) URI_INT_TO_UNICODE_HELPER(x)

#define URI_VER_ANSI_HELPER(ma, mi, r, s) \
	URI_INT_TO_ANSI(ma) "." \
	URI_INT_TO_ANSI(mi) "." \
	URI_INT_TO_ANSI(r) \
	s

#define URI_VER_UNICODE_HELPER(ma, mi, r, s) \
	URI_INT_TO_UNICODE(ma) L"." \
	URI_INT_TO_UNICODE(mi) L"." \
	URI_INT_TO_UNICODE(r) \
	s



/* Full version strings */
#define URI_VER_ANSI     URI_VER_ANSI_HELPER(URI_VER_MAJOR, URI_VER_MINOR, URI_VER_RELEASE, URI_VER_SUFFIX_ANSI)
#define URI_VER_UNICODE  URI_VER_UNICODE_HELPER(URI_VER_MAJOR, URI_VER_MINOR, URI_VER_RELEASE, URI_VER_SUFFIX_UNICODE)



/* Unused parameter macro */
#ifdef __GNUC__
# define URI_UNUSED(x) unused_##x __attribute__((unused))
#else
# define URI_UNUSED(x) x
#endif



typedef int UriBool; /**< Boolean type */

#define URI_TRUE     1
#define URI_FALSE    0



/* Shared errors */
#define URI_SUCCESS                        0
#define URI_ERROR_SYNTAX                   1 /* Parsed text violates expected format */
#define URI_ERROR_NULL                     2 /* One of the params passed was NULL
                                                although it mustn't be */
#define URI_ERROR_MALLOC                   3 /* Requested memory could not be allocated */
#define URI_ERROR_OUTPUT_TOO_LARGE         4 /* Some output is to large for the receiving buffer */
#define URI_ERROR_NOT_IMPLEMENTED          8 /* The called function is not implemented yet */
#define URI_ERROR_RANGE_INVALID            9 /* The parameters passed contained invalid ranges */


/* Errors specific to ToString */
#define URI_ERROR_TOSTRING_TOO_LONG        URI_ERROR_OUTPUT_TOO_LARGE /* Deprecated, test for URI_ERROR_OUTPUT_TOO_LARGE instead */

/* Errors specific to AddBaseUri */
#define URI_ERROR_ADDBASE_REL_BASE         5 /* Given base is not absolute */

/* Errors specific to RemoveBaseUri */
#define URI_ERROR_REMOVEBASE_REL_BASE      6 /* Given base is not absolute */
#define URI_ERROR_REMOVEBASE_REL_SOURCE    7 /* Given base is not absolute */


/* Function inlining, not ANSI/ISO C! */
#if (defined(URI_DOXYGEN) || defined(URI_SIZEDOWN))
# define URI_INLINE
#elif defined(__INTEL_COMPILER)
/* Intel C/C++ */
/* http://predef.sourceforge.net/precomp.html#sec20 */
/* http://www.intel.com/support/performancetools/c/windows/sb/CS-007751.htm#2 */
# define URI_INLINE __force_inline
#elif defined(_MSC_VER)
/* Microsoft Visual C++ */
/* http://predef.sourceforge.net/precomp.html#sec32 */
/* http://msdn2.microsoft.com/en-us/library/ms882281.aspx */
# define URI_INLINE __forceinline
#elif (__GNUC__ >= 4)
/* GCC C/C++ 4.x.x */
/* http://predef.sourceforge.net/precomp.html#sec13 */
# define URI_INLINE __attribute__((always_inline))
#elif (__STDC_VERSION__ >= 199901L)
/* C99, "inline" is a keyword */
# define URI_INLINE inline
#else
/* No inlining */
# define URI_INLINE
#endif



/* Header: UriBase.h */
/**
 * Holds an IPv4 address.
 */
typedef struct UriIp4Struct {
	unsigned char data[4]; /**< Each octet in one byte */
} UriIp4; /**< @copydoc UriIp4Struct */



/**
 * Holds an IPv6 address.
 */
typedef struct UriIp6Struct {
	unsigned char data[16]; /**< Each quad in two bytes */
} UriIp6; /**< @copydoc UriIp6Struct */



/**
 * Specifies a line break conversion mode
 */
typedef enum UriBreakConversionEnum {
	URI_BR_TO_LF, /**< Convert to Unix line breaks ("\\x0a") */
	URI_BR_TO_CRLF, /**< Convert to Windows line breaks ("\\x0d\\x0a") */
	URI_BR_TO_CR, /**< Convert to Macintosh line breaks ("\\x0d") */
	URI_BR_TO_UNIX = URI_BR_TO_LF, /**< @copydoc UriBreakConversionEnum::URI_BR_TO_LF */
	URI_BR_TO_WINDOWS = URI_BR_TO_CRLF, /**< @copydoc UriBreakConversionEnum::URI_BR_TO_CRLF */
	URI_BR_TO_MAC = URI_BR_TO_CR, /**< @copydoc UriBreakConversionEnum::URI_BR_TO_CR */
	URI_BR_DONT_TOUCH /**< Copy line breaks unmodified */
} UriBreakConversion; /**< @copydoc UriBreakConversionEnum */



/**
 * Specifies which component of a %URI has to be normalized.
 */
typedef enum UriNormalizationMaskEnum {
	URI_NORMALIZED = 0, /**< Do not normalize anything */
	URI_NORMALIZE_SCHEME = 1 << 0, /**< Normalize scheme (fix uppercase letters) */
	URI_NORMALIZE_USER_INFO = 1 << 1, /**< Normalize user info (fix uppercase percent-encodings) */
	URI_NORMALIZE_HOST = 1 << 2, /**< Normalize host (fix uppercase letters) */
	URI_NORMALIZE_PATH = 1 << 3, /**< Normalize path (fix uppercase percent-encodings and redundant dot segments) */
	URI_NORMALIZE_QUERY = 1 << 4, /**< Normalize query (fix uppercase percent-encodings) */
	URI_NORMALIZE_FRAGMENT = 1 << 5 /**< Normalize fragment (fix uppercase percent-encodings) */
} UriNormalizationMask; /**< @copydoc UriNormalizationMaskEnum */


/* Header: Uri.h */
/**
 * Specifies a range of characters within a string.
 * The range includes all characters from <c>first</c>
 * to one before <c>afterLast</c>. So if both are
 * non-NULL the difference is the length of the text range.
 *
 * @see UriUriA
 * @see UriPathSegmentA
 * @see UriHostDataA
 * @since 0.3.0
 */
typedef struct URI_TYPE(TextRangeStruct) {
	const URI_CHAR * first; /**< Pointer to first character */
	const URI_CHAR * afterLast; /**< Pointer to character after the last one still in */
} URI_TYPE(TextRange); /**< @copydoc UriTextRangeStructA */



/**
 * Represents a path segment within a %URI path.
 * More precisely it is a node in a linked
 * list of path segments.
 *
 * @see UriUriA
 * @since 0.3.0
 */
typedef struct URI_TYPE(PathSegmentStruct) {
	URI_TYPE(TextRange) text; /**< Path segment name */
	struct URI_TYPE(PathSegmentStruct) * next; /**< Pointer to the next path segment in the list, can be NULL if last already */

	void * reserved; /**< Reserved to the parser */
} URI_TYPE(PathSegment); /**< @copydoc UriPathSegmentStructA */



/**
 * Holds structured host information.
 * This is either a IPv4, IPv6, plain
 * text for IPvFuture or all zero for
 * a registered name.
 *
 * @see UriUriA
 * @since 0.3.0
 */
typedef struct URI_TYPE(HostDataStruct) {
	UriIp4 * ip4; /**< IPv4 address */
	UriIp6 * ip6; /**< IPv6 address */
	URI_TYPE(TextRange) ipFuture; /**< IPvFuture address */
} URI_TYPE(HostData); /**< @copydoc UriHostDataStructA */



/**
 * Represents an RFC 3986 %URI.
 * Missing components can be {NULL, NULL} ranges.
 *
 * @see uriParseUriA
 * @see uriFreeUriMembersA
 * @see UriParserStateA
 * @since 0.3.0
 */
typedef struct URI_TYPE(UriStruct) {
	URI_TYPE(TextRange) scheme; /**< Scheme (e.g. "http") */
	URI_TYPE(TextRange) userInfo; /**< User info (e.g. "user:pass") */
	URI_TYPE(TextRange) hostText; /**< Host text (set for all hosts, excluding square brackets) */
	URI_TYPE(HostData) hostData; /**< Structured host type specific data */
	URI_TYPE(TextRange) portText; /**< Port (e.g. "80") */
	URI_TYPE(PathSegment) * pathHead; /**< Head of a linked list of path segments */
	URI_TYPE(PathSegment) * pathTail; /**< Tail of the list behind pathHead */
	URI_TYPE(TextRange) query; /**< Query without leading "?" */
	URI_TYPE(TextRange) fragment; /**< Query without leading "#" */
	UriBool absolutePath; /**< Absolute path flag, distincting "a" and "/a" */
	UriBool owner; /**< Memory owner flag */

	void * reserved; /**< Reserved to the parser */
} URI_TYPE(Uri); /**< @copydoc UriUriStructA */



/**
 * Represents a state of the %URI parser.
 * Missing components can be NULL to reflect
 * a components absence.
 *
 * @see uriFreeUriMembersA
 * @since 0.3.0
 */
typedef struct URI_TYPE(ParserStateStruct) {
	URI_TYPE(Uri) * uri; /**< Plug in the %URI structure to be filled while parsing here */
	int errorCode; /**< Code identifying the occured error */
	const URI_CHAR * errorPos; /**< Pointer to position in case of a syntax error */

	void * reserved; /**< Reserved to the parser */
} URI_TYPE(ParserState); /**< @copydoc UriParserStateStructA */



/**
 * Represents a query element.
 * More precisely it is a node in a linked
 * list of query elements.
 *
 * @since 0.7.0
 */
typedef struct URI_TYPE(QueryListStruct) {
	URI_CHAR * key; /**< Key of the query element */
	URI_CHAR * value; /**< Value of the query element, can be NULL */

	struct URI_TYPE(QueryListStruct) * next; /**< Pointer to the next key/value pair in the list, can be NULL if last already */
} URI_TYPE(QueryList); /**< @copydoc UriQueryListStructA */



/**
 * Parses a RFC 3986 URI.
 *
 * @param state       <b>INOUT</b>: Parser state with set output %URI, must not be NULL
 * @param first       <b>IN</b>: Pointer to the first character to parse, must not be NULL
 * @param afterLast   <b>IN</b>: Pointer to the character after the last to parse, must not be NULL
 * @return            0 on success, error code otherwise
 *
 * @see uriParseUriA
 * @see uriToStringA
 * @since 0.3.0
 */
int URI_FUNC(ParseUriEx)(URI_TYPE(ParserState) * state,
		const URI_CHAR * first, const URI_CHAR * afterLast);



/**
 * Parses a RFC 3986 %URI.
 *
 * @param state   <b>INOUT</b>: Parser state with set output %URI, must not be NULL
 * @param text    <b>IN</b>: Text to parse, must not be NULL
 * @return        0 on success, error code otherwise
 *
 * @see uriParseUriExA
 * @see uriToStringA
 * @since 0.3.0
 */
int URI_FUNC(ParseUri)(URI_TYPE(ParserState) * state,
		const URI_CHAR * text);



/**
 * Frees all memory associated with the members
 * of the %URI structure. Note that the structure
 * itself is not freed, only its members.
 *
 * @param uri   <b>INOUT</b>: %URI structure whose members should be freed
 *
 * @since 0.3.0
 */
void URI_FUNC(FreeUriMembers)(URI_TYPE(Uri) * uri);



/**
 * Percent-encodes all unreserved characters from the input string and
 * writes the encoded version to the output string.
 * Be sure to allocate <b>3 times</b> the space of the input buffer for
 * the output buffer for <c>normalizeBreaks == URI_FALSE</c> and <b>6 times</b>
 * the space for <c>normalizeBreaks == URI_TRUE</c>
 * (since e.g. "\x0d" becomes "%0D%0A" in that case)
 *
 * @param inFirst           <b>IN</b>: Pointer to first character of the input text
 * @param inAfterLast       <b>IN</b>: Pointer after the last character of the input text
 * @param out               <b>OUT</b>: Encoded text destination
 * @param spaceToPlus       <b>IN</b>: Wether to convert ' ' to '+' or not
 * @param normalizeBreaks   <b>IN</b>: Wether to convert CR and LF to CR-LF or not.
 * @return                  Position of terminator in output string
 *
 * @see uriEscapeA
 * @see uriUnescapeInPlaceExA
 * @since 0.5.2
 */
URI_CHAR * URI_FUNC(EscapeEx)(const URI_CHAR * inFirst,
		const URI_CHAR * inAfterLast, URI_CHAR * out,
		UriBool spaceToPlus, UriBool normalizeBreaks);



/**
 * Percent-encodes all unreserved characters from the input string and
 * writes the encoded version to the output string.
 * Be sure to allocate <b>3 times</b> the space of the input buffer for
 * the output buffer for <c>normalizeBreaks == URI_FALSE</c> and <b>6 times</b>
 * the space for <c>normalizeBreaks == URI_FALSE</c>
 * (since e.g. "\x0d" becomes "%0D%0A" in that case)
 *
 * @param in                <b>IN</b>: Text source
 * @param out               <b>OUT</b>: Encoded text destination
 * @param spaceToPlus       <b>IN</b>: Wether to convert ' ' to '+' or not
 * @param normalizeBreaks   <b>IN</b>: Wether to convert CR and LF to CR-LF or not.
 * @return                  Position of terminator in output string
 *
 * @see uriEscapeExA
 * @see uriUnescapeInPlaceA
 * @since 0.5.0
 */
URI_CHAR * URI_FUNC(Escape)(const URI_CHAR * in, URI_CHAR * out,
		UriBool spaceToPlus, UriBool normalizeBreaks);



/**
 * Unescapes percent-encoded groups in a given string.
 * E.g. "%20" will become " ". Unescaping is done in place.
 * The return value will be point to the new position
 * of the terminating zero. Use this value to get the new
 * length of the string. NULL is only returned if <c>inout</c>
 * is NULL.
 *
 * @param inout             <b>INOUT</b>: Text to unescape/decode
 * @param plusToSpace       <b>IN</b>: Whether to convert '+' to ' ' or not
 * @param breakConversion   <b>IN</b>: Line break conversion mode
 * @return                  Pointer to new position of the terminating zero
 *
 * @see uriUnescapeInPlaceA
 * @see uriEscapeExA
 * @since 0.5.0
 */
const URI_CHAR * URI_FUNC(UnescapeInPlaceEx)(URI_CHAR * inout,
		UriBool plusToSpace, UriBreakConversion breakConversion);



/**
 * Unescapes percent-encoded groups in a given string.
 * E.g. "%20" will become " ". Unescaping is done in place.
 * The return value will be point to the new position
 * of the terminating zero. Use this value to get the new
 * length of the string. NULL is only returned if <c>inout</c>
 * is NULL.
 *
 * NOTE: '+' is not decoded to ' ' and line breaks are not converted.
 * Use the more advanced UnescapeInPlaceEx for that features instead.
 *
 * @param inout   <b>INOUT</b>: Text to unescape/decode
 * @return        Pointer to new position of the terminating zero
 *
 * @see uriUnescapeInPlaceExA
 * @see uriEscapeA
 * @since 0.3.0
 */
const URI_CHAR * URI_FUNC(UnescapeInPlace)(URI_CHAR * inout);



/**
 * Performs reference resolution as described in
 * <a href="http://tools.ietf.org/html/rfc3986#section-5.2.2">section 5.2.2 of RFC 3986</a>.
 * NOTE: On success you have to call uriFreeUriMembersA on \p absoluteDest manually later.
 *
 * @param absoluteDest     <b>OUT</b>: Result %URI
 * @param relativeSource   <b>IN</b>: Reference to resolve
 * @param absoluteBase     <b>IN</b>: Base %URI to apply
 * @return                 Error code or 0 on success
 *
 * @see uriRemoveBaseUriA
 * @since 0.4.0
 */
int URI_FUNC(AddBaseUri)(URI_TYPE(Uri) * absoluteDest,
		const URI_TYPE(Uri) * relativeSource,
		const URI_TYPE(Uri) * absoluteBase);



/**
 * Tries to make a relative %URI (a reference) from an
 * absolute %URI and a given base %URI. This can only work if
 * the absolute %URI shares scheme and authority with
 * the base %URI. If it does not the result will still be
 * an absolute URI (with scheme part if necessary).
 * NOTE: On success you have to call uriFreeUriMembersA on
 * \p dest manually later.
 *
 * @param dest             <b>OUT</b>: Result %URI
 * @param absoluteSource   <b>IN</b>: Absolute %URI to make relative
 * @param absoluteBase     <b>IN</b>: Base %URI
 * @param domainRootMode   <b>IN</b>: Create %URI with path relative to domain root
 * @return                 Error code or 0 on success
 *
 * @see uriAddBaseUriA
 * @since 0.5.2
 */
int URI_FUNC(RemoveBaseUri)(URI_TYPE(Uri) * dest,
		const URI_TYPE(Uri) * absoluteSource,
		const URI_TYPE(Uri) * absoluteBase,
		UriBool domainRootMode);



/**
 * Checks two URIs for equivalence. Comparison is done
 * the naive way, without prior normalization.
 * NOTE: Two <c>NULL</c> URIs are equal as well.
 *
 * @param a   <b>IN</b>: First %URI
 * @param b   <b>IN</b>: Second %URI
 * @return    <c>URI_TRUE</c> when equal, <c>URI_FAlSE</c> else
 *
 * @since 0.4.0
 */
UriBool URI_FUNC(EqualsUri)(const URI_TYPE(Uri) * a, const URI_TYPE(Uri) * b);



/**
 * Calculates the number of characters needed to store the
 * string representation of the given %URI excluding the
 * terminator.
 *
 * @param uri             <b>IN</b>: %URI to measure
 * @param charsRequired   <b>OUT</b>: Length of the string representation in characters <b>excluding</b> terminator
 * @return                Error code or 0 on success
 *
 * @see uriToStringA
 * @since 0.5.0
 */
int URI_FUNC(ToStringCharsRequired)(const URI_TYPE(Uri) * uri,
		int * charsRequired);



/**
 * Converts a %URI structure back to text as described in
 * <a href="http://tools.ietf.org/html/rfc3986#section-5.3">section 5.3 of RFC 3986</a>.
 *
 * @param dest           <b>OUT</b>: Output destination
 * @param uri            <b>IN</b>: %URI to convert
 * @param maxChars       <b>IN</b>: Maximum number of characters to copy <b>including</b> terminator
 * @param charsWritten   <b>OUT</b>: Number of characters written, can be lower than maxChars even if the %URI is too long!
 * @return               Error code or 0 on success
 *
 * @see uriToStringCharsRequiredA
 * @since 0.4.0
 */
int URI_FUNC(ToString)(URI_CHAR * dest, const URI_TYPE(Uri) * uri, int maxChars, int * charsWritten);



/**
 * Determines the components of a %URI that are not normalized.
 *
 * @param uri   <b>IN</b>: %URI to check
 * @return      Normalization job mask
 *
 * @see uriNormalizeSyntaxA
 * @since 0.5.0
 */
unsigned int URI_FUNC(NormalizeSyntaxMaskRequired)(const URI_TYPE(Uri) * uri);



/**
 * Normalizes a %URI using a normalization mask.
 * The normalization mask decides what components are normalized.
 *
 * NOTE: If necessary the %URI becomes owner of all memory
 * behind the text pointed to. Text is duplicated in that case.
 *
 * @param uri    <b>INOUT</b>: %URI to normalize
 * @param mask   <b>IN</b>: Normalization mask
 * @return       Error code or 0 on success
 *
 * @see uriNormalizeSyntaxA
 * @see uriNormalizeSyntaxMaskRequiredA
 * @since 0.5.0
 */
int URI_FUNC(NormalizeSyntaxEx)(URI_TYPE(Uri) * uri, unsigned int mask);



/**
 * Normalizes all components of a %URI.
 *
 * NOTE: If necessary the %URI becomes owner of all memory
 * behind the text pointed to. Text is duplicated in that case.
 *
 * @param uri   <b>INOUT</b>: %URI to normalize
 * @return      Error code or 0 on success
 *
 * @see uriNormalizeSyntaxExA
 * @see uriNormalizeSyntaxMaskRequiredA
 * @since 0.5.0
 */
int URI_FUNC(NormalizeSyntax)(URI_TYPE(Uri) * uri);



/**
 * Converts a Unix filename to a %URI string.
 * The destination buffer must be large enough to hold 7 + 3 * len(filename) + 1
 * characters in case of an absolute filename or 3 * len(filename) + 1 in case
 * of a relative filename.
 *
 * EXAMPLE
 *   Input:  "/bin/bash"
 *   Output: "file:///bin/bash"
 *
 * @param filename     <b>IN</b>: Unix filename to convert
 * @param uriString    <b>OUT</b>: Destination to write %URI string to
 * @return             Error code or 0 on success
 *
 * @see uriUriStringToUnixFilenameA
 * @see uriWindowsFilenameToUriStringA
 * @since 0.5.2
 */
int URI_FUNC(UnixFilenameToUriString)(const URI_CHAR * filename,
		URI_CHAR * uriString);



/**
 * Converts a Windows filename to a %URI string.
 * The destination buffer must be large enough to hold 8 + 3 * len(filename) + 1
 * characters in case of an absolute filename or 3 * len(filename) + 1 in case
 * of a relative filename.
 *
 * EXAMPLE
 *   Input:  "E:\\Documents and Settings"
 *   Output: "file:///E:/Documents%20and%20Settings"
 *
 * @param filename     <b>IN</b>: Windows filename to convert
 * @param uriString    <b>OUT</b>: Destination to write %URI string to
 * @return             Error code or 0 on success
 *
 * @see uriUriStringToWindowsFilenameA
 * @see uriUnixFilenameToUriStringA
 * @since 0.5.2
 */
int URI_FUNC(WindowsFilenameToUriString)(const URI_CHAR * filename,
		URI_CHAR * uriString);



/**
 * Extracts a Unix filename from a %URI string.
 * The destination buffer must be large enough to hold len(uriString) + 1 - 7
 * characters in case of an absolute %URI or len(uriString) + 1 in case
 * of a relative %URI.
 *
 * @param uriString    <b>IN</b>: %URI string to convert
 * @param filename     <b>OUT</b>: Destination to write filename to
 * @return             Error code or 0 on success
 *
 * @see uriUnixFilenameToUriStringA
 * @see uriUriStringToWindowsFilenameA
 * @since 0.5.2
 */
int URI_FUNC(UriStringToUnixFilename)(const URI_CHAR * uriString,
		URI_CHAR * filename);



/**
 * Extracts a Windows filename from a %URI string.
 * The destination buffer must be large enough to hold len(uriString) + 1 - 8
 * characters in case of an absolute %URI or len(uriString) + 1 in case
 * of a relative %URI.
 *
 * @param uriString    <b>IN</b>: %URI string to convert
 * @param filename     <b>OUT</b>: Destination to write filename to
 * @return             Error code or 0 on success
 *
 * @see uriWindowsFilenameToUriStringA
 * @see uriUriStringToUnixFilenameA
 * @since 0.5.2
 */
int URI_FUNC(UriStringToWindowsFilename)(const URI_CHAR * uriString,
		URI_CHAR * filename);



/**
 * Calculates the number of characters needed to store the
 * string representation of the given query list excluding the
 * terminator. It is assumed that line breaks are will be
 * normalized to "%0D%0A".
 *
 * @param queryList         <b>IN</b>: Query list to measure
 * @param charsRequired     <b>OUT</b>: Length of the string representation in characters <b>excluding</b> terminator
 * @return                  Error code or 0 on success
 *
 * @see uriComposeQueryCharsRequiredExA
 * @see uriComposeQueryA
 * @since 0.7.0
 */
int URI_FUNC(ComposeQueryCharsRequired)(const URI_TYPE(QueryList) * queryList,
		int * charsRequired);



/**
 * Calculates the number of characters needed to store the
 * string representation of the given query list excluding the
 * terminator.
 *
 * @param queryList         <b>IN</b>: Query list to measure
 * @param charsRequired     <b>OUT</b>: Length of the string representation in characters <b>excluding</b> terminator
 * @param spaceToPlus       <b>IN</b>: Wether to convert ' ' to '+' or not
 * @param normalizeBreaks   <b>IN</b>: Wether to convert CR and LF to CR-LF or not.
 * @return                  Error code or 0 on success
 *
 * @see uriComposeQueryCharsRequiredA
 * @see uriComposeQueryExA
 * @since 0.7.0
 */
int URI_FUNC(ComposeQueryCharsRequiredEx)(const URI_TYPE(QueryList) * queryList,
		int * charsRequired, UriBool spaceToPlus, UriBool normalizeBreaks);



/**
 * Converts a query list structure back to a query string.
 * The composed string does not start with '?',
 * on the way ' ' is converted to '+' and line breaks are
 * normalized to "%0D%0A".
 *
 * @param dest              <b>OUT</b>: Output destination
 * @param queryList         <b>IN</b>: Query list to convert
 * @param maxChars          <b>IN</b>: Maximum number of characters to copy <b>including</b> terminator
 * @param charsWritten      <b>OUT</b>: Number of characters written, can be lower than maxChars even if the query list is too long!
 * @return                  Error code or 0 on success
 *
 * @see uriComposeQueryExA
 * @see uriComposeQueryMallocA
 * @see uriComposeQueryCharsRequiredA
 * @see uriDissectQueryMallocA
 * @since 0.7.0
 */
int URI_FUNC(ComposeQuery)(URI_CHAR * dest,
		const URI_TYPE(QueryList) * queryList, int maxChars, int * charsWritten);



/**
 * Converts a query list structure back to a query string.
 * The composed string does not start with '?'.
 *
 * @param dest              <b>OUT</b>: Output destination
 * @param queryList         <b>IN</b>: Query list to convert
 * @param maxChars          <b>IN</b>: Maximum number of characters to copy <b>including</b> terminator
 * @param charsWritten      <b>OUT</b>: Number of characters written, can be lower than maxChars even if the query list is too long!
 * @param spaceToPlus       <b>IN</b>: Wether to convert ' ' to '+' or not
 * @param normalizeBreaks   <b>IN</b>: Wether to convert CR and LF to CR-LF or not.
 * @return                  Error code or 0 on success
 *
 * @see uriComposeQueryA
 * @see uriComposeQueryMallocExA
 * @see uriComposeQueryCharsRequiredExA
 * @see uriDissectQueryMallocExA
 * @since 0.7.0
 */
int URI_FUNC(ComposeQueryEx)(URI_CHAR * dest,
		const URI_TYPE(QueryList) * queryList, int maxChars, int * charsWritten,
		UriBool spaceToPlus, UriBool normalizeBreaks);



/**
 * Converts a query list structure back to a query string.
 * Memory for this string is allocated internally.
 * The composed string does not start with '?',
 * on the way ' ' is converted to '+' and line breaks are
 * normalized to "%0D%0A".
 *
 * @param dest              <b>OUT</b>: Output destination
 * @param queryList         <b>IN</b>: Query list to convert
 * @return                  Error code or 0 on success
 *
 * @see uriComposeQueryMallocExA
 * @see uriComposeQueryA
 * @see uriDissectQueryMallocA
 * @since 0.7.0
 */
int URI_FUNC(ComposeQueryMalloc)(URI_CHAR ** dest,
		const URI_TYPE(QueryList) * queryList);



/**
 * Converts a query list structure back to a query string.
 * Memory for this string is allocated internally.
 * The composed string does not start with '?'.
 *
 * @param dest              <b>OUT</b>: Output destination
 * @param queryList         <b>IN</b>: Query list to convert
 * @param spaceToPlus       <b>IN</b>: Wether to convert ' ' to '+' or not
 * @param normalizeBreaks   <b>IN</b>: Wether to convert CR and LF to CR-LF or not.
 * @return                  Error code or 0 on success
 *
 * @see uriComposeQueryMallocA
 * @see uriComposeQueryExA
 * @see uriDissectQueryMallocExA
 * @since 0.7.0
 */
int URI_FUNC(ComposeQueryMallocEx)(URI_CHAR ** dest,
		const URI_TYPE(QueryList) * queryList,
		UriBool spaceToPlus, UriBool normalizeBreaks);



/**
 * Constructs a query list from the raw query string of a given URI.
 * On the way '+' is converted back to ' ', line breaks are not modified.
 *
 * @param dest              <b>OUT</b>: Output destination
 * @param itemCount         <b>OUT</b>: Number of items found, can be NULL
 * @param first             <b>IN</b>: Pointer to first character <b>after</b> '?'
 * @param afterLast         <b>IN</b>: Pointer to character after the last one still in
 * @return                  Error code or 0 on success
 *
 * @see uriDissectQueryMallocExA
 * @see uriComposeQueryA
 * @see uriFreeQueryListA
 * @since 0.7.0
 */
int URI_FUNC(DissectQueryMalloc)(URI_TYPE(QueryList) ** dest, int * itemCount,
		const URI_CHAR * first, const URI_CHAR * afterLast);



/**
 * Constructs a query list from the raw query string of a given URI.
 *
 * @param dest              <b>OUT</b>: Output destination
 * @param itemCount         <b>OUT</b>: Number of items found, can be NULL
 * @param first             <b>IN</b>: Pointer to first character <b>after</b> '?'
 * @param afterLast         <b>IN</b>: Pointer to character after the last one still in
 * @param plusToSpace       <b>IN</b>: Whether to convert '+' to ' ' or not
 * @param breakConversion   <b>IN</b>: Line break conversion mode
 * @return                  Error code or 0 on success
 *
 * @see uriDissectQueryMallocA
 * @see uriComposeQueryExA
 * @see uriFreeQueryListA
 * @since 0.7.0
 */
int URI_FUNC(DissectQueryMallocEx)(URI_TYPE(QueryList) ** dest, int * itemCount,
		const URI_CHAR * first, const URI_CHAR * afterLast,
		UriBool plusToSpace, UriBreakConversion breakConversion);



/**
 * Frees all memory associated with the given query list.
 * The structure itself is freed as well.
 *
 * @param queryList   <b>INOUT</b>: Query list to free
 *
 * @since 0.7.0
 */
void URI_FUNC(FreeQueryList)(URI_TYPE(QueryList) * queryList);


/* Header: UriIp4.h */
/**
 * Converts a IPv4 text representation into four bytes.
 *
 * @param octetOutput  Output destination
 * @param first        First character of IPv4 text to parse
 * @param afterLast    Position to stop parsing at
 * @return Error code or 0 on success
 */
int URI_FUNC(ParseIpFourAddress)(unsigned char * octetOutput,
		const URI_CHAR * first, const URI_CHAR * afterLast);

#endif


// yajl.h
/*
 * Copyright 2007, Lloyd Hilaiel.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the name of Lloyd Hilaiel nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef YAJL_H
#define YAJL_H


/* Header: yajl_common.h */
#define YAJL_MAX_DEPTH 128

/* msft dll export gunk.  To build a DLL on windows, you
 * must define WIN32, YAJL_SHARED, and YAJL_BUILD.  To use a shared
 * DLL, you must define YAJL_SHARED and WIN32 */
#if defined(WIN32) && defined(YAJL_SHARED)
#  ifdef YAJL_BUILD
#    define YAJL_API __declspec(dllexport)
#  else
#    define YAJL_API __declspec(dllimport)
#  endif
#else
#  define YAJL_API
#endif


/* Header: yajl_buf.h */
/**
 * yajl_buf is a buffer with exponential growth.  the buffer ensures that
 * you are always null padded.
 */
typedef struct yajl_buf_t * yajl_buf;

/* allocate a new buffer */
yajl_buf yajl_buf_alloc(void);

/* free the buffer */
void yajl_buf_free(yajl_buf buf);

/* append a number of bytes to the buffer */
void yajl_buf_append(yajl_buf buf, const void * data, unsigned int len);

/* empty the buffer */
void yajl_buf_clear(yajl_buf buf);

/* get a pointer to the beginning of the buffer */
const unsigned char * yajl_buf_data(yajl_buf buf);

/* get the length of the buffer */
unsigned int yajl_buf_len(yajl_buf buf);

/* truncate the buffer */
void yajl_buf_truncate(yajl_buf buf, unsigned int len);


/* Header: yajl_encode.h */
void yajl_string_encode(yajl_buf buf, const unsigned char * str,
                        unsigned int length);

void yajl_string_decode(yajl_buf buf, const unsigned char * str,
                        unsigned int length);


/* yajl_gen.h */
    /** generator status codes */
    typedef enum {
        /** no error */
        yajl_gen_status_ok = 0,
        /** at a point where a map key is generated, a function other than
         *  yajl_gen_string was called */
        yajl_gen_keys_must_be_strings,
        /** YAJL's maximum generation depth was exceeded.  see
         *  YAJL_MAX_DEPTH */
        yajl_max_depth_exceeded,
        /** A generator function (yajl_gen_XXX) was called while in an error
         *  state */
        yajl_gen_in_error_state,
        /** A complete JSON document has been generated */
        yajl_gen_generation_complete
    } yajl_gen_status;

    /** an opaque handle to a generator */
    typedef struct yajl_gen_t * yajl_gen;

    /** configuration structure for the generator */
    typedef struct {
        /** generate indented (beautiful) output */
        unsigned int beautify;
        /** an opportunity to define an indent string.  such as \\t or
         *  some number of spaces.  default is four spaces '    '.  This
         *  member is only relevant when beautify is true */
        const char * indentString;
    } yajl_gen_config;

    /** allocate a generator handle */
    yajl_gen YAJL_API yajl_gen_alloc(const yajl_gen_config * config);

    /** free a generator handle */
    void YAJL_API yajl_gen_free(yajl_gen handle);

    yajl_gen_status YAJL_API yajl_gen_integer(yajl_gen hand, long int number);
    yajl_gen_status YAJL_API yajl_gen_double(yajl_gen hand, double number);
    yajl_gen_status YAJL_API yajl_gen_number(yajl_gen hand,
                                             const char * num,
                                             unsigned int len);
    yajl_gen_status YAJL_API yajl_gen_string(yajl_gen hand,
                                             const unsigned char * str,
                                             unsigned int len);
    yajl_gen_status YAJL_API yajl_gen_null(yajl_gen hand);
    yajl_gen_status YAJL_API yajl_gen_bool(yajl_gen hand, int boolean);
    yajl_gen_status YAJL_API yajl_gen_map_open(yajl_gen hand);
    yajl_gen_status YAJL_API yajl_gen_map_close(yajl_gen hand);
    yajl_gen_status YAJL_API yajl_gen_array_open(yajl_gen hand);
    yajl_gen_status YAJL_API yajl_gen_array_close(yajl_gen hand);

    /** access the null terminated generator buffer.  If incrementally
     *  outputing JSON, one should call yajl_gen_clear to clear the
     *  buffer.  This allows stream generation. */
    yajl_gen_status YAJL_API yajl_gen_get_buf(yajl_gen hand,
                                              const unsigned char ** buf,
                                              unsigned int * len);

    /** clear yajl's output buffer, but maintain all internal generation
     *  state.  This function will not "reset" the generator state, and is
     *  intended to enable incremental JSON outputing. */
    void YAJL_API yajl_gen_clear(yajl_gen hand);


/* yajl_lex.h */
    typedef enum {
    yajl_tok_bool,
    yajl_tok_colon,
    yajl_tok_comma,
    yajl_tok_eof,
    yajl_tok_error,
    yajl_tok_left_brace,
    yajl_tok_left_bracket,
    yajl_tok_null,
    yajl_tok_right_brace,
    yajl_tok_right_bracket,

    /* we differentiate between integers and doubles to allow the
     * parser to interpret the number without re-scanning */
    yajl_tok_integer,
    yajl_tok_double,

    /* we differentiate between strings which require further processing,
     * and strings that do not */
    yajl_tok_string,
    yajl_tok_string_with_escapes,

    /* comment tokens are not currently returned to the parser, ever */
    yajl_tok_comment
} yajl_tok;

typedef struct yajl_lexer_t * yajl_lexer;

yajl_lexer yajl_lex_alloc(unsigned int allowComments,
                          unsigned int validateUTF8);

void yajl_lex_free(yajl_lexer lexer);

/**
 * run/continue a lex. "offset" is an input/output parameter.
 * It should be initialized to zero for a
 * new chunk of target text, and upon subsetquent calls with the same
 * target text should passed with the value of the previous invocation.
 *
 * the client may be interested in the value of offset when an error is
 * returned from the lexer.  This allows the client to render useful
n * error messages.
 *
 * When you pass the next chunk of data, context should be reinitialized
 * to zero.
 *
 * Finally, the output buffer is usually just a pointer into the jsonText,
 * however in cases where the entity being lexed spans multiple chunks,
 * the lexer will buffer the entity and the data returned will be
 * a pointer into that buffer.
 *
 * This behavior is abstracted from client code except for the performance
 * implications which require that the client choose a reasonable chunk
 * size to get adequate performance.
 */
yajl_tok yajl_lex_lex(yajl_lexer lexer, const unsigned char * jsonText,
                      unsigned int jsonTextLen, unsigned int * offset,
                      const unsigned char ** outBuf, unsigned int * outLen);

/** have a peek at the next token, but don't move the lexer forward */
yajl_tok yajl_lex_peek(yajl_lexer lexer, const unsigned char * jsonText,
                       unsigned int jsonTextLen, unsigned int offset);


typedef enum {
    yajl_lex_e_ok = 0,
    yajl_lex_string_invalid_utf8,
    yajl_lex_string_invalid_escaped_char,
    yajl_lex_string_invalid_json_char,
    yajl_lex_string_invalid_hex_char,
    yajl_lex_invalid_char,
    yajl_lex_invalid_string,
    yajl_lex_missing_integer_after_decimal,
    yajl_lex_missing_integer_after_exponent,
    yajl_lex_missing_integer_after_minus,
    yajl_lex_unallowed_comment
} yajl_lex_error;

const char * yajl_lex_error_to_string(yajl_lex_error error);

/** allows access to more specific information about the lexical
 *  error when yajl_lex_lex returns yajl_tok_error. */
yajl_lex_error yajl_lex_get_error(yajl_lexer lexer);

/** get the current offset into the most recently lexed json string. */
unsigned int yajl_lex_current_offset(yajl_lexer lexer);

/** get the number of lines lexed by this lexer instance */
unsigned int yajl_lex_current_line(yajl_lexer lexer);

/** get the number of chars lexed by this lexer instance since the last
 *  \n or \r */
unsigned int yajl_lex_current_char(yajl_lexer lexer);


/* yajl_parse.h */
    /** error codes returned from this interface */
    typedef enum {
        /** no error was encountered */
        yajl_status_ok,
        /** a client callback returned zero, stopping the parse */
        yajl_status_client_canceled,
        /** The parse cannot yet complete because more json input text
         *  is required, call yajl_parse with the next buffer of input text.
         *  (pertinent only when stream parsing) */
        yajl_status_insufficient_data,
        /** An error occured during the parse.  Call yajl_get_error for
         *  more information about the encountered error */
        yajl_status_error
    } yajl_status;

    /** attain a human readable, english, string for an error */
    const char * YAJL_API yajl_status_to_string(yajl_status code);

    /** an opaque handle to a parser */
    typedef struct yajl_handle_t * yajl_handle;

    /** yajl is an event driven parser.  this means as json elements are
     *  parsed, you are called back to do something with the data.  The
     *  functions in this table indicate the various events for which
     *  you will be called back.  Each callback accepts a "context"
     *  pointer, this is a void * that is passed into the yajl_parse
     *  function which the client code may use to pass around context.
     *
     *  All callbacks return an integer.  If non-zero, the parse will
     *  continue.  If zero, the parse will be canceled and
     *  yajl_status_client_canceled will be returned from the parse.
     *
     *  Note about handling of numbers:
     *    yajl will only convert numbers that can be represented in a double
     *    or a long int.  All other numbers will be passed to the client
     *    in string form using the yajl_number callback.  Furthermore, if
     *    yajl_number is not NULL, it will always be used to return numbers,
     *    that is yajl_integer and yajl_double will be ignored.  If
     *    yajl_number is NULL but one of yajl_integer or yajl_double are
     *    defined, parsing of a number larger than is representable
     *    in a double or long int will result in a parse error.
     */
    typedef struct {
        int (* yajl_null)(void * ctx);
        int (* yajl_boolean)(void * ctx, int boolVal);
        int (* yajl_integer)(void * ctx, long integerVal);
        int (* yajl_double)(void * ctx, double doubleVal);
        /** A callback which passes the string representation of the number
         *  back to the client.  Will be used for all numbers when present */
        int (* yajl_number)(void * ctx, const char * numberVal,
                            unsigned int numberLen);

        /** strings are returned as pointers into the JSON text when,
         * possible, as a result, they are _not_ null padded */
        int (* yajl_string)(void * ctx, const unsigned char * stringVal,
                            unsigned int stringLen);

        int (* yajl_start_map)(void * ctx);
        int (* yajl_map_key)(void * ctx, const unsigned char * key,
                             unsigned int stringLen);
        int (* yajl_end_map)(void * ctx);

        int (* yajl_start_array)(void * ctx);
        int (* yajl_end_array)(void * ctx);
    } yajl_callbacks;

    /** configuration structure for the generator */
    typedef struct {
        /** if nonzero, javascript style comments will be allowed in
         *  the json input, both slash star and slash slash */
        unsigned int allowComments;
        /** if nonzero, invalid UTF8 strings will cause a parse
         *  error */
        unsigned int checkUTF8;
    } yajl_parser_config;

    /** allocate a parser handle
     *  \param callbacks  a yajl callbacks structure specifying the
     *                    functions to call when different JSON entities
     *                    are encountered in the input text.  May be NULL,
     *                    which is only useful for validation.
     *  \param config     configuration parameters for the parse.
     *  \param ctx        a context pointer that will be passed to callbacks.
     */
    yajl_handle YAJL_API yajl_alloc(const yajl_callbacks * callbacks,
                                    const yajl_parser_config * config,
                                    void * ctx);

    /** free a parser handle */
    void YAJL_API yajl_free(yajl_handle handle);

    /** Parse some json!
     *  \param hand - a handle to the json parser allocated with yajl_alloc
     *  \param jsonText - a pointer to the UTF8 json text to be parsed
     *  \param jsonTextLength - the length, in bytes, of input text
     */
    yajl_status YAJL_API yajl_parse(yajl_handle hand,
                                    const unsigned char * jsonText,
                                    unsigned int jsonTextLength);

    /** get an error string describing the state of the
     *  parse.
     *
     *  If verbose is non-zero, the message will include the JSON
     *  text where the error occured, along with an arrow pointing to
     *  the specific char.
     *
     *  A dynamically allocated string will be returned which should
     *  be freed with yajl_free_error
     */
    unsigned char * YAJL_API yajl_get_error(yajl_handle hand, int verbose,
                                            const unsigned char * jsonText,
                                            unsigned int jsonTextLength);

    /** free an error returned from yajl_get_error */
    void YAJL_API yajl_free_error(unsigned char * str);


/* yajl_parser.h */
    typedef enum {
    yajl_state_start = 0,
    yajl_state_parse_complete,
    yajl_state_parse_error,
    yajl_state_lexical_error,
    yajl_state_map_start,
    yajl_state_map_sep,
    yajl_state_map_need_val,
    yajl_state_map_got_val,
    yajl_state_map_need_key,
    yajl_state_array_start,
    yajl_state_array_got_val,
    yajl_state_array_need_val
} yajl_state;

struct yajl_handle_t {
    const yajl_callbacks * callbacks;
    void * ctx;
    yajl_lexer lexer;
    const char * parseError;
    unsigned int errorOffset;
    /* temporary storage for decoded strings */
    yajl_buf decodeBuf;
    /* a stack of states.  access with yajl_state_XXX routines */
    yajl_buf stateBuf;
};

yajl_status
yajl_do_parse(yajl_handle handle, unsigned int * offset,
              const unsigned char * jsonText, unsigned int jsonTextLen);

unsigned char *
yajl_render_error_string(yajl_handle hand, const unsigned char * jsonText,
                         unsigned int jsonTextLen, int verbose);

yajl_state yajl_state_current(yajl_handle handle);

void yajl_state_push(yajl_handle handle, yajl_state state);

yajl_state yajl_state_pop(yajl_handle handle);

unsigned int yajl_parse_depth(yajl_handle handle);

void yajl_state_set(yajl_handle handle, yajl_state state);


#endif


// uriparser.c
/*
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2007, Weijia Song <songweijia@gmail.com>
 * Copyright (C) 2007, Sebastian Pipping <webmaster@hartwork.org>
 * All rights reserved.
 *
 * Redistribution  and use in source and binary forms, with or without
 * modification,  are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions   of  source  code  must  retain  the   above
 *       copyright  notice, this list of conditions and the  following
 *       disclaimer.
 *
 *     * Redistributions  in  binary  form must  reproduce  the  above
 *       copyright  notice, this list of conditions and the  following
 *       disclaimer   in  the  documentation  and/or  other  materials
 *       provided with the distribution.
 *
 *     * Neither  the name of the <ORGANIZATION> nor the names of  its
 *       contributors  may  be  used to endorse  or  promote  products
 *       derived  from  this software without specific  prior  written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT  NOT
 * LIMITED  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS
 * FOR  A  PARTICULAR  PURPOSE ARE DISCLAIMED. IN NO EVENT  SHALL  THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL,    SPECIAL,   EXEMPLARY,   OR   CONSEQUENTIAL   DAMAGES
 * (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES;  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT  LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE  OR  OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */



/* Prototypes: UriCommon.h */
/* Used to point to from empty path segments.
 * X.first and X.afterLast must be the same non-NULL value then. */
extern const URI_CHAR * const URI_FUNC(SafeToPointTo);
extern const URI_CHAR * const URI_FUNC(ConstPwd);
extern const URI_CHAR * const URI_FUNC(ConstParent);



void URI_FUNC(ResetUri)(URI_TYPE(Uri) * uri);

UriBool URI_FUNC(RemoveDotSegmentsAbsolute)(URI_TYPE(Uri) * uri);
UriBool URI_FUNC(RemoveDotSegments)(URI_TYPE(Uri) * uri, UriBool relative);
UriBool URI_FUNC(RemoveDotSegmentsEx)(URI_TYPE(Uri) * uri,
        UriBool relative, UriBool pathOwned);

unsigned char URI_FUNC(HexdigToInt)(URI_CHAR hexdig);
URI_CHAR URI_FUNC(HexToLetter)(unsigned int value);
URI_CHAR URI_FUNC(HexToLetterEx)(unsigned int value, UriBool uppercase);

UriBool URI_FUNC(IsHostSet)(const URI_TYPE(Uri) * uri);

UriBool URI_FUNC(CopyPath)(URI_TYPE(Uri) * dest, const URI_TYPE(Uri) * source);
UriBool URI_FUNC(CopyAuthority)(URI_TYPE(Uri) * dest, const URI_TYPE(Uri) * source);

UriBool URI_FUNC(FixAmbiguity)(URI_TYPE(Uri) * uri);
void URI_FUNC(FixEmptyTrailSegment)(URI_TYPE(Uri) * uri);

/* Prototypes: UriCommon.c */
/*extern*/ const URI_CHAR * const URI_FUNC(SafeToPointTo) = _UT("X");
/*extern*/ const URI_CHAR * const URI_FUNC(ConstPwd) = _UT(".");
/*extern*/ const URI_CHAR * const URI_FUNC(ConstParent) = _UT("..");


/* Prototypes: UriCompare.c */
static int URI_FUNC(CompareRange)(const URI_TYPE(TextRange) * a,
    const URI_TYPE(TextRange) * b);


/* Prototypes: UriIpBase.h */
typedef struct UriIp4ParserStruct {
  unsigned char stackCount;
  unsigned char stackOne;
  unsigned char stackTwo;
  unsigned char stackThree;
} UriIp4Parser;



void uriPushToStack(UriIp4Parser * parser, unsigned char digit);
void uriStackToOctet(UriIp4Parser * parser, unsigned char * octet);

/* Prototypes: UriIp4.c */
static const URI_CHAR * URI_FUNC(ParseDecOctet)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseDecOctetOne)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseDecOctetTwo)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseDecOctetThree)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseDecOctetFour)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast);


/* Prototypes: UriNormalizeBase.h */
UriBool uriIsUnreserved(int code);

/* Prototypes: UriNormalize.c */
static int URI_FUNC(NormalizeSyntaxEngine)(URI_TYPE(Uri) * uri, unsigned int inMask,
    unsigned int * outMask);

static UriBool URI_FUNC(MakeRangeOwner)(unsigned int * doneMask,
    unsigned int maskTest, URI_TYPE(TextRange) * range);
static UriBool URI_FUNC(MakeOwner)(URI_TYPE(Uri) * uri,
    unsigned int * doneMask);

static void URI_FUNC(FixPercentEncodingInplace)(const URI_CHAR * first,
    const URI_CHAR ** afterLast);
static UriBool URI_FUNC(FixPercentEncodingMalloc)(const URI_CHAR ** first,
    const URI_CHAR ** afterLast);
static void URI_FUNC(FixPercentEncodingEngine)(
    const URI_CHAR * inFirst, const URI_CHAR * inAfterLast,
    const URI_CHAR * outFirst, const URI_CHAR ** outAfterLast);

static UriBool URI_FUNC(ContainsUppercaseLetters)(const URI_CHAR * first,
    const URI_CHAR * afterLast);
static UriBool URI_FUNC(ContainsUglyPercentEncoding)(const URI_CHAR * first,
    const URI_CHAR * afterLast);

static void URI_FUNC(LowercaseInplace)(const URI_CHAR * first,
    const URI_CHAR * afterLast);
static UriBool URI_FUNC(LowercaseMalloc)(const URI_CHAR ** first,
    const URI_CHAR ** afterLast);

static void URI_FUNC(PreventLeakage)(URI_TYPE(Uri) * uri,
    unsigned int revertMask);


/* Prototypes: UriParseBase.h */
void uriWriteQuadToDoubleByte(const unsigned char * hexDigits, int digitCount,
    unsigned char * output);
unsigned char uriGetOctetValue(const unsigned char * digits, int digitCount);

/* Prototypes: UriParse.c */
static const URI_CHAR * URI_FUNC(ParseAuthority)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseAuthorityTwo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseHexZero)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseHierPart)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseIpFutLoop)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseIpFutStopGo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseIpLit2)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseIPv6address2)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseMustBeSegmentNzNc)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseOwnHost)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseOwnHost2)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseOwnHostUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseOwnHostUserInfoNz)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseOwnPortUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseOwnUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParsePartHelperTwo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParsePathAbsEmpty)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParsePathAbsNoLeadSlash)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParsePathRootless)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParsePchar)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParsePctEncoded)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParsePctSubUnres)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParsePort)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseQueryFrag)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseSegment)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseSegmentNz)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseSegmentNzNcOrScheme2)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseUriReference)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseUriTail)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseUriTailTwo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);
static const URI_CHAR * URI_FUNC(ParseZeroMoreSlashSegs)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);

static UriBool URI_FUNC(OnExitOwnHost2)(URI_TYPE(ParserState) * state, const URI_CHAR * first);
static UriBool URI_FUNC(OnExitOwnHostUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first);
static UriBool URI_FUNC(OnExitOwnPortUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first);
static UriBool URI_FUNC(OnExitSegmentNzNcOrScheme2)(URI_TYPE(ParserState) * state, const URI_CHAR * first);
static void URI_FUNC(OnExitPartHelperTwo)(URI_TYPE(ParserState) * state);

static void URI_FUNC(ResetParserState)(URI_TYPE(ParserState) * state);

static UriBool URI_FUNC(PushPathSegment)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast);

static void URI_FUNC(StopSyntax)(URI_TYPE(ParserState) * state, const URI_CHAR * errorPos);
static void URI_FUNC(StopMalloc)(URI_TYPE(ParserState) * state);


/* Prototypes: UriQuery.c */
static int URI_FUNC(ComposeQueryEngine)(URI_CHAR * dest,
    const URI_TYPE(QueryList) * queryList,
    int maxChars, int * charsWritten, int * charsRequired,
    UriBool spaceToPlus, UriBool normalizeBreaks);

static UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** prevNext,
    int * itemCount, const URI_CHAR * keyFirst, const URI_CHAR * keyAfter,
    const URI_CHAR * valueFirst, const URI_CHAR * valueAfter,
    UriBool plusToSpace, UriBreakConversion breakConversion);


/* Prototypes: UriRecompose.c */
static int URI_FUNC(ToStringEngine)(URI_CHAR * dest, const URI_TYPE(Uri) * uri,
    int maxChars, int * charsWritten, int * charsRequired);




/* Source: UriCommon.c */
void URI_FUNC(ResetUri)(URI_TYPE(Uri) * uri) {
  memset(uri, 0, sizeof(URI_TYPE(Uri)));
}



/* Properly removes "." and ".." path segments */
UriBool URI_FUNC(RemoveDotSegments)(URI_TYPE(Uri) * uri,
    UriBool relative) {
  if (uri == NULL) {
    return URI_TRUE;
  }
  return URI_FUNC(RemoveDotSegmentsEx)(uri, relative, uri->owner);
}



UriBool URI_FUNC(RemoveDotSegmentsEx)(URI_TYPE(Uri) * uri,
        UriBool relative, UriBool pathOwned) {
  URI_TYPE(PathSegment) * walker;
  if ((uri == NULL) || (uri->pathHead == NULL)) {
    return URI_TRUE;
  }

  walker = uri->pathHead;
  walker->reserved = NULL; /* Prev pointer */
  do {
    UriBool removeSegment = URI_FALSE;
    int len = (int)(walker->text.afterLast - walker->text.first);
    switch (len) {
    case 1:
      if ((walker->text.first)[0] == _UT('.')) {
        /* "." segment -> remove if not essential */
        URI_TYPE(PathSegment) * const prev = ( URI_TYPE(PathSegment) * const )walker->reserved;
        URI_TYPE(PathSegment) * const nextBackup = ( URI_TYPE(PathSegment) * const )walker->next;

        /* Is this dot segment essential? */
        removeSegment = URI_TRUE;
        if (relative && (walker == uri->pathHead) && (walker->next != NULL)) {
          const URI_CHAR * ch = walker->next->text.first;
          for (; ch < walker->next->text.afterLast; ch++) {
            if (*ch == _UT(':')) {
              removeSegment = URI_FALSE;
              break;
            }
          }
        }

        if (removeSegment) {
          /* Last segment? */
          if (walker->next != NULL) {
            /* Not last segment */
            walker->next->reserved = prev;

            if (prev == NULL) {
              /* First but not last segment */
              uri->pathHead = walker->next;
            } else {
              /* Middle segment */
              prev->next = walker->next;
            }

            if (pathOwned && (walker->text.first != walker->text.afterLast)) {
              free((URI_CHAR *)walker->text.first);
            }
            free(walker);
          } else {
            /* Last segment */
            if (pathOwned && (walker->text.first != walker->text.afterLast)) {
              free((URI_CHAR *)walker->text.first);
            }

            if (prev == NULL) {
              /* Last and first */
              if (URI_FUNC(IsHostSet)(uri)) {
                /* Replace "." with empty segment to represent trailing slash */
                walker->text.first = URI_FUNC(SafeToPointTo);
                walker->text.afterLast = URI_FUNC(SafeToPointTo);
              } else {
                free(walker);

                uri->pathHead = NULL;
                uri->pathTail = NULL;
              }
            } else {
              /* Last but not first, replace "." with empty segment to represent trailing slash */
              walker->text.first = URI_FUNC(SafeToPointTo);
              walker->text.afterLast = URI_FUNC(SafeToPointTo);
            }
          }

          walker = nextBackup;
        }
      }
      break;

    case 2:
      if (((walker->text.first)[0] == _UT('.'))
          && ((walker->text.first)[1] == _UT('.'))) {
        /* Path ".." -> remove this and the previous segment */
        URI_TYPE(PathSegment) * const prev = ( URI_TYPE(PathSegment) * const )walker->reserved;
        URI_TYPE(PathSegment) * prevPrev;
        URI_TYPE(PathSegment) * const nextBackup = ( URI_TYPE(PathSegment) * const )walker->next;

        removeSegment = URI_TRUE;
        if (relative) {
          if (prev == NULL) {
            removeSegment = URI_FALSE;
          } else if ((prev != NULL)
              && ((prev->text.afterLast - prev->text.first) == 2)
              && ((prev->text.first)[0] == _UT('.'))
              && ((prev->text.first)[1] == _UT('.'))) {
            removeSegment = URI_FALSE;
          }
        }

        if (removeSegment) {
          if (prev != NULL) {
            /* Not first segment */
            prevPrev = ( URI_TYPE(PathSegment) * const )prev->reserved;
            if (prevPrev != NULL) {
              /* Not even prev is the first one */
              prevPrev->next = walker->next;
              if (walker->next != NULL) {
                walker->next->reserved = prevPrev;
              } else {
                /* Last segment -> insert "" segment to represent trailing slash, update tail */
                URI_TYPE(PathSegment) * const segment = ( URI_TYPE(PathSegment) * const )malloc(1 * sizeof(URI_TYPE(PathSegment)));
                if (segment == NULL) {
                  if (pathOwned && (walker->text.first != walker->text.afterLast)) {
                    free((URI_CHAR *)walker->text.first);
                  }
                  free(walker);

                  if (pathOwned && (prev->text.first != prev->text.afterLast)) {
                    free((URI_CHAR *)prev->text.first);
                  }
                  free(prev);

                  return URI_FALSE; /* Raises malloc error */
                }
                memset(segment, 0, sizeof(URI_TYPE(PathSegment)));
                segment->text.first = URI_FUNC(SafeToPointTo);
                segment->text.afterLast = URI_FUNC(SafeToPointTo);
                prevPrev->next = segment;
                uri->pathTail = segment;
              }

              if (pathOwned && (walker->text.first != walker->text.afterLast)) {
                free((URI_CHAR *)walker->text.first);
              }
              free(walker);

              if (pathOwned && (prev->text.first != prev->text.afterLast)) {
                free((URI_CHAR *)prev->text.first);
              }
              free(prev);

              walker = nextBackup;
            } else {
              /* Prev is the first segment */
              if (walker->next != NULL) {
                uri->pathHead = walker->next;
                walker->next->reserved = NULL;

                if (pathOwned && (walker->text.first != walker->text.afterLast)) {
                  free((URI_CHAR *)walker->text.first);
                }
                free(walker);
              } else {
                /* Re-use segment for "" path segment to represent trailing slash, update tail */
                URI_TYPE(PathSegment) * const segment = walker;
                if (pathOwned && (segment->text.first != segment->text.afterLast)) {
                  free((URI_CHAR *)segment->text.first);
                }
                segment->text.first = URI_FUNC(SafeToPointTo);
                segment->text.afterLast = URI_FUNC(SafeToPointTo);
                uri->pathHead = segment;
                uri->pathTail = segment;
              }

              if (pathOwned && (prev->text.first != prev->text.afterLast)) {
                free((URI_CHAR *)prev->text.first);
              }
              free(prev);

              walker = nextBackup;
            }
          } else {
            URI_TYPE(PathSegment) * const nextBackup = walker->next;
            /* First segment -> update head pointer */
            uri->pathHead = walker->next;
            if (walker->next != NULL) {
              walker->next->reserved = NULL;
            } else {
              /* Last segment -> update tail */
              uri->pathTail = NULL;
            }

            if (pathOwned && (walker->text.first != walker->text.afterLast)) {
              free((URI_CHAR *)walker->text.first);
            }
            free(walker);

            walker = nextBackup;
          }
        }
      }
      break;

    }

    if (!removeSegment) {
      if (walker->next != NULL) {
        walker->next->reserved = walker;
      } else {
        /* Last segment -> update tail */
        uri->pathTail = walker;
      }
      walker = walker->next;
    }
  } while (walker != NULL);

  return URI_TRUE;
}



/* Properly removes "." and ".." path segments */
UriBool URI_FUNC(RemoveDotSegmentsAbsolute)(URI_TYPE(Uri) * uri) {
  const UriBool ABSOLUTE = URI_FALSE;
  return URI_FUNC(RemoveDotSegments)(uri, ABSOLUTE);
}



unsigned char URI_FUNC(HexdigToInt)(URI_CHAR hexdig) {
  switch (hexdig) {
  case _UT('0'):
  case _UT('1'):
  case _UT('2'):
  case _UT('3'):
  case _UT('4'):
  case _UT('5'):
  case _UT('6'):
  case _UT('7'):
  case _UT('8'):
  case _UT('9'):
    return (unsigned char)(9 + hexdig - _UT('9'));

  case _UT('a'):
  case _UT('b'):
  case _UT('c'):
  case _UT('d'):
  case _UT('e'):
  case _UT('f'):
    return (unsigned char)(15 + hexdig - _UT('f'));

  case _UT('A'):
  case _UT('B'):
  case _UT('C'):
  case _UT('D'):
  case _UT('E'):
  case _UT('F'):
    return (unsigned char)(15 + hexdig - _UT('F'));

  default:
    return 0;
  }
}



URI_CHAR URI_FUNC(HexToLetter)(unsigned int value) {
  /* Uppercase recommended in section 2.1. of RFC 3986 *
   * http://tools.ietf.org/html/rfc3986#section-2.1    */
  return URI_FUNC(HexToLetterEx)(value, URI_TRUE);
}



URI_CHAR URI_FUNC(HexToLetterEx)(unsigned int value, UriBool uppercase) {
  switch (value) {
  case  0: return _UT('0');
  case  1: return _UT('1');
  case  2: return _UT('2');
  case  3: return _UT('3');
  case  4: return _UT('4');
  case  5: return _UT('5');
  case  6: return _UT('6');
  case  7: return _UT('7');
  case  8: return _UT('8');
  case  9: return _UT('9');

  case 10: return (uppercase == URI_TRUE) ? _UT('A') : _UT('a');
  case 11: return (uppercase == URI_TRUE) ? _UT('B') : _UT('b');
  case 12: return (uppercase == URI_TRUE) ? _UT('C') : _UT('c');
  case 13: return (uppercase == URI_TRUE) ? _UT('D') : _UT('d');
  case 14: return (uppercase == URI_TRUE) ? _UT('E') : _UT('e');
  default: return (uppercase == URI_TRUE) ? _UT('F') : _UT('f');
  }
}



/* Checks if a URI has the host component set. */
UriBool URI_FUNC(IsHostSet)(const URI_TYPE(Uri) * uri) {
  return (uri != NULL)
      && ((uri->hostText.first != NULL)
        || (uri->hostData.ip4 != NULL)
        || (uri->hostData.ip6 != NULL)
        || (uri->hostData.ipFuture.first != NULL)
      );
}



/* Copies the path segment list from one URI to another. */
UriBool URI_FUNC(CopyPath)(URI_TYPE(Uri) * dest,
    const URI_TYPE(Uri) * source) {
  if (source->pathHead == NULL) {
    /* No path component */
    dest->pathHead = NULL;
    dest->pathTail = NULL;
  } else {
    /* Copy list but not the text contained */
    URI_TYPE(PathSegment) * sourceWalker = source->pathHead;
    URI_TYPE(PathSegment) * destPrev = NULL;
    do {
      URI_TYPE(PathSegment) * cur = ( URI_TYPE(PathSegment) * const )malloc(sizeof(URI_TYPE(PathSegment)));
      if (cur == NULL) {
        /* Fix broken list */
        if (destPrev != NULL) {
          destPrev->next = NULL;
        }
        return URI_FALSE; /* Raises malloc error */
      }

      /* From this functions usage we know that *
       * the dest URI cannot be uri->owner      */
      cur->text = sourceWalker->text;
      if (destPrev == NULL) {
        /* First segment ever */
        dest->pathHead = cur;
      } else {
        destPrev->next = cur;
      }
      destPrev = cur;
      sourceWalker = sourceWalker->next;
    } while (sourceWalker != NULL);
    dest->pathTail = destPrev;
    dest->pathTail->next = NULL;
  }

  dest->absolutePath = source->absolutePath;
  return URI_TRUE;
}



/* Copies the authority part of an URI over to another. */
UriBool URI_FUNC(CopyAuthority)(URI_TYPE(Uri) * dest,
    const URI_TYPE(Uri) * source) {
  /* From this functions usage we know that *
   * the dest URI cannot be uri->owner      */

  /* Copy userInfo */
  dest->userInfo = source->userInfo;

  /* Copy hostText */
  dest->hostText = source->hostText;

  /* Copy hostData */
  if (source->hostData.ip4 != NULL) {
    dest->hostData.ip4 = ( UriIp4* )malloc(sizeof(UriIp4));
    if (dest->hostData.ip4 == NULL) {
      return URI_FALSE; /* Raises malloc error */
    }
    *(dest->hostData.ip4) = *(source->hostData.ip4);
    dest->hostData.ip6 = NULL;
    dest->hostData.ipFuture.first = NULL;
    dest->hostData.ipFuture.afterLast = NULL;
  } else if (source->hostData.ip6 != NULL) {
    dest->hostData.ip4 = NULL;
    dest->hostData.ip6 = ( UriIp6* )malloc(sizeof(UriIp6));
    if (dest->hostData.ip6 == NULL) {
      return URI_FALSE; /* Raises malloc error */
    }
    *(dest->hostData.ip6) = *(source->hostData.ip6);
    dest->hostData.ipFuture.first = NULL;
    dest->hostData.ipFuture.afterLast = NULL;
  } else {
    dest->hostData.ip4 = NULL;
    dest->hostData.ip6 = NULL;
    dest->hostData.ipFuture = source->hostData.ipFuture;
  }

  /* Copy portText */
  dest->portText = source->portText;

  return URI_TRUE;
}



UriBool URI_FUNC(FixAmbiguity)(URI_TYPE(Uri) * uri) {
  URI_TYPE(PathSegment) * segment;

  if (  /* Case 1: absolute path, empty first segment */
      (uri->absolutePath
      && (uri->pathHead != NULL)
      && (uri->pathHead->text.afterLast == uri->pathHead->text.first))

      /* Case 2: relative path, empty first and second segment */
      || (!uri->absolutePath
      && (uri->pathHead != NULL)
      && (uri->pathHead->next != NULL)
      && (uri->pathHead->text.afterLast == uri->pathHead->text.first)
      && (uri->pathHead->next->text.afterLast == uri->pathHead->next->text.first))) {
    /* NOOP */
  } else {
    return URI_TRUE;
  }

  segment = ( URI_TYPE(PathSegment) * const )malloc(1 * sizeof(URI_TYPE(PathSegment)));
  if (segment == NULL) {
    return URI_FALSE; /* Raises malloc error */
  }

  /* Insert "." segment in front */
  segment->next = uri->pathHead;
  segment->text.first = URI_FUNC(ConstPwd);
  segment->text.afterLast = URI_FUNC(ConstPwd) + 1;
  uri->pathHead = segment;
  return URI_TRUE;
}



void URI_FUNC(FixEmptyTrailSegment)(URI_TYPE(Uri) * uri) {
  /* Fix path if only one empty segment */
  if (!uri->absolutePath
      && !URI_FUNC(IsHostSet)(uri)
      && (uri->pathHead != NULL)
      && (uri->pathHead->next == NULL)
      && (uri->pathHead->text.first == uri->pathHead->text.afterLast)) {
    free(uri->pathHead);
    uri->pathHead = NULL;
    uri->pathTail = NULL;
  }
}


/* Source: UriIp4Base.c */
void uriStackToOctet(UriIp4Parser * parser, unsigned char * octet) {
  switch (parser->stackCount) {
  case 1:
    *octet = parser->stackOne;
    break;

  case 2:
    *octet = parser->stackOne * 10
        + parser->stackTwo;
    break;

  case 3:
    *octet = parser->stackOne * 100
        + parser->stackTwo * 10
        + parser->stackThree;
    break;

  default:
    ;
  }
  parser->stackCount = 0;
}



void uriPushToStack(UriIp4Parser * parser, unsigned char digit) {
  switch (parser->stackCount) {
  case 0:
    parser->stackOne = digit;
    parser->stackCount = 1;
    break;

  case 1:
    parser->stackTwo = digit;
    parser->stackCount = 2;
    break;

  case 2:
    parser->stackThree = digit;
    parser->stackCount = 3;
    break;

  default:
    ;
  }
}


/* Source: UriIp4.c */

/*
 * [ipFourAddress]->[decOctet]<.>[decOctet]<.>[decOctet]<.>[decOctet]
 */
int URI_FUNC(ParseIpFourAddress)(unsigned char * octetOutput,
    const URI_CHAR * first, const URI_CHAR * afterLast) {
  const URI_CHAR * after;
  UriIp4Parser parser;

  /* Essential checks */
  if ((octetOutput == NULL) || (first == NULL)
      || (afterLast <= first)) {
    return URI_ERROR_SYNTAX;
  }

  /* Reset parser */
  parser.stackCount = 0;

  /* Octet #1 */
  after = URI_FUNC(ParseDecOctet)(&parser, first, afterLast);
  if ((after == NULL) || (after >= afterLast) || (*after != _UT('.'))) {
    return URI_ERROR_SYNTAX;
  }
  uriStackToOctet(&parser, octetOutput);

  /* Octet #2 */
  after = URI_FUNC(ParseDecOctet)(&parser, after + 1, afterLast);
  if ((after == NULL) || (after >= afterLast) || (*after != _UT('.'))) {
    return URI_ERROR_SYNTAX;
  }
  uriStackToOctet(&parser, octetOutput + 1);

  /* Octet #3 */
  after = URI_FUNC(ParseDecOctet)(&parser, after + 1, afterLast);
  if ((after == NULL) || (after >= afterLast) || (*after != _UT('.'))) {
    return URI_ERROR_SYNTAX;
  }
  uriStackToOctet(&parser, octetOutput + 2);

  /* Octet #4 */
  after = URI_FUNC(ParseDecOctet)(&parser, after + 1, afterLast);
  if (after != afterLast) {
    return URI_ERROR_SYNTAX;
  }
  uriStackToOctet(&parser, octetOutput + 3);

  return URI_SUCCESS;
}



/*
 * [decOctet]-><0>
 * [decOctet]-><1>[decOctetOne]
 * [decOctet]-><2>[decOctetTwo]
 * [decOctet]-><3>[decOctetThree]
 * [decOctet]-><4>[decOctetThree]
 * [decOctet]-><5>[decOctetThree]
 * [decOctet]-><6>[decOctetThree]
 * [decOctet]-><7>[decOctetThree]
 * [decOctet]-><8>[decOctetThree]
 * [decOctet]-><9>[decOctetThree]
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseDecOctet)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return NULL;
  }

  switch (*first) {
  case _UT('0'):
    uriPushToStack(parser, 0);
    return first + 1;

  case _UT('1'):
    uriPushToStack(parser, 1);
    return (const URI_CHAR *)URI_FUNC(ParseDecOctetOne)(parser, first + 1, afterLast);

  case _UT('2'):
    uriPushToStack(parser, 2);
    return (const URI_CHAR *)URI_FUNC(ParseDecOctetTwo)(parser, first + 1, afterLast);

  case _UT('3'):
  case _UT('4'):
  case _UT('5'):
  case _UT('6'):
  case _UT('7'):
  case _UT('8'):
  case _UT('9'):
    uriPushToStack(parser, (unsigned char)(9 + *first - _UT('9')));
    return (const URI_CHAR *)URI_FUNC(ParseDecOctetThree)(parser, first + 1, afterLast);

  default:
    return NULL;
  }
}



/*
 * [decOctetOne]-><NULL>
 * [decOctetOne]->[DIGIT][decOctetThree]
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseDecOctetOne)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('0'):
  case _UT('1'):
  case _UT('2'):
  case _UT('3'):
  case _UT('4'):
  case _UT('5'):
  case _UT('6'):
  case _UT('7'):
  case _UT('8'):
  case _UT('9'):
    uriPushToStack(parser, (unsigned char)(9 + *first - _UT('9')));
    return (const URI_CHAR *)URI_FUNC(ParseDecOctetThree)(parser, first + 1, afterLast);

  default:
    return first;
  }
}



/*
 * [decOctetTwo]-><NULL>
 * [decOctetTwo]-><0>[decOctetThree]
 * [decOctetTwo]-><1>[decOctetThree]
 * [decOctetTwo]-><2>[decOctetThree]
 * [decOctetTwo]-><3>[decOctetThree]
 * [decOctetTwo]-><4>[decOctetThree]
 * [decOctetTwo]-><5>[decOctetFour]
 * [decOctetTwo]-><6>
 * [decOctetTwo]-><7>
 * [decOctetTwo]-><8>
 * [decOctetTwo]-><9>
*/
static URI_INLINE const URI_CHAR * URI_FUNC(ParseDecOctetTwo)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('0'):
  case _UT('1'):
  case _UT('2'):
  case _UT('3'):
  case _UT('4'):
    uriPushToStack(parser, (unsigned char)(9 + *first - _UT('9')));
    return (const URI_CHAR *)URI_FUNC(ParseDecOctetThree)(parser, first + 1, afterLast);

  case _UT('5'):
    uriPushToStack(parser, 5);
    return (const URI_CHAR *)URI_FUNC(ParseDecOctetFour)(parser, first + 1, afterLast);

  case _UT('6'):
  case _UT('7'):
  case _UT('8'):
  case _UT('9'):
    uriPushToStack(parser, (unsigned char)(9 + *first - _UT('9')));
    return first + 1;

  default:
    return first;
  }
}



/*
 * [decOctetThree]-><NULL>
 * [decOctetThree]->[DIGIT]
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseDecOctetThree)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('0'):
  case _UT('1'):
  case _UT('2'):
  case _UT('3'):
  case _UT('4'):
  case _UT('5'):
  case _UT('6'):
  case _UT('7'):
  case _UT('8'):
  case _UT('9'):
    uriPushToStack(parser, (unsigned char)(9 + *first - _UT('9')));
    return first + 1;

  default:
    return first;
  }
}



/*
 * [decOctetFour]-><NULL>
 * [decOctetFour]-><0>
 * [decOctetFour]-><1>
 * [decOctetFour]-><2>
 * [decOctetFour]-><3>
 * [decOctetFour]-><4>
 * [decOctetFour]-><5>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseDecOctetFour)(UriIp4Parser * parser,
    const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('0'):
  case _UT('1'):
  case _UT('2'):
  case _UT('3'):
  case _UT('4'):
  case _UT('5'):
    uriPushToStack(parser, (unsigned char)(9 + *first - _UT('9')));
    return first + 1;

  default:
    return first;
  }
}


/* Source: UriCompare.c */
static int URI_FUNC(CompareRange)(const URI_TYPE(TextRange) * a,
    const URI_TYPE(TextRange) * b);



/* Compares two text ranges for equal text content */
static URI_INLINE int URI_FUNC(CompareRange)(const URI_TYPE(TextRange) * a,
    const URI_TYPE(TextRange) * b) {
  int diff;

  /* NOTE: Both NULL means equal! */
  if ((a == NULL) || (b == NULL)) {
    return ((a == NULL) && (b == NULL)) ? URI_TRUE : URI_FALSE;
  }

  diff = ((int)(a->afterLast - a->first) - (int)(b->afterLast - b->first));
  if (diff > 0) {
    return 1;
  } else if (diff < 0) {
    return -1;
  }

  return URI_STRNCMP(a->first, b->first, (a->afterLast - a->first));
}



UriBool URI_FUNC(EqualsUri)(const URI_TYPE(Uri) * a,
    const URI_TYPE(Uri) * b) {
  /* NOTE: Both NULL means equal! */
  if ((a == NULL) || (b == NULL)) {
    return ((a == NULL) && (b == NULL)) ? URI_TRUE : URI_FALSE;
  }

  /* scheme */
  if (URI_FUNC(CompareRange)(&(a->scheme), &(b->scheme))) {
    return URI_FALSE;
  }

  /* absolutePath */
  if ((a->scheme.first == NULL)&& (a->absolutePath != b->absolutePath)) {
    return URI_FALSE;
  }

  /* userInfo */
  if (URI_FUNC(CompareRange)(&(a->userInfo), &(b->userInfo))) {
    return URI_FALSE;
  }

  /* Host */
  if (((a->hostData.ip4 == NULL) != (b->hostData.ip4 == NULL))
      || ((a->hostData.ip6 == NULL) != (b->hostData.ip6 == NULL))
      || ((a->hostData.ipFuture.first == NULL)
        != (b->hostData.ipFuture.first == NULL))) {
    return URI_FALSE;
  }

  if (a->hostData.ip4 != NULL) {
    if (memcmp(a->hostData.ip4->data, b->hostData.ip4->data, 4)) {
      return URI_FALSE;
    }
  }

  if (a->hostData.ip6 != NULL) {
    if (memcmp(a->hostData.ip6->data, b->hostData.ip6->data, 16)) {
      return URI_FALSE;
    }
  }

  if (a->hostData.ipFuture.first != NULL) {
    if (URI_FUNC(CompareRange)(&(a->hostData.ipFuture), &(b->hostData.ipFuture))) {
      return URI_FALSE;
    }
  }

  if ((a->hostData.ip4 == NULL)
      && (a->hostData.ip6 == NULL)
      && (a->hostData.ipFuture.first == NULL)) {
    if (URI_FUNC(CompareRange)(&(a->hostText), &(b->hostText))) {
      return URI_FALSE;
    }
  }

  /* portText */
  if (URI_FUNC(CompareRange)(&(a->portText), &(b->portText))) {
    return URI_FALSE;
  }

  /* Path */
  if ((a->pathHead == NULL) != (b->pathHead == NULL)) {
    return URI_FALSE;
  }

  if (a->pathHead != NULL) {
    URI_TYPE(PathSegment) * walkA = a->pathHead;
    URI_TYPE(PathSegment) * walkB = b->pathHead;
    do {
      if (URI_FUNC(CompareRange)(&(walkA->text), &(walkB->text))) {
        return URI_FALSE;
      }
      if ((walkA->next == NULL) != (walkB->next == NULL)) {
        return URI_FALSE;
      }
      walkA = walkA->next;
      walkB = walkB->next;
    } while (walkA != NULL);
  }

  /* query */
  if (URI_FUNC(CompareRange)(&(a->query), &(b->query))) {
    return URI_FALSE;
  }

  /* fragment */
  if (URI_FUNC(CompareRange)(&(a->fragment), &(b->fragment))) {
    return URI_FALSE;
  }

  return URI_TRUE; /* Equal*/
}


/* Source: UriEscape.c */
URI_CHAR * URI_FUNC(Escape)(const URI_CHAR * in, URI_CHAR * out,
    UriBool spaceToPlus, UriBool normalizeBreaks) {
  return URI_FUNC(EscapeEx)(in, NULL, out, spaceToPlus, normalizeBreaks);
}



URI_CHAR * URI_FUNC(EscapeEx)(const URI_CHAR * inFirst,
    const URI_CHAR * inAfterLast, URI_CHAR * out,
    UriBool spaceToPlus, UriBool normalizeBreaks) {
  const URI_CHAR * read = inFirst;
  URI_CHAR * write = out;
  UriBool prevWasCr = URI_FALSE;
  if ((out == NULL) || (inFirst == out)) {
    return NULL;
  } else if (inFirst == NULL) {
    if (out != NULL) {
      out[0] = _UT('\0');
    }
    return out;
  }

  for (;;) {
    if ((inAfterLast != NULL) && (read >= inAfterLast)) {
      write[0] = _UT('\0');
      return write;
    }

    switch (read[0]) {
    case _UT('\0'):
      write[0] = _UT('\0');
      return write;

    case _UT(' '):
      if (spaceToPlus) {
        write[0] = _UT('+');
        write++;
      } else {
        write[0] = _UT('%');
        write[1] = _UT('2');
        write[2] = _UT('0');
        write += 3;
      }
      prevWasCr = URI_FALSE;
      break;

    case _UT('a'): /* ALPHA */
    case _UT('A'):
    case _UT('b'):
    case _UT('B'):
    case _UT('c'):
    case _UT('C'):
    case _UT('d'):
    case _UT('D'):
    case _UT('e'):
    case _UT('E'):
    case _UT('f'):
    case _UT('F'):
    case _UT('g'):
    case _UT('G'):
    case _UT('h'):
    case _UT('H'):
    case _UT('i'):
    case _UT('I'):
    case _UT('j'):
    case _UT('J'):
    case _UT('k'):
    case _UT('K'):
    case _UT('l'):
    case _UT('L'):
    case _UT('m'):
    case _UT('M'):
    case _UT('n'):
    case _UT('N'):
    case _UT('o'):
    case _UT('O'):
    case _UT('p'):
    case _UT('P'):
    case _UT('q'):
    case _UT('Q'):
    case _UT('r'):
    case _UT('R'):
    case _UT('s'):
    case _UT('S'):
    case _UT('t'):
    case _UT('T'):
    case _UT('u'):
    case _UT('U'):
    case _UT('v'):
    case _UT('V'):
    case _UT('w'):
    case _UT('W'):
    case _UT('x'):
    case _UT('X'):
    case _UT('y'):
    case _UT('Y'):
    case _UT('z'):
    case _UT('Z'):
    case _UT('0'): /* DIGIT */
    case _UT('1'):
    case _UT('2'):
    case _UT('3'):
    case _UT('4'):
    case _UT('5'):
    case _UT('6'):
    case _UT('7'):
    case _UT('8'):
    case _UT('9'):
    case _UT('-'): /* "-" / "." / "_" / "~" */
    case _UT('.'):
    case _UT('_'):
    case _UT('~'):
      /* Copy unmodified */
      write[0] = read[0];
      write++;

      prevWasCr = URI_FALSE;
      break;

    case _UT('\x0a'):
      if (normalizeBreaks) {
        if (!prevWasCr) {
          write[0] = _UT('%');
          write[1] = _UT('0');
          write[2] = _UT('D');
          write[3] = _UT('%');
          write[4] = _UT('0');
          write[5] = _UT('A');
          write += 6;
        }
      } else {
        write[0] = _UT('%');
        write[1] = _UT('0');
        write[2] = _UT('A');
        write += 3;
      }
      prevWasCr = URI_FALSE;
      break;

    case _UT('\x0d'):
      if (normalizeBreaks) {
        write[0] = _UT('%');
        write[1] = _UT('0');
        write[2] = _UT('D');
        write[3] = _UT('%');
        write[4] = _UT('0');
        write[5] = _UT('A');
        write += 6;
      } else {
        write[0] = _UT('%');
        write[1] = _UT('0');
        write[2] = _UT('D');
        write += 3;
      }
      prevWasCr = URI_TRUE;
      break;

    default:
      /* Percent encode */
      {
        const unsigned char code = (unsigned char)read[0];
        write[0] = _UT('%');
        write[1] = URI_FUNC(HexToLetter)(code >> 4);
        write[2] = URI_FUNC(HexToLetter)(code & 0x0f);
        write += 3;
      }
      prevWasCr = URI_FALSE;
      break;
    }

    read++;
  }
}



const URI_CHAR * URI_FUNC(UnescapeInPlace)(URI_CHAR * inout) {
  return URI_FUNC(UnescapeInPlaceEx)(inout, URI_FALSE, URI_BR_DONT_TOUCH);
}



const URI_CHAR * URI_FUNC(UnescapeInPlaceEx)(URI_CHAR * inout,
    UriBool plusToSpace, UriBreakConversion breakConversion) {
  URI_CHAR * read = inout;
  URI_CHAR * write = inout;
  UriBool prevWasCr = URI_FALSE;

  if (inout == NULL) {
    return NULL;
  }

  for (;;) {
    switch (read[0]) {
    case _UT('\0'):
      if (read > write) {
        write[0] = _UT('\0');
      }
      return write;

    case _UT('%'):
      switch (read[1]) {
      case _UT('0'):
      case _UT('1'):
      case _UT('2'):
      case _UT('3'):
      case _UT('4'):
      case _UT('5'):
      case _UT('6'):
      case _UT('7'):
      case _UT('8'):
      case _UT('9'):
      case _UT('a'):
      case _UT('b'):
      case _UT('c'):
      case _UT('d'):
      case _UT('e'):
      case _UT('f'):
      case _UT('A'):
      case _UT('B'):
      case _UT('C'):
      case _UT('D'):
      case _UT('E'):
      case _UT('F'):
        switch (read[2]) {
        case _UT('0'):
        case _UT('1'):
        case _UT('2'):
        case _UT('3'):
        case _UT('4'):
        case _UT('5'):
        case _UT('6'):
        case _UT('7'):
        case _UT('8'):
        case _UT('9'):
        case _UT('a'):
        case _UT('b'):
        case _UT('c'):
        case _UT('d'):
        case _UT('e'):
        case _UT('f'):
        case _UT('A'):
        case _UT('B'):
        case _UT('C'):
        case _UT('D'):
        case _UT('E'):
        case _UT('F'):
          {
            /* Percent group found */
            const unsigned char left = URI_FUNC(HexdigToInt)(read[1]);
            const unsigned char right = URI_FUNC(HexdigToInt)(read[2]);
            const int code = 16 * left + right;
            switch (code) {
            case 10:
              switch (breakConversion) {
              case URI_BR_TO_LF:
                if (!prevWasCr) {
                  write[0] = (URI_CHAR)10;
                  write++;
                }
                break;

              case URI_BR_TO_CRLF:
                if (!prevWasCr) {
                  write[0] = (URI_CHAR)13;
                  write[1] = (URI_CHAR)10;
                  write += 2;
                }
                break;

              case URI_BR_TO_CR:
                if (!prevWasCr) {
                  write[0] = (URI_CHAR)13;
                  write++;
                }
                break;

              case URI_BR_DONT_TOUCH:
              default:
                write[0] = (URI_CHAR)10;
                write++;

              }
              prevWasCr = URI_FALSE;
              break;

            case 13:
              switch (breakConversion) {
              case URI_BR_TO_LF:
                write[0] = (URI_CHAR)10;
                write++;
                break;

              case URI_BR_TO_CRLF:
                write[0] = (URI_CHAR)13;
                write[1] = (URI_CHAR)10;
                write += 2;
                break;

              case URI_BR_TO_CR:
                write[0] = (URI_CHAR)13;
                write++;
                break;

              case URI_BR_DONT_TOUCH:
              default:
                write[0] = (URI_CHAR)13;
                write++;

              }
              prevWasCr = URI_TRUE;
              break;

            default:
              write[0] = (URI_CHAR)(code);
              write++;

              prevWasCr = URI_FALSE;

            }
            read += 3;
          }
          break;

        default:
          /* Copy two chars unmodified and */
          /* look at this char again */
          if (read > write) {
            write[0] = read[0];
            write[1] = read[1];
          }
          read += 2;
          write += 2;

          prevWasCr = URI_FALSE;
        }
        break;

      default:
        /* Copy one char unmodified and */
        /* look at this char again */
        if (read > write) {
          write[0] = read[0];
        }
        read++;
        write++;

        prevWasCr = URI_FALSE;
      }
      break;

    case _UT('+'):
      if (plusToSpace) {
        /* Convert '+' to ' ' */
        write[0] = _UT(' ');
      } else {
        /* Copy one char unmodified */
        if (read > write) {
          write[0] = read[0];
        }
      }
      read++;
      write++;

      prevWasCr = URI_FALSE;
      break;

    default:
      /* Copy one char unmodified */
      if (read > write) {
        write[0] = read[0];
      }
      read++;
      write++;

      prevWasCr = URI_FALSE;
    }
  }
}


/* Source: UriFile.c */

static URI_INLINE int URI_FUNC(FilenameToUriString)(const URI_CHAR * filename,
    URI_CHAR * uriString, UriBool fromUnix) {
  const URI_CHAR * input = filename;
  const URI_CHAR * lastSep = input - 1;
  UriBool firstSegment = URI_TRUE;
  URI_CHAR * output = uriString;
  const UriBool absolute = (filename != NULL) && ((fromUnix && (filename[0] == _UT('/')))
      || (!fromUnix && (filename[0] != _UT('\0')) && (filename[1] == _UT(':'))));

  if ((filename == NULL) || (uriString == NULL)) {
    return URI_ERROR_NULL;
  }

  if (absolute) {
    const URI_CHAR * const prefix = fromUnix ? _UT("file://") : _UT("file:///");
    const int prefixLen = fromUnix ? 7 : 8;

    /* Copy prefix */
    memcpy(uriString, prefix, prefixLen * sizeof(URI_CHAR));
    output += prefixLen;
  }

  /* Copy and escape on the fly */
  for (;;) {
    if ((input[0] == _UT('\0'))
        || (fromUnix && input[0] == _UT('/'))
        || (!fromUnix && input[0] == _UT('\\'))) {
      /* Copy text after last seperator */
      if (lastSep + 1 < input) {
        if (!fromUnix && absolute && (firstSegment == URI_TRUE)) {
          /* Quick hack to not convert "C:" to "C%3A" */
          const int charsToCopy = (int)(input - (lastSep + 1));
          memcpy(output, lastSep + 1, charsToCopy * sizeof(URI_CHAR));
          output += charsToCopy;
        } else {
          output = URI_FUNC(EscapeEx)(lastSep + 1, input, output,
              URI_FALSE, URI_FALSE);
        }
      }
      firstSegment = URI_FALSE;
    }

    if (input[0] == _UT('\0')) {
      output[0] = _UT('\0');
      break;
    } else if (fromUnix && (input[0] == _UT('/'))) {
      /* Copy separators unmodified */
      output[0] = _UT('/');
      output++;
      lastSep = input;
    } else if (!fromUnix && (input[0] == _UT('\\'))) {
      /* Convert backslashes to forward slashes */
      output[0] = _UT('/');
      output++;
      lastSep = input;
    }
    input++;
  }

  return URI_SUCCESS;
}



static URI_INLINE int URI_FUNC(UriStringToFilename)(const URI_CHAR * uriString,
    URI_CHAR * filename, UriBool toUnix) {
  const URI_CHAR * const prefix = toUnix ? _UT("file://") : _UT("file:///");
  const int prefixLen = toUnix ? 7 : 8;
  URI_CHAR * walker = filename;
  size_t charsToCopy;
  const UriBool absolute = (URI_STRNCMP(uriString, prefix, prefixLen) == 0);
  const int charsToSkip = (absolute ? prefixLen : 0);

  charsToCopy = URI_STRLEN(uriString + charsToSkip) + 1;
  memcpy(filename, uriString + charsToSkip, charsToCopy * sizeof(URI_CHAR));
  URI_FUNC(UnescapeInPlaceEx)(filename, URI_FALSE, URI_BR_DONT_TOUCH);

  /* Convert forward slashes to backslashes */
  if (!toUnix) {
    while (walker[0] != _UT('\0')) {
      if (walker[0] == _UT('/')) {
        walker[0] = _UT('\\');
      }
      walker++;
    }
  }

  return URI_SUCCESS;
}



int URI_FUNC(UnixFilenameToUriString)(const URI_CHAR * filename, URI_CHAR * uriString) {
  return URI_FUNC(FilenameToUriString)(filename, uriString, URI_TRUE);
}



int URI_FUNC(WindowsFilenameToUriString)(const URI_CHAR * filename, URI_CHAR * uriString) {
  return URI_FUNC(FilenameToUriString)(filename, uriString, URI_FALSE);
}



int URI_FUNC(UriStringToUnixFilename)(const URI_CHAR * uriString, URI_CHAR * filename) {
  return URI_FUNC(UriStringToFilename)(uriString, filename, URI_TRUE);
}



int URI_FUNC(UriStringToWindowsFilename)(const URI_CHAR * uriString, URI_CHAR * filename) {
  return URI_FUNC(UriStringToFilename)(uriString, filename, URI_FALSE);
}


/* Source: UriNormalizeBase.c */
UriBool uriIsUnreserved(int code) {
  switch (code) {
  case L'a': /* ALPHA */
  case L'A':
  case L'b':
  case L'B':
  case L'c':
  case L'C':
  case L'd':
  case L'D':
  case L'e':
  case L'E':
  case L'f':
  case L'F':
  case L'g':
  case L'G':
  case L'h':
  case L'H':
  case L'i':
  case L'I':
  case L'j':
  case L'J':
  case L'k':
  case L'K':
  case L'l':
  case L'L':
  case L'm':
  case L'M':
  case L'n':
  case L'N':
  case L'o':
  case L'O':
  case L'p':
  case L'P':
  case L'q':
  case L'Q':
  case L'r':
  case L'R':
  case L's':
  case L'S':
  case L't':
  case L'T':
  case L'u':
  case L'U':
  case L'v':
  case L'V':
  case L'w':
  case L'W':
  case L'x':
  case L'X':
  case L'y':
  case L'Y':
  case L'z':
  case L'Z':
  case L'0': /* DIGIT */
  case L'1':
  case L'2':
  case L'3':
  case L'4':
  case L'5':
  case L'6':
  case L'7':
  case L'8':
  case L'9':
  case L'-': /* "-" / "." / "_" / "~" */
  case L'.':
  case L'_':
  case L'~':
    return URI_TRUE;

  default:
    return URI_FALSE;
  }
}


/* Source: UriNormalize.c */

static URI_INLINE void URI_FUNC(PreventLeakage)(URI_TYPE(Uri) * uri,
    unsigned int revertMask) {
  if (revertMask & URI_NORMALIZE_SCHEME) {
    free((URI_CHAR *)uri->scheme.first);
    uri->scheme.first = NULL;
    uri->scheme.afterLast = NULL;
  }

  if (revertMask & URI_NORMALIZE_USER_INFO) {
    free((URI_CHAR *)uri->userInfo.first);
    uri->userInfo.first = NULL;
    uri->userInfo.afterLast = NULL;
  }

  if (revertMask & URI_NORMALIZE_HOST) {
    if (uri->hostData.ipFuture.first != NULL) {
      /* IPvFuture */
      free((URI_CHAR *)uri->hostData.ipFuture.first);
      uri->hostData.ipFuture.first = NULL;
      uri->hostData.ipFuture.afterLast = NULL;
      uri->hostText.first = NULL;
      uri->hostText.afterLast = NULL;
    } else if ((uri->hostText.first != NULL)
        && (uri->hostData.ip4 == NULL)
        && (uri->hostData.ip6 == NULL)) {
      /* Regname */
      free((URI_CHAR *)uri->hostText.first);
      uri->hostText.first = NULL;
      uri->hostText.afterLast = NULL;
    }
  }

  /* NOTE: Port cannot happen! */

  if (revertMask & URI_NORMALIZE_PATH) {
    URI_TYPE(PathSegment) * walker = uri->pathHead;
    while (walker != NULL) {
      URI_TYPE(PathSegment) * const next = walker->next;
      if (walker->text.afterLast > walker->text.first) {
        free((URI_CHAR *)walker->text.first);
      }
      free(walker);
      walker = next;
    }
    uri->pathHead = NULL;
    uri->pathTail = NULL;
  }

  if (revertMask & URI_NORMALIZE_QUERY) {
    free((URI_CHAR *)uri->query.first);
    uri->query.first = NULL;
    uri->query.afterLast = NULL;
  }

  if (revertMask & URI_NORMALIZE_FRAGMENT) {
    free((URI_CHAR *)uri->fragment.first);
    uri->fragment.first = NULL;
    uri->fragment.afterLast = NULL;
  }
}



static URI_INLINE UriBool URI_FUNC(ContainsUppercaseLetters)(const URI_CHAR * first,
    const URI_CHAR * afterLast) {
  if ((first != NULL) && (afterLast != NULL) && (afterLast > first)) {
    const URI_CHAR * i = first;
    for (; i < afterLast; i++) {
      /* 6.2.2.1 Case Normalization: uppercase letters in scheme or host */
      if ((*i >= _UT('A')) && (*i <= _UT('Z'))) {
        return URI_TRUE;
      }
    }
  }
  return URI_FALSE;
}



static URI_INLINE UriBool URI_FUNC(ContainsUglyPercentEncoding)(const URI_CHAR * first,
    const URI_CHAR * afterLast) {
  if ((first != NULL) && (afterLast != NULL) && (afterLast > first)) {
    const URI_CHAR * i = first;
    for (; i + 2 < afterLast; i++) {
      if (i[0] == _UT('%')) {
        /* 6.2.2.1 Case Normalization: *
         * lowercase percent-encodings */
        if (((i[1] >= _UT('a')) && (i[1] <= _UT('f')))
            || ((i[2] >= _UT('a')) && (i[2] <= _UT('f')))) {
          return URI_TRUE;
        } else {
          /* 6.2.2.2 Percent-Encoding Normalization: *
           * percent-encoded unreserved characters   */
          const unsigned char left = URI_FUNC(HexdigToInt)(i[1]);
          const unsigned char right = URI_FUNC(HexdigToInt)(i[2]);
          const int code = 16 * left + right;
          if (uriIsUnreserved(code)) {
            return URI_TRUE;
          }
        }
      }
    }
  }
  return URI_FALSE;
}



static URI_INLINE void URI_FUNC(LowercaseInplace)(const URI_CHAR * first,
    const URI_CHAR * afterLast) {
  if ((first != NULL) && (afterLast != NULL) && (afterLast > first)) {
    URI_CHAR * i = (URI_CHAR *)first;
    const int lowerUpperDiff = (_UT('a') - _UT('A'));
    for (; i < afterLast; i++) {
      if ((*i >= _UT('A')) && (*i <=_UT('Z'))) {
        *i = (URI_CHAR)(*i + lowerUpperDiff);
      }
    }
  }
}



static URI_INLINE UriBool URI_FUNC(LowercaseMalloc)(const URI_CHAR ** first,
    const URI_CHAR ** afterLast) {
  int lenInChars;
  const int lowerUpperDiff = (_UT('a') - _UT('A'));
  URI_CHAR * buffer;
  int i = 0;

  if ((first == NULL) || (afterLast == NULL) || (*first == NULL)
      || (*afterLast == NULL)) {
    return URI_FALSE;
  }

  lenInChars = (int)(*afterLast - *first);
  if (lenInChars == 0) {
    return URI_TRUE;
  } else if (lenInChars < 0) {
    return URI_FALSE;
  }

  buffer = ( URI_CHAR* )malloc(lenInChars * sizeof(URI_CHAR));
  if (buffer == NULL) {
    return URI_FALSE;
  }

  for (; i < lenInChars; i++) {
    if (((*first)[i] >= _UT('A')) && ((*first)[i] <=_UT('Z'))) {
      buffer[i] = (URI_CHAR)((*first)[i] + lowerUpperDiff);
    } else {
      buffer[i] = (*first)[i];
    }
  }

  *first = buffer;
  *afterLast = buffer + lenInChars;
  return URI_TRUE;
}



/* NOTE: Implementation must stay inplace-compatible */
static URI_INLINE void URI_FUNC(FixPercentEncodingEngine)(
    const URI_CHAR * inFirst, const URI_CHAR * inAfterLast,
    const URI_CHAR * outFirst, const URI_CHAR ** outAfterLast) {
  URI_CHAR * write = (URI_CHAR *)outFirst;
  const int lenInChars = (int)(inAfterLast - inFirst);
  int i = 0;

  /* All but last two */
  for (; i + 2 < lenInChars; i++) {
    if (inFirst[i] != _UT('%')) {
      write[0] = inFirst[i];
      write++;
    } else {
      /* 6.2.2.2 Percent-Encoding Normalization: *
       * percent-encoded unreserved characters   */
      const URI_CHAR one = inFirst[i + 1];
      const URI_CHAR two = inFirst[i + 2];
      const unsigned char left = URI_FUNC(HexdigToInt)(one);
      const unsigned char right = URI_FUNC(HexdigToInt)(two);
      const int code = 16 * left + right;
      if (uriIsUnreserved(code)) {
        write[0] = (URI_CHAR)(code);
        write++;
      } else {
        /* 6.2.2.1 Case Normalization: *
         * lowercase percent-encodings */
        write[0] = _UT('%');
        write[1] = URI_FUNC(HexToLetter)(left);
        write[2] = URI_FUNC(HexToLetter)(right);
        write += 3;
      }

      i += 2; /* For the two chars of the percent group we just ate */
    }
  }

  /* Last two */
  for (; i < lenInChars; i++) {
    write[0] = inFirst[i];
    write++;
  }

  *outAfterLast = write;
}



static URI_INLINE void URI_FUNC(FixPercentEncodingInplace)(const URI_CHAR * first,
    const URI_CHAR ** afterLast) {
  /* Death checks */
  if ((first == NULL) || (afterLast == NULL) || (*afterLast == NULL)) {
    return;
  }

  /* Fix inplace */
  URI_FUNC(FixPercentEncodingEngine)(first, *afterLast, first, afterLast);
}



static URI_INLINE UriBool URI_FUNC(FixPercentEncodingMalloc)(const URI_CHAR ** first,
    const URI_CHAR ** afterLast) {
  int lenInChars;
  URI_CHAR * buffer;

  /* Death checks */
  if ((first == NULL) || (afterLast == NULL)
      || (*first == NULL) || (*afterLast == NULL)) {
    return URI_FALSE;
  }

  /* Old text length */
  lenInChars = (int)(*afterLast - *first);
  if (lenInChars == 0) {
    return URI_TRUE;
  } else if (lenInChars < 0) {
    return URI_FALSE;
  }

  /* New buffer */
  buffer = ( URI_CHAR* )malloc(lenInChars * sizeof(URI_CHAR));
  if (buffer == NULL) {
    return URI_FALSE;
  }

  /* Fix on copy */
  URI_FUNC(FixPercentEncodingEngine)(*first, *afterLast, buffer, afterLast);
  *first = buffer;
  return URI_TRUE;
}



static URI_INLINE UriBool URI_FUNC(MakeRangeOwner)(unsigned int * doneMask,
    unsigned int maskTest, URI_TYPE(TextRange) * range) {
  if (((*doneMask & maskTest) == 0)
      && (range->first != NULL)
      && (range->afterLast != NULL)
      && (range->afterLast > range->first)) {
    const int lenInChars = (int)(range->afterLast - range->first);
    const int lenInBytes = lenInChars * sizeof(URI_CHAR);
    URI_CHAR * dup = ( URI_CHAR* )malloc(lenInBytes);
    if (dup == NULL) {
      return URI_FALSE; /* Raises malloc error */
    }
    memcpy(dup, range->first, lenInBytes);
    range->first = dup;
    range->afterLast = dup + lenInChars;
    *doneMask |= maskTest;
  }
  return URI_TRUE;
}



static URI_INLINE UriBool URI_FUNC(MakeOwner)(URI_TYPE(Uri) * uri,
    unsigned int * doneMask) {
  URI_TYPE(PathSegment) * walker = uri->pathHead;
  if (!URI_FUNC(MakeRangeOwner)(doneMask, URI_NORMALIZE_SCHEME,
        &(uri->scheme))
      || !URI_FUNC(MakeRangeOwner)(doneMask, URI_NORMALIZE_USER_INFO,
        &(uri->userInfo))
      || !URI_FUNC(MakeRangeOwner)(doneMask, URI_NORMALIZE_QUERY,
        &(uri->query))
      || !URI_FUNC(MakeRangeOwner)(doneMask, URI_NORMALIZE_FRAGMENT,
        &(uri->fragment))) {
    return URI_FALSE; /* Raises malloc error */
  }

  /* Host */
  if ((*doneMask & URI_NORMALIZE_HOST) == 0) {
    if ((uri->hostData.ip4 == NULL)
        && (uri->hostData.ip6 == NULL)) {
      if (uri->hostData.ipFuture.first != NULL) {
        /* IPvFuture */
        if (!URI_FUNC(MakeRangeOwner)(doneMask, URI_NORMALIZE_HOST,
            &(uri->hostData.ipFuture))) {
          return URI_FALSE; /* Raises malloc error */
        }
        uri->hostText.first = uri->hostData.ipFuture.first;
        uri->hostText.afterLast = uri->hostData.ipFuture.afterLast;
      } else if (uri->hostText.first != NULL) {
        /* Regname */
        if (!URI_FUNC(MakeRangeOwner)(doneMask, URI_NORMALIZE_HOST,
            &(uri->hostText))) {
          return URI_FALSE; /* Raises malloc error */
        }
      }
    }
  }

  /* Path */
  if ((*doneMask & URI_NORMALIZE_PATH) == 0) {
    while (walker != NULL) {
      if (!URI_FUNC(MakeRangeOwner)(doneMask, 0, &(walker->text))) {
        /* Kill path to one before walker */
        URI_TYPE(PathSegment) * ranger = uri->pathHead;
        while (ranger->next != walker) {
          URI_TYPE(PathSegment) * const next = ranger->next;
          if ((ranger->text.first != NULL)
              && (ranger->text.afterLast != NULL)
              && (ranger->text.afterLast > ranger->text.first)) {
            free((URI_CHAR *)ranger->text.first);
            free(ranger);
          }
          ranger = next;
        }

        /* Kill path from walker */
        while (walker != NULL) {
          URI_TYPE(PathSegment) * const next = walker->next;
          free(walker);
          walker = next;
        }

        uri->pathHead = NULL;
        uri->pathTail = NULL;
        return URI_FALSE; /* Raises malloc error */
      }
      walker = walker->next;
    }
    *doneMask |= URI_NORMALIZE_PATH;
  }

  /* Port text, must come last so we don't have to undo that one if it fails. *
   * Otherwise we would need and extra enum flag for it although the port      *
   * cannot go unnormalized...                                                */
  if (!URI_FUNC(MakeRangeOwner)(doneMask, 0, &(uri->portText))) {
    return URI_FALSE; /* Raises malloc error */
  }

  return URI_TRUE;
}



unsigned int URI_FUNC(NormalizeSyntaxMaskRequired)(const URI_TYPE(Uri) * uri) {
  unsigned int res;
#if defined(__GNUC__) && ((__GNUC__ > 4) \
        || ((__GNUC__ == 4) && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ >= 2)))
    /* Slower code that fixes a warning, not sure if this is a smart idea */
  URI_TYPE(Uri) writeableClone;
  memcpy(&writeableClone, uri, 1 * sizeof(URI_TYPE(Uri)));
  URI_FUNC(NormalizeSyntaxEngine)(&writeableClone, 0, &res);
#else
  URI_FUNC(NormalizeSyntaxEngine)((URI_TYPE(Uri) *)uri, 0, &res);
#endif
  return res;
}



int URI_FUNC(NormalizeSyntaxEx)(URI_TYPE(Uri) * uri, unsigned int mask) {
  return URI_FUNC(NormalizeSyntaxEngine)(uri, mask, NULL);
}



int URI_FUNC(NormalizeSyntax)(URI_TYPE(Uri) * uri) {
  return URI_FUNC(NormalizeSyntaxEx)(uri, (unsigned int)-1);
}



static URI_INLINE int URI_FUNC(NormalizeSyntaxEngine)(URI_TYPE(Uri) * uri, unsigned int inMask, unsigned int * outMask) {
  unsigned int doneMask = URI_NORMALIZED;
  if (uri == NULL) {
    if (outMask != NULL) {
      *outMask = URI_NORMALIZED;
      return URI_SUCCESS;
    } else {
      return URI_ERROR_NULL;
    }
  }

  if (outMask != NULL) {
    /* Reset mask */
    *outMask = URI_NORMALIZED;
  } else if (inMask == URI_NORMALIZED) {
    /* Nothing to do */
    return URI_SUCCESS;
  }

  /* Scheme, host */
  if (outMask != NULL) {
    const UriBool normalizeScheme = URI_FUNC(ContainsUppercaseLetters)(
        uri->scheme.first, uri->scheme.afterLast);
    const UriBool normalizeHostCase = URI_FUNC(ContainsUppercaseLetters)(
      uri->hostText.first, uri->hostText.afterLast);
    if (normalizeScheme) {
      *outMask |= URI_NORMALIZE_SCHEME;
    }

    if (normalizeHostCase) {
      *outMask |= URI_NORMALIZE_HOST;
    } else {
      const UriBool normalizeHostPrecent = URI_FUNC(ContainsUglyPercentEncoding)(
          uri->hostText.first, uri->hostText.afterLast);
      if (normalizeHostPrecent) {
        *outMask |= URI_NORMALIZE_HOST;
      }
    }
  } else {
    /* Scheme */
    if ((inMask & URI_NORMALIZE_SCHEME) && (uri->scheme.first != NULL)) {
      if (uri->owner) {
        URI_FUNC(LowercaseInplace)(uri->scheme.first, uri->scheme.afterLast);
      } else {
        if (!URI_FUNC(LowercaseMalloc)(&(uri->scheme.first), &(uri->scheme.afterLast))) {
          URI_FUNC(PreventLeakage)(uri, doneMask);
          return URI_ERROR_MALLOC;
        }
        doneMask |= URI_NORMALIZE_SCHEME;
      }
    }

    /* Host */
    if (inMask & URI_NORMALIZE_HOST) {
      if (uri->hostData.ipFuture.first != NULL) {
        /* IPvFuture */
        if (uri->owner) {
          URI_FUNC(LowercaseInplace)(uri->hostData.ipFuture.first,
              uri->hostData.ipFuture.afterLast);
        } else {
          if (!URI_FUNC(LowercaseMalloc)(&(uri->hostData.ipFuture.first),
              &(uri->hostData.ipFuture.afterLast))) {
            URI_FUNC(PreventLeakage)(uri, doneMask);
            return URI_ERROR_MALLOC;
          }
          doneMask |= URI_NORMALIZE_HOST;
        }
        uri->hostText.first = uri->hostData.ipFuture.first;
        uri->hostText.afterLast = uri->hostData.ipFuture.afterLast;
      } else if ((uri->hostText.first != NULL)
          && (uri->hostData.ip4 == NULL)
          && (uri->hostData.ip6 == NULL)) {
        /* Regname */
        if (uri->owner) {
          URI_FUNC(FixPercentEncodingInplace)(uri->hostText.first,
              &(uri->hostText.afterLast));
        } else {
          if (!URI_FUNC(FixPercentEncodingMalloc)(
              &(uri->hostText.first),
              &(uri->hostText.afterLast))) {
            URI_FUNC(PreventLeakage)(uri, doneMask);
            return URI_ERROR_MALLOC;
          }
          doneMask |= URI_NORMALIZE_HOST;
        }

        URI_FUNC(LowercaseInplace)(uri->hostText.first,
            uri->hostText.afterLast);
      }
    }
  }

  /* User info */
  if (outMask != NULL) {
    const UriBool normalizeUserInfo = URI_FUNC(ContainsUglyPercentEncoding)(
      uri->userInfo.first, uri->userInfo.afterLast);
    if (normalizeUserInfo) {
      *outMask |= URI_NORMALIZE_USER_INFO;
    }
  } else {
    if ((inMask & URI_NORMALIZE_USER_INFO) && (uri->userInfo.first != NULL)) {
      if (uri->owner) {
        URI_FUNC(FixPercentEncodingInplace)(uri->userInfo.first, &(uri->userInfo.afterLast));
      } else {
        if (!URI_FUNC(FixPercentEncodingMalloc)(&(uri->userInfo.first),
            &(uri->userInfo.afterLast))) {
          URI_FUNC(PreventLeakage)(uri, doneMask);
          return URI_ERROR_MALLOC;
        }
        doneMask |= URI_NORMALIZE_USER_INFO;
      }
    }
  }

  /* Path */
  if (outMask != NULL) {
    const URI_TYPE(PathSegment) * walker = uri->pathHead;
    while (walker != NULL) {
      const URI_CHAR * const first = walker->text.first;
      const URI_CHAR * const afterLast = walker->text.afterLast;
      if ((first != NULL)
          && (afterLast != NULL)
          && (afterLast > first)
          && (
            (((afterLast - first) == 1)
              && (first[0] == _UT('.')))
            ||
            (((afterLast - first) == 2)
              && (first[0] == _UT('.'))
              && (first[1] == _UT('.')))
            ||
            URI_FUNC(ContainsUglyPercentEncoding)(first, afterLast)
          )) {
        *outMask |= URI_NORMALIZE_PATH;
        break;
      }
      walker = walker->next;
    }
  } else if (inMask & URI_NORMALIZE_PATH) {
    URI_TYPE(PathSegment) * walker;
    const UriBool relative = ((uri->scheme.first == NULL)
        && !uri->absolutePath) ? URI_TRUE : URI_FALSE;

    /* Fix percent-encoding for each segment */
    walker = uri->pathHead;
    if (uri->owner) {
      while (walker != NULL) {
        URI_FUNC(FixPercentEncodingInplace)(walker->text.first, &(walker->text.afterLast));
        walker = walker->next;
      }
    } else {
      while (walker != NULL) {
        if (!URI_FUNC(FixPercentEncodingMalloc)(&(walker->text.first),
            &(walker->text.afterLast))) {
          URI_FUNC(PreventLeakage)(uri, doneMask);
          return URI_ERROR_MALLOC;
        }
        walker = walker->next;
      }
      doneMask |= URI_NORMALIZE_PATH;
    }

    /* 6.2.2.3 Path Segment Normalization */
    if (!URI_FUNC(RemoveDotSegmentsEx)(uri, relative,
        (uri->owner == URI_TRUE)
        || ((doneMask & URI_NORMALIZE_PATH) != 0)
        )) {
      URI_FUNC(PreventLeakage)(uri, doneMask);
      return URI_ERROR_MALLOC;
    }
    URI_FUNC(FixEmptyTrailSegment)(uri);
  }

  /* Query, fragment */
  if (outMask != NULL) {
    const UriBool normalizeQuery = URI_FUNC(ContainsUglyPercentEncoding)(
        uri->query.first, uri->query.afterLast);
    const UriBool normalizeFragment = URI_FUNC(ContainsUglyPercentEncoding)(
        uri->fragment.first, uri->fragment.afterLast);
    if (normalizeQuery) {
      *outMask |= URI_NORMALIZE_QUERY;
    }

    if (normalizeFragment) {
      *outMask |= URI_NORMALIZE_FRAGMENT;
    }
  } else {
    /* Query */
    if ((inMask & URI_NORMALIZE_QUERY) && (uri->query.first != NULL)) {
      if (uri->owner) {
        URI_FUNC(FixPercentEncodingInplace)(uri->query.first, &(uri->query.afterLast));
      } else {
        if (!URI_FUNC(FixPercentEncodingMalloc)(&(uri->query.first),
            &(uri->query.afterLast))) {
          URI_FUNC(PreventLeakage)(uri, doneMask);
          return URI_ERROR_MALLOC;
        }
        doneMask |= URI_NORMALIZE_QUERY;
      }
    }

    /* Fragment */
    if ((inMask & URI_NORMALIZE_FRAGMENT) && (uri->fragment.first != NULL)) {
      if (uri->owner) {
        URI_FUNC(FixPercentEncodingInplace)(uri->fragment.first, &(uri->fragment.afterLast));
      } else {
        if (!URI_FUNC(FixPercentEncodingMalloc)(&(uri->fragment.first),
            &(uri->fragment.afterLast))) {
          URI_FUNC(PreventLeakage)(uri, doneMask);
          return URI_ERROR_MALLOC;
        }
        doneMask |= URI_NORMALIZE_FRAGMENT;
      }
    }
  }

  /* Dup all not duped yet */
  if ((outMask == NULL) && !uri->owner) {
    if (!URI_FUNC(MakeOwner)(uri, &doneMask)) {
      URI_FUNC(PreventLeakage)(uri, doneMask);
      return URI_ERROR_MALLOC;
    }
    uri->owner = URI_TRUE;
  }

  return URI_SUCCESS;
}


/* Source: UriParseBase.c */



void uriWriteQuadToDoubleByte(const unsigned char * hexDigits, int digitCount, unsigned char * output) {
  switch (digitCount) {
  case 1:
    /* 0x___? -> \x00 \x0? */
    output[0] = 0;
    output[1] = hexDigits[0];
    break;

  case 2:
    /* 0x__?? -> \0xx \x?? */
    output[0] = 0;
    output[1] = 16 * hexDigits[0] + hexDigits[1];
    break;

  case 3:
    /* 0x_??? -> \0x? \x?? */
    output[0] = hexDigits[0];
    output[1] = 16 * hexDigits[1] + hexDigits[2];
    break;

  case 4:
    /* 0x???? -> \0?? \x?? */
    output[0] = 16 * hexDigits[0] + hexDigits[1];
    output[1] = 16 * hexDigits[2] + hexDigits[3];
    break;

  }
}



unsigned char uriGetOctetValue(const unsigned char * digits, int digitCount) {
  switch (digitCount) {
  case 1:
    return digits[0];

  case 2:
    return 10 * digits[0] + digits[1];

  case 3:
  default:
    return 100 * digits[0] + 10 * digits[1] + digits[2];

  }
}


/* Source: UriParse.c */

#define URI_SET_DIGIT \
       _UT('0'): \
  case _UT('1'): \
  case _UT('2'): \
  case _UT('3'): \
  case _UT('4'): \
  case _UT('5'): \
  case _UT('6'): \
  case _UT('7'): \
  case _UT('8'): \
  case _UT('9')

#define URI_SET_HEX_LETTER_UPPER \
       _UT('A'): \
  case _UT('B'): \
  case _UT('C'): \
  case _UT('D'): \
  case _UT('E'): \
  case _UT('F')

#define URI_SET_HEX_LETTER_LOWER \
       _UT('a'): \
  case _UT('b'): \
  case _UT('c'): \
  case _UT('d'): \
  case _UT('e'): \
  case _UT('f')

#define URI_SET_HEXDIG \
  URI_SET_DIGIT: \
  case URI_SET_HEX_LETTER_UPPER: \
  case URI_SET_HEX_LETTER_LOWER

#define URI_SET_ALPHA \
  URI_SET_HEX_LETTER_UPPER: \
  case URI_SET_HEX_LETTER_LOWER: \
  case _UT('g'): \
  case _UT('G'): \
  case _UT('h'): \
  case _UT('H'): \
  case _UT('i'): \
  case _UT('I'): \
  case _UT('j'): \
  case _UT('J'): \
  case _UT('k'): \
  case _UT('K'): \
  case _UT('l'): \
  case _UT('L'): \
  case _UT('m'): \
  case _UT('M'): \
  case _UT('n'): \
  case _UT('N'): \
  case _UT('o'): \
  case _UT('O'): \
  case _UT('p'): \
  case _UT('P'): \
  case _UT('q'): \
  case _UT('Q'): \
  case _UT('r'): \
  case _UT('R'): \
  case _UT('s'): \
  case _UT('S'): \
  case _UT('t'): \
  case _UT('T'): \
  case _UT('u'): \
  case _UT('U'): \
  case _UT('v'): \
  case _UT('V'): \
  case _UT('w'): \
  case _UT('W'): \
  case _UT('x'): \
  case _UT('X'): \
  case _UT('y'): \
  case _UT('Y'): \
  case _UT('z'): \
  case _UT('Z')

static URI_INLINE void URI_FUNC(StopSyntax)(URI_TYPE(ParserState) * state,
    const URI_CHAR * errorPos) {
  URI_FUNC(FreeUriMembers)(state->uri);
  state->errorPos = errorPos;
  state->errorCode = URI_ERROR_SYNTAX;
}



static URI_INLINE void URI_FUNC(StopMalloc)(URI_TYPE(ParserState) * state) {
  URI_FUNC(FreeUriMembers)(state->uri);
  state->errorPos = NULL;
  state->errorCode = URI_ERROR_MALLOC;
}



/*
 * [authority]-><[>[ipLit2][authorityTwo]
 * [authority]->[ownHostUserInfoNz]
 * [authority]-><NULL>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseAuthority)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    /* "" regname host */
    state->uri->hostText.first = URI_FUNC(SafeToPointTo);
    state->uri->hostText.afterLast = URI_FUNC(SafeToPointTo);
    return afterLast;
  }

  switch (*first) {
  case _UT('['):
    {
      const URI_CHAR * const afterIpLit2
          = URI_FUNC(ParseIpLit2)(state, first + 1, afterLast);
      if (afterIpLit2 == NULL) {
        return NULL;
      }
      state->uri->hostText.first = first + 1; /* HOST BEGIN */
      return URI_FUNC(ParseAuthorityTwo)(state, afterIpLit2, afterLast);
    }

  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(':'):
  case _UT(';'):
  case _UT('@'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    state->uri->userInfo.first = first; /* USERINFO BEGIN */
    return URI_FUNC(ParseOwnHostUserInfoNz)(state, first, afterLast);

  default:
    /* "" regname host */
    state->uri->hostText.first = URI_FUNC(SafeToPointTo);
    state->uri->hostText.afterLast = URI_FUNC(SafeToPointTo);
    return first;
  }
}



/*
 * [authorityTwo]-><:>[port]
 * [authorityTwo]-><NULL>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseAuthorityTwo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT(':'):
    {
      const URI_CHAR * const afterPort = URI_FUNC(ParsePort)(state, first + 1, afterLast);
      if (afterPort == NULL) {
        return NULL;
      }
      state->uri->portText.first = first + 1; /* PORT BEGIN */
      state->uri->portText.afterLast = afterPort; /* PORT END */
      return afterPort;
    }

  default:
    return first;
  }
}



/*
 * [hexZero]->[HEXDIG][hexZero]
 * [hexZero]-><NULL>
 */
static const URI_CHAR * URI_FUNC(ParseHexZero)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case URI_SET_HEXDIG:
    return URI_FUNC(ParseHexZero)(state, first + 1, afterLast);

  default:
    return first;
  }
}



/*
 * [hierPart]->[pathRootless]
 * [hierPart]-></>[partHelperTwo]
 * [hierPart]-><NULL>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseHierPart)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(':'):
  case _UT(';'):
  case _UT('@'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    return URI_FUNC(ParsePathRootless)(state, first, afterLast);

  case _UT('/'):
    return URI_FUNC(ParsePartHelperTwo)(state, first + 1, afterLast);

  default:
    return first;
  }
}



/*
 * [ipFutLoop]->[subDelims][ipFutStopGo]
 * [ipFutLoop]->[unreserved][ipFutStopGo]
 * [ipFutLoop]-><:>[ipFutStopGo]
 */
static const URI_CHAR * URI_FUNC(ParseIpFutLoop)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(':'):
  case _UT(';'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    return URI_FUNC(ParseIpFutStopGo)(state, first + 1, afterLast);

  default:
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }
}



/*
 * [ipFutStopGo]->[ipFutLoop]
 * [ipFutStopGo]-><NULL>
 */
static const URI_CHAR * URI_FUNC(ParseIpFutStopGo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(':'):
  case _UT(';'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    return URI_FUNC(ParseIpFutLoop)(state, first, afterLast);

  default:
    return first;
  }
}



/*
 * [ipFuture]-><v>[HEXDIG][hexZero]<.>[ipFutLoop]
 */
static const URI_CHAR * URI_FUNC(ParseIpFuture)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }

  /*
  First character has already been
  checked before entering this rule.

  switch (*first) {
  case _UT('v'):
  */
    if (first + 1 >= afterLast) {
      URI_FUNC(StopSyntax)(state, first + 1);
      return NULL;
    }

    switch (first[1]) {
    case URI_SET_HEXDIG:
      {
        const URI_CHAR * afterIpFutLoop;
        const URI_CHAR * const afterHexZero
            = URI_FUNC(ParseHexZero)(state, first + 2, afterLast);
        if (afterHexZero == NULL) {
          return NULL;
        }
        if ((afterHexZero >= afterLast)
            || (*afterHexZero != _UT('.'))) {
          URI_FUNC(StopSyntax)(state, afterHexZero);
          return NULL;
        }
        state->uri->hostText.first = first; /* HOST BEGIN */
        state->uri->hostData.ipFuture.first = first; /* IPFUTURE BEGIN */
        afterIpFutLoop = URI_FUNC(ParseIpFutLoop)(state, afterHexZero + 1, afterLast);
        if (afterIpFutLoop == NULL) {
          return NULL;
        }
        state->uri->hostText.afterLast = afterIpFutLoop; /* HOST END */
        state->uri->hostData.ipFuture.afterLast = afterIpFutLoop; /* IPFUTURE END */
        return afterIpFutLoop;
      }

    default:
      URI_FUNC(StopSyntax)(state, first + 1);
      return NULL;
    }

  /*
  default:
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }
  */
}



/*
 * [ipLit2]->[ipFuture]<]>
 * [ipLit2]->[IPv6address2]
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseIpLit2)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }

  switch (*first) {
  case _UT('v'):
    {
      const URI_CHAR * const afterIpFuture
          = URI_FUNC(ParseIpFuture)(state, first, afterLast);
      if (afterIpFuture == NULL) {
        return NULL;
      }
      if ((afterIpFuture >= afterLast)
          || (*afterIpFuture != _UT(']'))) {
        URI_FUNC(StopSyntax)(state, first);
        return NULL;
      }
      return afterIpFuture + 1;
    }

  case _UT(':'):
  case _UT(']'):
  case URI_SET_HEXDIG:
    state->uri->hostData.ip6 = ( UriIp6* )malloc(1 * sizeof(UriIp6)); /* Freed when stopping on parse error */
    if (state->uri->hostData.ip6 == NULL) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return URI_FUNC(ParseIPv6address2)(state, first, afterLast);

  default:
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }
}



/*
 * [IPv6address2]->..<]>
 */
static const URI_CHAR * URI_FUNC(ParseIPv6address2)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  int zipperEver = 0;
  int quadsDone = 0;
  int digitCount = 0;
  unsigned char digitHistory[4];
  int ip4OctetsDone = 0;

  unsigned char quadsAfterZipper[14];
  int quadsAfterZipperCount = 0;


  for (;;) {
    if (first >= afterLast) {
      URI_FUNC(StopSyntax)(state, first);
      return NULL;
    }

    /* Inside IPv4 part? */
    if (ip4OctetsDone > 0) {
      /* Eat rest of IPv4 address */
      for (;;) {
        switch (*first) {
        case URI_SET_DIGIT:
          if (digitCount == 4) {
            URI_FUNC(StopSyntax)(state, first);
            return NULL;
          }
          digitHistory[digitCount++] = (unsigned char)(9 + *first - _UT('9'));
          break;

        case _UT('.'):
          if ((ip4OctetsDone == 4) /* NOTE! */
              || (digitCount == 0)
              || (digitCount == 4)) {
            /* Invalid digit or octet count */
            URI_FUNC(StopSyntax)(state, first);
            return NULL;
          } else if ((digitCount > 1)
              && (digitHistory[0] == 0)) {
            /* Leading zero */
            URI_FUNC(StopSyntax)(state, first - digitCount);
            return NULL;
          } else if ((digitCount > 2)
              && (digitHistory[1] == 0)) {
            /* Leading zero */
            URI_FUNC(StopSyntax)(state, first - digitCount + 1);
            return NULL;
          } else if ((digitCount == 3)
              && (100 * digitHistory[0]
                + 10 * digitHistory[1]
                + digitHistory[2] > 255)) {
            /* Octet value too large */
            if (digitHistory[0] > 2) {
              URI_FUNC(StopSyntax)(state, first - 3);
            } else if (digitHistory[1] > 5) {
              URI_FUNC(StopSyntax)(state, first - 2);
            } else {
              URI_FUNC(StopSyntax)(state, first - 1);
            }
            return NULL;
          }

          /* Copy IPv4 octet */
          state->uri->hostData.ip6->data[16 - 4 + ip4OctetsDone] = uriGetOctetValue(digitHistory, digitCount);
          digitCount = 0;
          ip4OctetsDone++;
          break;

        case _UT(']'):
          if ((ip4OctetsDone != 3) /* NOTE! */
              || (digitCount == 0)
              || (digitCount == 4)) {
            /* Invalid digit or octet count */
            URI_FUNC(StopSyntax)(state, first);
            return NULL;
          } else if ((digitCount > 1)
              && (digitHistory[0] == 0)) {
            /* Leading zero */
            URI_FUNC(StopSyntax)(state, first - digitCount);
            return NULL;
          } else if ((digitCount > 2)
              && (digitHistory[1] == 0)) {
            /* Leading zero */
            URI_FUNC(StopSyntax)(state, first - digitCount + 1);
            return NULL;
          } else if ((digitCount == 3)
              && (100 * digitHistory[0]
                + 10 * digitHistory[1]
                + digitHistory[2] > 255)) {
            /* Octet value too large */
            if (digitHistory[0] > 2) {
              URI_FUNC(StopSyntax)(state, first - 3);
            } else if (digitHistory[1] > 5) {
              URI_FUNC(StopSyntax)(state, first - 2);
            } else {
              URI_FUNC(StopSyntax)(state, first - 1);
            }
            return NULL;
          }

          state->uri->hostText.afterLast = first; /* HOST END */

          /* Copy missing quads right before IPv4 */
          memcpy(state->uri->hostData.ip6->data + 16 - 4 - 2 * quadsAfterZipperCount,
                quadsAfterZipper, 2 * quadsAfterZipperCount);

          /* Copy last IPv4 octet */
          state->uri->hostData.ip6->data[16 - 4 + 3] = uriGetOctetValue(digitHistory, digitCount);

          return first + 1;

        default:
          URI_FUNC(StopSyntax)(state, first);
          return NULL;
        }
        first++;
      }
    } else {
      /* Eat while no dot in sight */
      int letterAmong = 0;
      int walking = 1;
      do {
        switch (*first) {
        case URI_SET_HEX_LETTER_LOWER:
          letterAmong = 1;
          if (digitCount == 4) {
            URI_FUNC(StopSyntax)(state, first);
            return NULL;
          }
          digitHistory[digitCount] = (unsigned char)(15 + *first - _UT('f'));
          digitCount++;
          break;

        case URI_SET_HEX_LETTER_UPPER:
          letterAmong = 1;
          if (digitCount == 4) {
            URI_FUNC(StopSyntax)(state, first);
            return NULL;
          }
          digitHistory[digitCount] = (unsigned char)(15 + *first - _UT('F'));
          digitCount++;
          break;

        case URI_SET_DIGIT:
          if (digitCount == 4) {
            URI_FUNC(StopSyntax)(state, first);
            return NULL;
          }
          digitHistory[digitCount] = (unsigned char)(9 + *first - _UT('9'));
          digitCount++;
          break;

        case _UT(':'):
          {
            int setZipper = 0;

            /* Too many quads? */
            if (quadsDone > 8 - zipperEver) {
              URI_FUNC(StopSyntax)(state, first);
              return NULL;
            }

            /* "::"? */
            if (first + 1 >= afterLast) {
              URI_FUNC(StopSyntax)(state, first + 1);
              return NULL;
            }
            if (first[1] == _UT(':')) {
              const int resetOffset = 2 * (quadsDone + (digitCount > 0));

              first++;
              if (zipperEver) {
                URI_FUNC(StopSyntax)(state, first);
                return NULL; /* "::.+::" */
              }

              /* Zero everything after zipper */
              memset(state->uri->hostData.ip6->data + resetOffset, 0, 16 - resetOffset);
              setZipper = 1;

              /* ":::+"? */
              if (first + 1 >= afterLast) {
                URI_FUNC(StopSyntax)(state, first + 1);
                return NULL; /* No ']' yet */
              }
              if (first[1] == _UT(':')) {
                URI_FUNC(StopSyntax)(state, first + 1);
                return NULL; /* ":::+ "*/
              }
            }
            if (digitCount > 0) {
              if (zipperEver) {
                uriWriteQuadToDoubleByte(digitHistory, digitCount, quadsAfterZipper + 2 * quadsAfterZipperCount);
                quadsAfterZipperCount++;
              } else {
                uriWriteQuadToDoubleByte(digitHistory, digitCount, state->uri->hostData.ip6->data + 2 * quadsDone);
              }
              quadsDone++;
              digitCount = 0;
            }
            letterAmong = 0;

            if (setZipper) {
              zipperEver = 1;
            }
          }
          break;

        case _UT('.'):
          if ((quadsDone > 6) /* NOTE */
              || (!zipperEver && (quadsDone < 6))
              || letterAmong
              || (digitCount == 0)
              || (digitCount == 4)) {
            /* Invalid octet before */
            URI_FUNC(StopSyntax)(state, first);
            return NULL;
          } else if ((digitCount > 1)
              && (digitHistory[0] == 0)) {
            /* Leading zero */
            URI_FUNC(StopSyntax)(state, first - digitCount);
            return NULL;
          } else if ((digitCount > 2)
              && (digitHistory[1] == 0)) {
            /* Leading zero */
            URI_FUNC(StopSyntax)(state, first - digitCount + 1);
            return NULL;
          } else if ((digitCount == 3)
              && (100 * digitHistory[0]
                + 10 * digitHistory[1]
                + digitHistory[2] > 255)) {
            /* Octet value too large */
            if (digitHistory[0] > 2) {
              URI_FUNC(StopSyntax)(state, first - 3);
            } else if (digitHistory[1] > 5) {
              URI_FUNC(StopSyntax)(state, first - 2);
            } else {
              URI_FUNC(StopSyntax)(state, first - 1);
            }
            return NULL;
          }

          /* Copy first IPv4 octet */
          state->uri->hostData.ip6->data[16 - 4] = uriGetOctetValue(digitHistory, digitCount);
          digitCount = 0;

          /* Switch over to IPv4 loop */
          ip4OctetsDone = 1;
          walking = 0;
          break;

        case _UT(']'):
          /* Too little quads? */
          if (!zipperEver && !((quadsDone == 7) && (digitCount > 0))) {
            URI_FUNC(StopSyntax)(state, first);
            return NULL;
          }

          if (digitCount > 0) {
            if (zipperEver) {
              uriWriteQuadToDoubleByte(digitHistory, digitCount, quadsAfterZipper + 2 * quadsAfterZipperCount);
              quadsAfterZipperCount++;
            } else {
              uriWriteQuadToDoubleByte(digitHistory, digitCount, state->uri->hostData.ip6->data + 2 * quadsDone);
            }
            /*
            quadsDone++;
            digitCount = 0;
            */
          }

          /* Copy missing quads to the end */
          memcpy(state->uri->hostData.ip6->data + 16 - 2 * quadsAfterZipperCount,
                quadsAfterZipper, 2 * quadsAfterZipperCount);

          state->uri->hostText.afterLast = first; /* HOST END */
          return first + 1; /* Fine */

        default:
          URI_FUNC(StopSyntax)(state, first);
          return NULL;
        }
        first++;

        if (first >= afterLast) {
          URI_FUNC(StopSyntax)(state, first);
          return NULL; /* No ']' yet */
        }
      } while (walking);
    }
  }
}



/*
 * [mustBeSegmentNzNc]->[pctEncoded][mustBeSegmentNzNc]
 * [mustBeSegmentNzNc]->[subDelims][mustBeSegmentNzNc]
 * [mustBeSegmentNzNc]->[unreserved][mustBeSegmentNzNc]
 * [mustBeSegmentNzNc]->[uriTail] // can take <NULL>
 * [mustBeSegmentNzNc]-></>[segment][zeroMoreSlashSegs][uriTail]
 * [mustBeSegmentNzNc]-><@>[mustBeSegmentNzNc]
 */
static const URI_CHAR * URI_FUNC(ParseMustBeSegmentNzNc)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    if (!URI_FUNC(PushPathSegment)(state, state->uri->scheme.first, first)) { /* SEGMENT BOTH */
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    state->uri->scheme.first = NULL; /* Not a scheme, reset */
    return afterLast;
  }

  switch (*first) {
  case _UT('%'):
    {
      const URI_CHAR * const afterPctEncoded
          = URI_FUNC(ParsePctEncoded)(state, first, afterLast);
      if (afterPctEncoded == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseMustBeSegmentNzNc)(state, afterPctEncoded, afterLast);
    }

  case _UT('@'):
  case _UT('!'):
  case _UT('$'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('*'):
  case _UT(','):
  case _UT(';'):
  case _UT('\''):
  case _UT('+'):
  case _UT('='):
  case _UT('-'):
  case _UT('.'):
  case _UT('_'):
  case _UT('~'):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    return URI_FUNC(ParseMustBeSegmentNzNc)(state, first + 1, afterLast);

  case _UT('/'):
    {
      const URI_CHAR * afterZeroMoreSlashSegs;
      const URI_CHAR * afterSegment;
      if (!URI_FUNC(PushPathSegment)(state, state->uri->scheme.first, first)) { /* SEGMENT BOTH */
        URI_FUNC(StopMalloc)(state);
        return NULL;
      }
      state->uri->scheme.first = NULL; /* Not a scheme, reset */
      afterSegment = URI_FUNC(ParseSegment)(state, first + 1, afterLast);
      if (afterSegment == NULL) {
        return NULL;
      }
      if (!URI_FUNC(PushPathSegment)(state, first + 1, afterSegment)) { /* SEGMENT BOTH */
        URI_FUNC(StopMalloc)(state);
        return NULL;
      }
      afterZeroMoreSlashSegs
          = URI_FUNC(ParseZeroMoreSlashSegs)(state, afterSegment, afterLast);
      if (afterZeroMoreSlashSegs == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseUriTail)(state, afterZeroMoreSlashSegs, afterLast);
    }

  default:
    if (!URI_FUNC(PushPathSegment)(state, state->uri->scheme.first, first)) { /* SEGMENT BOTH */
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    state->uri->scheme.first = NULL; /* Not a scheme, reset */
    return URI_FUNC(ParseUriTail)(state, first, afterLast);
  }
}



/*
 * [ownHost]-><[>[ipLit2][authorityTwo]
 * [ownHost]->[ownHost2] // can take <NULL>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseOwnHost)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('['):
    {
      const URI_CHAR * const afterIpLit2
          = URI_FUNC(ParseIpLit2)(state, first + 1, afterLast);
      if (afterIpLit2 == NULL) {
        return NULL;
      }
      state->uri->hostText.first = first + 1; /* HOST BEGIN */
      return URI_FUNC(ParseAuthorityTwo)(state, afterIpLit2, afterLast);
    }

  default:
    return URI_FUNC(ParseOwnHost2)(state, first, afterLast);
  }
}



static URI_INLINE UriBool URI_FUNC(OnExitOwnHost2)(URI_TYPE(ParserState) * state, const URI_CHAR * first) {
  state->uri->hostText.afterLast = first; /* HOST END */

  /* Valid IPv4 or just a regname? */
  state->uri->hostData.ip4 = ( UriIp4* )malloc(1 * sizeof(UriIp4)); /* Freed when stopping on parse error */
  if (state->uri->hostData.ip4 == NULL) {
    return URI_FALSE; /* Raises malloc error */
  }
  if (URI_FUNC(ParseIpFourAddress)(state->uri->hostData.ip4->data,
      state->uri->hostText.first, state->uri->hostText.afterLast)) {
    /* Not IPv4 */
    free(state->uri->hostData.ip4);
    state->uri->hostData.ip4 = NULL;
  }
  return URI_TRUE; /* Success */
}



/*
 * [ownHost2]->[authorityTwo] // can take <NULL>
 * [ownHost2]->[pctSubUnres][ownHost2]
 */
static const URI_CHAR * URI_FUNC(ParseOwnHost2)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    if (!URI_FUNC(OnExitOwnHost2)(state, first)) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return afterLast;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(';'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    {
      const URI_CHAR * const afterPctSubUnres
          = URI_FUNC(ParsePctSubUnres)(state, first, afterLast);
      if (afterPctSubUnres == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseOwnHost2)(state, afterPctSubUnres, afterLast);
    }

  default:
    if (!URI_FUNC(OnExitOwnHost2)(state, first)) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return URI_FUNC(ParseAuthorityTwo)(state, first, afterLast);
  }
}



static URI_INLINE UriBool URI_FUNC(OnExitOwnHostUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first) {
  state->uri->hostText.first = state->uri->userInfo.first; /* Host instead of userInfo, update */
  state->uri->userInfo.first = NULL; /* Not a userInfo, reset */
  state->uri->hostText.afterLast = first; /* HOST END */

  /* Valid IPv4 or just a regname? */
  state->uri->hostData.ip4 = ( UriIp4* )malloc(1 * sizeof(UriIp4)); /* Freed when stopping on parse error */
  if (state->uri->hostData.ip4 == NULL) {
    return URI_FALSE; /* Raises malloc error */
  }
  if (URI_FUNC(ParseIpFourAddress)(state->uri->hostData.ip4->data,
      state->uri->hostText.first, state->uri->hostText.afterLast)) {
    /* Not IPv4 */
    free(state->uri->hostData.ip4);
    state->uri->hostData.ip4 = NULL;
  }
  return URI_TRUE; /* Success */
}



/*
 * [ownHostUserInfo]->[ownHostUserInfoNz]
 * [ownHostUserInfo]-><NULL>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseOwnHostUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    if (!URI_FUNC(OnExitOwnHostUserInfo)(state, first)) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return afterLast;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(':'):
  case _UT(';'):
  case _UT('@'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    return URI_FUNC(ParseOwnHostUserInfoNz)(state, first, afterLast);

  default:
    if (!URI_FUNC(OnExitOwnHostUserInfo)(state, first)) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return first;
  }
}



/*
 * [ownHostUserInfoNz]->[pctSubUnres][ownHostUserInfo]
 * [ownHostUserInfoNz]-><:>[ownPortUserInfo]
 * [ownHostUserInfoNz]-><@>[ownHost]
 */
static const URI_CHAR * URI_FUNC(ParseOwnHostUserInfoNz)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(';'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    {
      const URI_CHAR * const afterPctSubUnres
          = URI_FUNC(ParsePctSubUnres)(state, first, afterLast);
      if (afterPctSubUnres == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseOwnHostUserInfo)(state, afterPctSubUnres, afterLast);
    }

  case _UT(':'):
    state->uri->hostText.afterLast = first; /* HOST END */
    state->uri->portText.first = first + 1; /* PORT BEGIN */
    return URI_FUNC(ParseOwnPortUserInfo)(state, first + 1, afterLast);

  case _UT('@'):
    state->uri->userInfo.afterLast = first; /* USERINFO END */
    state->uri->hostText.first = first + 1; /* HOST BEGIN */
    return URI_FUNC(ParseOwnHost)(state, first + 1, afterLast);

  default:
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }
}



static URI_INLINE UriBool URI_FUNC(OnExitOwnPortUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first) {
  state->uri->hostText.first = state->uri->userInfo.first; /* Host instead of userInfo, update */
  state->uri->userInfo.first = NULL; /* Not a userInfo, reset */
  state->uri->portText.afterLast = first; /* PORT END */

  /* Valid IPv4 or just a regname? */
  state->uri->hostData.ip4 = ( UriIp4* )malloc(1 * sizeof(UriIp4)); /* Freed when stopping on parse error */
  if (state->uri->hostData.ip4 == NULL) {
    return URI_FALSE; /* Raises malloc error */
  }
  if (URI_FUNC(ParseIpFourAddress)(state->uri->hostData.ip4->data,
      state->uri->hostText.first, state->uri->hostText.afterLast)) {
    /* Not IPv4 */
    free(state->uri->hostData.ip4);
    state->uri->hostData.ip4 = NULL;
  }
  return URI_TRUE; /* Success */
}



/*
 * [ownPortUserInfo]->[ALPHA][ownUserInfo]
 * [ownPortUserInfo]->[DIGIT][ownPortUserInfo]
 * [ownPortUserInfo]-><.>[ownUserInfo]
 * [ownPortUserInfo]-><_>[ownUserInfo]
 * [ownPortUserInfo]-><~>[ownUserInfo]
 * [ownPortUserInfo]-><->[ownUserInfo]
 * [ownPortUserInfo]-><@>[ownHost]
 * [ownPortUserInfo]-><NULL>
 */
static const URI_CHAR * URI_FUNC(ParseOwnPortUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    if (!URI_FUNC(OnExitOwnPortUserInfo)(state, first)) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return afterLast;
  }

  switch (*first) {
  case _UT('.'):
  case _UT('_'):
  case _UT('~'):
  case _UT('-'):
  case URI_SET_ALPHA:
    state->uri->hostText.afterLast = NULL; /* Not a host, reset */
    state->uri->portText.first = NULL; /* Not a port, reset */
    return URI_FUNC(ParseOwnUserInfo)(state, first + 1, afterLast);

  case URI_SET_DIGIT:
    return URI_FUNC(ParseOwnPortUserInfo)(state, first + 1, afterLast);

  case _UT('@'):
    state->uri->hostText.afterLast = NULL; /* Not a host, reset */
    state->uri->portText.first = NULL; /* Not a port, reset */
    state->uri->userInfo.afterLast = first; /* USERINFO END */
    state->uri->hostText.first = first + 1; /* HOST BEGIN */
    return URI_FUNC(ParseOwnHost)(state, first + 1, afterLast);

  default:
    if (!URI_FUNC(OnExitOwnPortUserInfo)(state, first)) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return first;
  }
}



/*
 * [ownUserInfo]->[pctSubUnres][ownUserInfo]
 * [ownUserInfo]-><:>[ownUserInfo]
 * [ownUserInfo]-><@>[ownHost]
 */
static const URI_CHAR * URI_FUNC(ParseOwnUserInfo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(';'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    {
      const URI_CHAR * const afterPctSubUnres
          = URI_FUNC(ParsePctSubUnres)(state, first, afterLast);
      if (afterPctSubUnres == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseOwnUserInfo)(state, afterPctSubUnres, afterLast);
    }

  case _UT(':'):
    return URI_FUNC(ParseOwnUserInfo)(state, first + 1, afterLast);

  case _UT('@'):
    /* SURE */
    state->uri->userInfo.afterLast = first; /* USERINFO END */
    state->uri->hostText.first = first + 1; /* HOST BEGIN */
    return URI_FUNC(ParseOwnHost)(state, first + 1, afterLast);

  default:
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }
}



static URI_INLINE void URI_FUNC(OnExitPartHelperTwo)(URI_TYPE(ParserState) * state) {
  state->uri->absolutePath = URI_TRUE;
}



/*
 * [partHelperTwo]->[pathAbsNoLeadSlash] // can take <NULL>
 * [partHelperTwo]-></>[authority][pathAbsEmpty]
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParsePartHelperTwo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(OnExitPartHelperTwo)(state);
    return afterLast;
  }

  switch (*first) {
  case _UT('/'):
    {
      const URI_CHAR * const afterAuthority
          = URI_FUNC(ParseAuthority)(state, first + 1, afterLast);
      const URI_CHAR * afterPathAbsEmpty;
      if (afterAuthority == NULL) {
        return NULL;
      }
      afterPathAbsEmpty = URI_FUNC(ParsePathAbsEmpty)(state, afterAuthority, afterLast);

      URI_FUNC(FixEmptyTrailSegment)(state->uri);

      return afterPathAbsEmpty;
    }

  default:
    URI_FUNC(OnExitPartHelperTwo)(state);
    return URI_FUNC(ParsePathAbsNoLeadSlash)(state, first, afterLast);
  }
}



/*
 * [pathAbsEmpty]-></>[segment][pathAbsEmpty]
 * [pathAbsEmpty]-><NULL>
 */
static const URI_CHAR * URI_FUNC(ParsePathAbsEmpty)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('/'):
    {
      const URI_CHAR * const afterSegment
          = URI_FUNC(ParseSegment)(state, first + 1, afterLast);
      if (afterSegment == NULL) {
        return NULL;
      }
      if (!URI_FUNC(PushPathSegment)(state, first + 1, afterSegment)) { /* SEGMENT BOTH */
        URI_FUNC(StopMalloc)(state);
        return NULL;
      }
      return URI_FUNC(ParsePathAbsEmpty)(state, afterSegment, afterLast);
    }

  default:
    return first;
  }
}



/*
 * [pathAbsNoLeadSlash]->[segmentNz][zeroMoreSlashSegs]
 * [pathAbsNoLeadSlash]-><NULL>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParsePathAbsNoLeadSlash)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(':'):
  case _UT(';'):
  case _UT('@'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    {
      const URI_CHAR * const afterSegmentNz
          = URI_FUNC(ParseSegmentNz)(state, first, afterLast);
      if (afterSegmentNz == NULL) {
        return NULL;
      }
      if (!URI_FUNC(PushPathSegment)(state, first, afterSegmentNz)) { /* SEGMENT BOTH */
        URI_FUNC(StopMalloc)(state);
        return NULL;
      }
      return URI_FUNC(ParseZeroMoreSlashSegs)(state, afterSegmentNz, afterLast);
    }

  default:
    return first;
  }
}



/*
 * [pathRootless]->[segmentNz][zeroMoreSlashSegs]
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParsePathRootless)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  const URI_CHAR * const afterSegmentNz
      = URI_FUNC(ParseSegmentNz)(state, first, afterLast);
  if (afterSegmentNz == NULL) {
    return NULL;
  } else {
    if (!URI_FUNC(PushPathSegment)(state, first, afterSegmentNz)) { /* SEGMENT BOTH */
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
  }
  return URI_FUNC(ParseZeroMoreSlashSegs)(state, afterSegmentNz, afterLast);
}



/*
 * [pchar]->[pctEncoded]
 * [pchar]->[subDelims]
 * [pchar]->[unreserved]
 * [pchar]-><:>
 * [pchar]-><@>
 */
static const URI_CHAR * URI_FUNC(ParsePchar)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }

  switch (*first) {
  case _UT('%'):
    return URI_FUNC(ParsePctEncoded)(state, first, afterLast);

  case _UT(':'):
  case _UT('@'):
  case _UT('!'):
  case _UT('$'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('*'):
  case _UT(','):
  case _UT(';'):
  case _UT('\''):
  case _UT('+'):
  case _UT('='):
  case _UT('-'):
  case _UT('.'):
  case _UT('_'):
  case _UT('~'):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    return first + 1;

  default:
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }
}



/*
 * [pctEncoded]-><%>[HEXDIG][HEXDIG]
 */
static const URI_CHAR * URI_FUNC(ParsePctEncoded)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }

  /*
  First character has already been
  checked before entering this rule.

  switch (*first) {
  case _UT('%'):
  */
    if (first + 1 >= afterLast) {
      URI_FUNC(StopSyntax)(state, first + 1);
      return NULL;
    }

    switch (first[1]) {
    case URI_SET_HEXDIG:
      if (first + 2 >= afterLast) {
        URI_FUNC(StopSyntax)(state, first + 2);
        return NULL;
      }

      switch (first[2]) {
      case URI_SET_HEXDIG:
        return first + 3;

      default:
        URI_FUNC(StopSyntax)(state, first + 2);
        return NULL;
      }

    default:
      URI_FUNC(StopSyntax)(state, first + 1);
      return NULL;
    }

  /*
  default:
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }
  */
}



/*
 * [pctSubUnres]->[pctEncoded]
 * [pctSubUnres]->[subDelims]
 * [pctSubUnres]->[unreserved]
 */
static const URI_CHAR * URI_FUNC(ParsePctSubUnres)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }

  switch (*first) {
  case _UT('%'):
    return URI_FUNC(ParsePctEncoded)(state, first, afterLast);

  case _UT('!'):
  case _UT('$'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('*'):
  case _UT(','):
  case _UT(';'):
  case _UT('\''):
  case _UT('+'):
  case _UT('='):
  case _UT('-'):
  case _UT('.'):
  case _UT('_'):
  case _UT('~'):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    return first + 1;

  default:
    URI_FUNC(StopSyntax)(state, first);
    return NULL;
  }
}



/*
 * [port]->[DIGIT][port]
 * [port]-><NULL>
 */
static const URI_CHAR * URI_FUNC(ParsePort)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case URI_SET_DIGIT:
    return URI_FUNC(ParsePort)(state, first + 1, afterLast);

  default:
    return first;
  }
}



/*
 * [queryFrag]->[pchar][queryFrag]
 * [queryFrag]-></>[queryFrag]
 * [queryFrag]-><?>[queryFrag]
 * [queryFrag]-><NULL>
 */
static const URI_CHAR * URI_FUNC(ParseQueryFrag)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(':'):
  case _UT(';'):
  case _UT('@'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    {
      const URI_CHAR * const afterPchar
          = URI_FUNC(ParsePchar)(state, first, afterLast);
      if (afterPchar == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseQueryFrag)(state, afterPchar, afterLast);
    }

  case _UT('/'):
  case _UT('?'):
    return URI_FUNC(ParseQueryFrag)(state, first + 1, afterLast);

  default:
    return first;
  }
}



/*
 * [segment]->[pchar][segment]
 * [segment]-><NULL>
 */
static const URI_CHAR * URI_FUNC(ParseSegment)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('!'):
  case _UT('$'):
  case _UT('%'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('-'):
  case _UT('*'):
  case _UT(','):
  case _UT('.'):
  case _UT(':'):
  case _UT(';'):
  case _UT('@'):
  case _UT('\''):
  case _UT('_'):
  case _UT('~'):
  case _UT('+'):
  case _UT('='):
  case URI_SET_DIGIT:
  case URI_SET_ALPHA:
    {
      const URI_CHAR * const afterPchar
          = URI_FUNC(ParsePchar)(state, first, afterLast);
      if (afterPchar == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseSegment)(state, afterPchar, afterLast);
    }

  default:
    return first;
  }
}



/*
 * [segmentNz]->[pchar][segment]
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseSegmentNz)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  const URI_CHAR * const afterPchar
      = URI_FUNC(ParsePchar)(state, first, afterLast);
  if (afterPchar == NULL) {
    return NULL;
  }
  return URI_FUNC(ParseSegment)(state, afterPchar, afterLast);
}



static URI_INLINE UriBool URI_FUNC(OnExitSegmentNzNcOrScheme2)(URI_TYPE(ParserState) * state, const URI_CHAR * first) {
  if (!URI_FUNC(PushPathSegment)(state, state->uri->scheme.first, first)) { /* SEGMENT BOTH */
    return URI_FALSE; /* Raises malloc error*/
  }
  state->uri->scheme.first = NULL; /* Not a scheme, reset */
  return URI_TRUE; /* Success */
}



/*
 * [segmentNzNcOrScheme2]->[ALPHA][segmentNzNcOrScheme2]
 * [segmentNzNcOrScheme2]->[DIGIT][segmentNzNcOrScheme2]
 * [segmentNzNcOrScheme2]->[pctEncoded][mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]->[uriTail] // can take <NULL>
 * [segmentNzNcOrScheme2]-><!>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><$>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><&>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><(>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><)>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><*>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><,>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><.>[segmentNzNcOrScheme2]
 * [segmentNzNcOrScheme2]-></>[segment][zeroMoreSlashSegs][uriTail]
 * [segmentNzNcOrScheme2]-><:>[hierPart][uriTail]
 * [segmentNzNcOrScheme2]-><;>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><@>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><_>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><~>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><+>[segmentNzNcOrScheme2]
 * [segmentNzNcOrScheme2]-><=>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><'>[mustBeSegmentNzNc]
 * [segmentNzNcOrScheme2]-><->[segmentNzNcOrScheme2]
 */
static const URI_CHAR * URI_FUNC(ParseSegmentNzNcOrScheme2)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    if (!URI_FUNC(OnExitSegmentNzNcOrScheme2)(state, first)) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return afterLast;
  }

  switch (*first) {
  case _UT('.'):
  case _UT('+'):
  case _UT('-'):
  case URI_SET_ALPHA:
  case URI_SET_DIGIT:
    return URI_FUNC(ParseSegmentNzNcOrScheme2)(state, first + 1, afterLast);

  case _UT('%'):
    {
      const URI_CHAR * const afterPctEncoded
          = URI_FUNC(ParsePctEncoded)(state, first, afterLast);
      if (afterPctEncoded == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseMustBeSegmentNzNc)(state, afterPctEncoded, afterLast);
    }

  case _UT('!'):
  case _UT('$'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('*'):
  case _UT(','):
  case _UT(';'):
  case _UT('@'):
  case _UT('_'):
  case _UT('~'):
  case _UT('='):
  case _UT('\''):
    return URI_FUNC(ParseMustBeSegmentNzNc)(state, first + 1, afterLast);

  case _UT('/'):
    {
      const URI_CHAR * afterZeroMoreSlashSegs;
      const URI_CHAR * const afterSegment
          = URI_FUNC(ParseSegment)(state, first + 1, afterLast);
      if (afterSegment == NULL) {
        return NULL;
      }
      if (!URI_FUNC(PushPathSegment)(state, state->uri->scheme.first, first)) { /* SEGMENT BOTH */
        URI_FUNC(StopMalloc)(state);
        return NULL;
      }
      state->uri->scheme.first = NULL; /* Not a scheme, reset */
      if (!URI_FUNC(PushPathSegment)(state, first + 1, afterSegment)) { /* SEGMENT BOTH */
        URI_FUNC(StopMalloc)(state);
        return NULL;
      }
      afterZeroMoreSlashSegs
          = URI_FUNC(ParseZeroMoreSlashSegs)(state, afterSegment, afterLast);
      if (afterZeroMoreSlashSegs == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseUriTail)(state, afterZeroMoreSlashSegs, afterLast);
    }

  case _UT(':'):
    {
      const URI_CHAR * const afterHierPart
          = URI_FUNC(ParseHierPart)(state, first + 1, afterLast);
      state->uri->scheme.afterLast = first; /* SCHEME END */
      if (afterHierPart == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseUriTail)(state, afterHierPart, afterLast);
    }

  default:
    if (!URI_FUNC(OnExitSegmentNzNcOrScheme2)(state, first)) {
      URI_FUNC(StopMalloc)(state);
      return NULL;
    }
    return URI_FUNC(ParseUriTail)(state, first, afterLast);
  }
}



/*
 * [uriReference]->[ALPHA][segmentNzNcOrScheme2]
 * [uriReference]->[DIGIT][mustBeSegmentNzNc]
 * [uriReference]->[pctEncoded][mustBeSegmentNzNc]
 * [uriReference]->[subDelims][mustBeSegmentNzNc]
 * [uriReference]->[uriTail] // can take <NULL>
 * [uriReference]-><.>[mustBeSegmentNzNc]
 * [uriReference]-></>[partHelperTwo][uriTail]
 * [uriReference]-><@>[mustBeSegmentNzNc]
 * [uriReference]-><_>[mustBeSegmentNzNc]
 * [uriReference]-><~>[mustBeSegmentNzNc]
 * [uriReference]-><->[mustBeSegmentNzNc]
 */
static const URI_CHAR * URI_FUNC(ParseUriReference)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case URI_SET_ALPHA:
    state->uri->scheme.first = first; /* SCHEME BEGIN */
    return URI_FUNC(ParseSegmentNzNcOrScheme2)(state, first + 1, afterLast);

  case URI_SET_DIGIT:
  case _UT('!'):
  case _UT('$'):
  case _UT('&'):
  case _UT('('):
  case _UT(')'):
  case _UT('*'):
  case _UT(','):
  case _UT(';'):
  case _UT('\''):
  case _UT('+'):
  case _UT('='):
  case _UT('.'):
  case _UT('_'):
  case _UT('~'):
  case _UT('-'):
  case _UT('@'):
    state->uri->scheme.first = first; /* SEGMENT BEGIN, ABUSE SCHEME POINTER */
    return URI_FUNC(ParseMustBeSegmentNzNc)(state, first + 1, afterLast);

  case _UT('%'):
    {
      const URI_CHAR * const afterPctEncoded
          = URI_FUNC(ParsePctEncoded)(state, first, afterLast);
      if (afterPctEncoded == NULL) {
        return NULL;
      }
      state->uri->scheme.first = first; /* SEGMENT BEGIN, ABUSE SCHEME POINTER */
      return URI_FUNC(ParseMustBeSegmentNzNc)(state, afterPctEncoded, afterLast);
    }

  case _UT('/'):
    {
      const URI_CHAR * const afterPartHelperTwo
          = URI_FUNC(ParsePartHelperTwo)(state, first + 1, afterLast);
      if (afterPartHelperTwo == NULL) {
        return NULL;
      }
      return URI_FUNC(ParseUriTail)(state, afterPartHelperTwo, afterLast);
    }

  default:
    return URI_FUNC(ParseUriTail)(state, first, afterLast);
  }
}



/*
 * [uriTail]-><#>[queryFrag]
 * [uriTail]-><?>[queryFrag][uriTailTwo]
 * [uriTail]-><NULL>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseUriTail)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('#'):
    {
      const URI_CHAR * const afterQueryFrag = URI_FUNC(ParseQueryFrag)(state, first + 1, afterLast);
      if (afterQueryFrag == NULL) {
        return NULL;
      }
      state->uri->fragment.first = first + 1; /* FRAGMENT BEGIN */
      state->uri->fragment.afterLast = afterQueryFrag; /* FRAGMENT END */
      return afterQueryFrag;
    }

  case _UT('?'):
    {
      const URI_CHAR * const afterQueryFrag
          = URI_FUNC(ParseQueryFrag)(state, first + 1, afterLast);
      if (afterQueryFrag == NULL) {
        return NULL;
      }
      state->uri->query.first = first + 1; /* QUERY BEGIN */
      state->uri->query.afterLast = afterQueryFrag; /* QUERY END */
      return URI_FUNC(ParseUriTailTwo)(state, afterQueryFrag, afterLast);
    }

  default:
    return first;
  }
}



/*
 * [uriTailTwo]-><#>[queryFrag]
 * [uriTailTwo]-><NULL>
 */
static URI_INLINE const URI_CHAR * URI_FUNC(ParseUriTailTwo)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('#'):
    {
      const URI_CHAR * const afterQueryFrag = URI_FUNC(ParseQueryFrag)(state, first + 1, afterLast);
      if (afterQueryFrag == NULL) {
        return NULL;
      }
      state->uri->fragment.first = first + 1; /* FRAGMENT BEGIN */
      state->uri->fragment.afterLast = afterQueryFrag; /* FRAGMENT END */
      return afterQueryFrag;
    }

  default:
    return first;
  }
}



/*
 * [zeroMoreSlashSegs]-></>[segment][zeroMoreSlashSegs]
 * [zeroMoreSlashSegs]-><NULL>
 */
static const URI_CHAR * URI_FUNC(ParseZeroMoreSlashSegs)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  if (first >= afterLast) {
    return afterLast;
  }

  switch (*first) {
  case _UT('/'):
    {
      const URI_CHAR * const afterSegment
          = URI_FUNC(ParseSegment)(state, first + 1, afterLast);
      if (afterSegment == NULL) {
        return NULL;
      }
      if (!URI_FUNC(PushPathSegment)(state, first + 1, afterSegment)) { /* SEGMENT BOTH */
        URI_FUNC(StopMalloc)(state);
        return NULL;
      }
      return URI_FUNC(ParseZeroMoreSlashSegs)(state, afterSegment, afterLast);
    }

  default:
    return first;
  }
}



static URI_INLINE void URI_FUNC(ResetParserState)(URI_TYPE(ParserState) * state) {
  URI_TYPE(Uri) * const uriBackup = state->uri;
  memset(state, 0, sizeof(URI_TYPE(ParserState)));
  state->uri = uriBackup;
}



static URI_INLINE UriBool URI_FUNC(PushPathSegment)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  URI_TYPE(PathSegment) * segment = ( URI_TYPE(PathSegment) * )malloc(1 * sizeof(URI_TYPE(PathSegment)));
  if (segment == NULL) {
    return URI_FALSE; /* Raises malloc error */
  }
  memset(segment, 0, sizeof(URI_TYPE(PathSegment)));
  if (first == afterLast) {
    segment->text.first = URI_FUNC(SafeToPointTo);
    segment->text.afterLast = URI_FUNC(SafeToPointTo);
  } else {
    segment->text.first = first;
    segment->text.afterLast = afterLast;
  }

  /* First segment ever? */
  if (state->uri->pathHead == NULL) {
    /* First segement ever, set head and tail */
    state->uri->pathHead = segment;
    state->uri->pathTail = segment;
  } else {
    /* Append, update tail */
    state->uri->pathTail->next = segment;
    state->uri->pathTail = segment;
  }

  return URI_TRUE; /* Success */
}



int URI_FUNC(ParseUriEx)(URI_TYPE(ParserState) * state, const URI_CHAR * first, const URI_CHAR * afterLast) {
  const URI_CHAR * afterUriReference;
  URI_TYPE(Uri) * uri;

  /* Check params */
  if ((state == NULL) || (first == NULL) || (afterLast == NULL)) {
    return URI_ERROR_NULL;
  }
  uri = state->uri;

  /* Init parser */
  URI_FUNC(ResetParserState)(state);
  URI_FUNC(ResetUri)(uri);

  /* Parse */
  afterUriReference = URI_FUNC(ParseUriReference)(state, first, afterLast);
  if (afterUriReference == NULL) {
    return state->errorCode;
  }
  if (afterUriReference != afterLast) {
    return URI_ERROR_SYNTAX;
  }
  return URI_SUCCESS;
}



int URI_FUNC(ParseUri)(URI_TYPE(ParserState) * state, const URI_CHAR * text) {
  if ((state == NULL) || (text == NULL)) {
    return URI_ERROR_NULL;
  }
  return URI_FUNC(ParseUriEx)(state, text, text + URI_STRLEN(text));
}



void URI_FUNC(FreeUriMembers)(URI_TYPE(Uri) * uri) {
  if (uri == NULL) {
    return;
  }

  if (uri->owner) {
    /* Scheme */
    if (uri->scheme.first != NULL) {
      if (uri->scheme.first != uri->scheme.afterLast) {
        free((URI_CHAR *)uri->scheme.first);
      }
      uri->scheme.first = NULL;
      uri->scheme.afterLast = NULL;
    }

    /* User info */
    if (uri->userInfo.first != NULL) {
      if (uri->userInfo.first != uri->userInfo.afterLast) {
        free((URI_CHAR *)uri->userInfo.first);
      }
      uri->userInfo.first = NULL;
      uri->userInfo.afterLast = NULL;
    }

    /* Host data - IPvFuture */
    if (uri->hostData.ipFuture.first != NULL) {
      if (uri->hostData.ipFuture.first != uri->hostData.ipFuture.afterLast) {
        free((URI_CHAR *)uri->hostData.ipFuture.first);
      }
      uri->hostData.ipFuture.first = NULL;
      uri->hostData.ipFuture.afterLast = NULL;
      uri->hostText.first = NULL;
      uri->hostText.afterLast = NULL;
    }

    /* Host text (if regname, after IPvFuture!) */
    if ((uri->hostText.first != NULL)
        && (uri->hostData.ip4 == NULL)
        && (uri->hostData.ip6 == NULL)) {
      /* Real regname */
      if (uri->hostText.first != uri->hostText.afterLast) {
        free((URI_CHAR *)uri->hostText.first);
      }
      uri->hostText.first = NULL;
      uri->hostText.afterLast = NULL;
    }
  }

  /* Host data - IPv4 */
  if (uri->hostData.ip4 != NULL) {
    free(uri->hostData.ip4);
    uri->hostData.ip4 = NULL;
  }

  /* Host data - IPv6 */
  if (uri->hostData.ip6 != NULL) {
    free(uri->hostData.ip6);
    uri->hostData.ip6 = NULL;
  }

  /* Port text */
  if (uri->owner && (uri->portText.first != NULL)) {
    if (uri->portText.first != uri->portText.afterLast) {
      free((URI_CHAR *)uri->portText.first);
    }
    uri->portText.first = NULL;
    uri->portText.afterLast = NULL;
  }

  /* Path */
  if (uri->pathHead != NULL) {
    URI_TYPE(PathSegment) * segWalk = uri->pathHead;
    while (segWalk != NULL) {
      URI_TYPE(PathSegment) * const next = segWalk->next;
      if (uri->owner && (segWalk->text.first != NULL)
          && (segWalk->text.first < segWalk->text.afterLast)) {
        free((URI_CHAR *)segWalk->text.first);
      }
      free(segWalk);
      segWalk = next;
    }
    uri->pathHead = NULL;
    uri->pathTail = NULL;
  }

  if (uri->owner) {
    /* Query */
    if (uri->query.first != NULL) {
      if (uri->query.first != uri->query.afterLast) {
        free((URI_CHAR *)uri->query.first);
      }
      uri->query.first = NULL;
      uri->query.afterLast = NULL;
    }

    /* Fragment */
    if (uri->fragment.first != NULL) {
      if (uri->fragment.first != uri->fragment.afterLast) {
        free((URI_CHAR *)uri->fragment.first);
      }
      uri->fragment.first = NULL;
      uri->fragment.afterLast = NULL;
    }
  }
}



UriBool URI_FUNC(_TESTING_ONLY_ParseIpSix)(const URI_CHAR * text) {
  URI_TYPE(Uri) uri;
  URI_TYPE(ParserState) parser;
  const URI_CHAR * const afterIpSix = text + URI_STRLEN(text);
  const URI_CHAR * res;

  URI_FUNC(ResetParserState)(&parser);
  URI_FUNC(ResetUri)(&uri);
  parser.uri = &uri;
  parser.uri->hostData.ip6 = ( UriIp6* )malloc(1 * sizeof(UriIp6));
  res = URI_FUNC(ParseIPv6address2)(&parser, text, afterIpSix);
  URI_FUNC(FreeUriMembers)(&uri);
  return res == afterIpSix ? URI_TRUE : URI_FALSE;
}



UriBool URI_FUNC(_TESTING_ONLY_ParseIpFour)(const URI_CHAR * text) {
  unsigned char octets[4];
  int res = URI_FUNC(ParseIpFourAddress)(octets, text, text + URI_STRLEN(text));
  return (res == URI_SUCCESS) ? URI_TRUE : URI_FALSE;
}



#undef URI_SET_DIGIT
#undef URI_SET_HEX_LETTER_UPPER
#undef URI_SET_HEX_LETTER_LOWER
#undef URI_SET_HEXDIG
#undef URI_SET_ALPHA


/* Source: UriQuery.c */
static int URI_FUNC(ComposeQueryEngine)(URI_CHAR * dest,
    const URI_TYPE(QueryList) * queryList,
    int maxChars, int * charsWritten, int * charsRequired,
    UriBool spaceToPlus, UriBool normalizeBreaks);

static UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** prevNext,
    int * itemCount, const URI_CHAR * keyFirst, const URI_CHAR * keyAfter,
    const URI_CHAR * valueFirst, const URI_CHAR * valueAfter,
    UriBool plusToSpace, UriBreakConversion breakConversion);



int URI_FUNC(ComposeQueryCharsRequired)(const URI_TYPE(QueryList) * queryList,
    int * charsRequired) {
  const UriBool spaceToPlus = URI_TRUE;
  const UriBool normalizeBreaks = URI_TRUE;

  return URI_FUNC(ComposeQueryCharsRequiredEx)(queryList, charsRequired,
      spaceToPlus, normalizeBreaks);
}



int URI_FUNC(ComposeQueryCharsRequiredEx)(const URI_TYPE(QueryList) * queryList,
    int * charsRequired, UriBool spaceToPlus, UriBool normalizeBreaks) {
  if ((queryList == NULL) || (charsRequired == NULL)) {
    return URI_ERROR_NULL;
  }

  return URI_FUNC(ComposeQueryEngine)(NULL, queryList, 0, NULL,
      charsRequired, spaceToPlus, normalizeBreaks);
}



int URI_FUNC(ComposeQuery)(URI_CHAR * dest,
               const URI_TYPE(QueryList) * queryList, int maxChars, int * charsWritten) {
  const UriBool spaceToPlus = URI_TRUE;
  const UriBool normalizeBreaks = URI_TRUE;

  return URI_FUNC(ComposeQueryEx)(dest, queryList, maxChars, charsWritten,
      spaceToPlus, normalizeBreaks);
}



int URI_FUNC(ComposeQueryEx)(URI_CHAR * dest,
    const URI_TYPE(QueryList) * queryList, int maxChars, int * charsWritten,
    UriBool spaceToPlus, UriBool normalizeBreaks) {
  if ((dest == NULL) || (queryList == NULL)) {
    return URI_ERROR_NULL;
  }

  if (maxChars < 1) {
    return URI_ERROR_OUTPUT_TOO_LARGE;
  }

  return URI_FUNC(ComposeQueryEngine)(dest, queryList, maxChars,
      charsWritten, NULL, spaceToPlus, normalizeBreaks);
}



int URI_FUNC(ComposeQueryMalloc)(URI_CHAR ** dest,
    const URI_TYPE(QueryList) * queryList) {
  const UriBool spaceToPlus = URI_TRUE;
  const UriBool normalizeBreaks = URI_TRUE;

  return URI_FUNC(ComposeQueryMallocEx)(dest, queryList,
      spaceToPlus, normalizeBreaks);
}



int URI_FUNC(ComposeQueryMallocEx)(URI_CHAR ** dest,
    const URI_TYPE(QueryList) * queryList,
    UriBool spaceToPlus, UriBool normalizeBreaks) {
  int charsRequired;
  int res;
  URI_CHAR * queryString;

  if (dest == NULL) {
    return URI_ERROR_NULL;
  }

  /* Calculate space */
  res = URI_FUNC(ComposeQueryCharsRequiredEx)(queryList, &charsRequired,
      spaceToPlus, normalizeBreaks);
  if (res != URI_SUCCESS) {
    return res;
  }
  charsRequired++;

  /* Allocate space */
  queryString = ( URI_CHAR* )malloc(charsRequired * sizeof(URI_CHAR));
  if (queryString == NULL) {
    return URI_ERROR_MALLOC;
  }

  /* Put query in */
  res = URI_FUNC(ComposeQueryEx)(queryString, queryList, charsRequired,
      NULL, spaceToPlus, normalizeBreaks);
  if (res != URI_SUCCESS) {
    free(queryString);
    return res;
  }

  *dest = queryString;
  return URI_SUCCESS;
}



int URI_FUNC(ComposeQueryEngine)(URI_CHAR * dest,
    const URI_TYPE(QueryList) * queryList,
    int maxChars, int * charsWritten, int * charsRequired,
    UriBool spaceToPlus, UriBool normalizeBreaks) {
  UriBool firstItem = URI_TRUE;
  int ampersandLen = 0;
  URI_CHAR * write = dest;

  /* Subtract terminator */
  if (dest == NULL) {
    *charsRequired = 0;
  } else {
    maxChars--;
  }

  while (queryList != NULL) {
    const URI_CHAR * const key = queryList->key;
    const URI_CHAR * const value = queryList->value;
    const int worstCase = (normalizeBreaks == URI_TRUE ? 6 : 3);
    const int keyLen = (key == NULL) ? 0 : (int)URI_STRLEN(key);
    const int keyRequiredChars = worstCase * keyLen;
    const int valueLen = (value == NULL) ? 0 : (int)URI_STRLEN(value);
    const int valueRequiredChars = worstCase * valueLen;

    if (dest == NULL) {
      if (firstItem == URI_TRUE) {
        ampersandLen = 1;
        firstItem = URI_FALSE;
      }

      (*charsRequired) += ampersandLen + keyRequiredChars + ((value == NULL)
            ? 0
            : 1 + valueRequiredChars);
    } else {
      URI_CHAR * afterKey;

      if ((write - dest) + ampersandLen + keyRequiredChars > maxChars) {
        return URI_ERROR_OUTPUT_TOO_LARGE;
      }

      /* Copy key */
      if (firstItem == URI_TRUE) {
        firstItem = URI_FALSE;
      } else {
        write[0] = _UT('&');
        write++;
      }
      afterKey = URI_FUNC(EscapeEx)(key, key + keyLen,
          write, spaceToPlus, normalizeBreaks);
      write += (afterKey - write);

      if (value != NULL) {
        URI_CHAR * afterValue;

        if ((write - dest) + 1 + valueRequiredChars > maxChars) {
          return URI_ERROR_OUTPUT_TOO_LARGE;
        }

        /* Copy value */
        write[0] = _UT('=');
        write++;
        afterValue = URI_FUNC(EscapeEx)(value, value + valueLen,
            write, spaceToPlus, normalizeBreaks);
        write += (afterValue - write);
      }
    }

    queryList = queryList->next;
  }

  if (dest != NULL) {
    write[0] = _UT('\0');
    if (charsWritten != NULL) {
      *charsWritten = (int)(write - dest) + 1; /* .. for terminator */
    }
  }

  return URI_SUCCESS;
}



UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** prevNext,
    int * itemCount, const URI_CHAR * keyFirst, const URI_CHAR * keyAfter,
    const URI_CHAR * valueFirst, const URI_CHAR * valueAfter,
    UriBool plusToSpace, UriBreakConversion breakConversion) {
  const int keyLen = (int)(keyAfter - keyFirst);
  const int valueLen = (int)(valueAfter - valueFirst);
  URI_CHAR * key;
  URI_CHAR * value;

  if ((prevNext == NULL) || (itemCount == NULL)
      || (keyFirst == NULL) || (keyAfter == NULL)
      || (keyFirst > keyAfter) || (valueFirst > valueAfter)
      || ((keyFirst == keyAfter)
        && (valueFirst == NULL) && (valueAfter == NULL))) {
    return URI_TRUE;
  }

  /* Append new empty item */
  *prevNext = ( URI_TYPE(QueryList) * )malloc(1 * sizeof(URI_TYPE(QueryList)));
  if (*prevNext == NULL) {
    return URI_FALSE; /* Raises malloc error */
  }
  (*prevNext)->next = NULL;


  /* Fill key */
  key = ( URI_CHAR* )malloc((keyLen + 1) * sizeof(URI_CHAR));
  if (key == NULL) {
    free(*prevNext);
    *prevNext = NULL;
    return URI_FALSE; /* Raises malloc error */
  }

  key[keyLen] = _UT('\0');
  if (keyLen > 0) {
    /* Copy 1:1 */
    memcpy(key, keyFirst, keyLen * sizeof(URI_CHAR));

    /* Unescape */
    URI_FUNC(UnescapeInPlaceEx)(key, plusToSpace, breakConversion);
  }
  (*prevNext)->key = key;


  /* Fill value */
  if (valueFirst != NULL) {
    value = ( URI_CHAR* )malloc((valueLen + 1) * sizeof(URI_CHAR));
    if (value == NULL) {
      free(key);
      free(*prevNext);
      *prevNext = NULL;
      return URI_FALSE; /* Raises malloc error */
    }

    value[valueLen] = _UT('\0');
    if (valueLen > 0) {
      /* Copy 1:1 */
      memcpy(value, valueFirst, valueLen * sizeof(URI_CHAR));

      /* Unescape */
      URI_FUNC(UnescapeInPlaceEx)(value, plusToSpace, breakConversion);
    }
    (*prevNext)->value = value;
  } else {
    value = NULL;
  }
  (*prevNext)->value = value;

  (*itemCount)++;
  return URI_TRUE;
}



void URI_FUNC(FreeQueryList)(URI_TYPE(QueryList) * queryList) {
  while (queryList != NULL) {
    URI_TYPE(QueryList) * nextBackup = queryList->next;
    free(queryList->key);
    free(queryList->value);
    free(queryList);
    queryList = nextBackup;
  }
}



int URI_FUNC(DissectQueryMalloc)(URI_TYPE(QueryList) ** dest, int * itemCount,
    const URI_CHAR * first, const URI_CHAR * afterLast) {
  const UriBool plusToSpace = URI_TRUE;
  const UriBreakConversion breakConversion = URI_BR_DONT_TOUCH;

  return URI_FUNC(DissectQueryMallocEx)(dest, itemCount, first, afterLast,
      plusToSpace, breakConversion);
}



int URI_FUNC(DissectQueryMallocEx)(URI_TYPE(QueryList) ** dest, int * itemCount,
    const URI_CHAR * first, const URI_CHAR * afterLast,
    UriBool plusToSpace, UriBreakConversion breakConversion) {
  const URI_CHAR * walk = first;
  const URI_CHAR * keyFirst = first;
  const URI_CHAR * keyAfter = NULL;
  const URI_CHAR * valueFirst = NULL;
  const URI_CHAR * valueAfter = NULL;
  URI_TYPE(QueryList) ** prevNext = dest;
  int nullCounter;
  int * itemsAppended = (itemCount == NULL) ? &nullCounter : itemCount;

  if ((dest == NULL) || (first == NULL) || (afterLast == NULL)) {
    return URI_ERROR_NULL;
  }

  if (first > afterLast) {
    return URI_ERROR_RANGE_INVALID;
  }

  *dest = NULL;
  *itemsAppended = 0;

  /* Parse query string */
  for (; walk < afterLast; walk++) {
    switch (*walk) {
    case _UT('&'):
      if (valueFirst != NULL) {
        valueAfter = walk;
      } else {
        keyAfter = walk;
      }

      if (URI_FUNC(AppendQueryItem)(prevNext, itemsAppended,
          keyFirst, keyAfter, valueFirst, valueAfter,
          plusToSpace, breakConversion)
          == URI_FALSE) {
        /* Free list we built */
        *itemsAppended = 0;
        URI_FUNC(FreeQueryList)(*dest);
        return URI_ERROR_MALLOC;
      }

      /* Make future items children of the current */
      if ((prevNext != NULL) && (*prevNext != NULL)) {
        prevNext = &((*prevNext)->next);
      }

      if (walk + 1 < afterLast) {
        keyFirst = walk + 1;
      } else {
        keyFirst = NULL;
      }
      keyAfter = NULL;
      valueFirst = NULL;
      valueAfter = NULL;
      break;

    case _UT('='):
      /* NOTE: WE treat the first '=' as a separator, */
      /*       all following go into the value part   */
      if (keyAfter == NULL) {
        keyAfter = walk;
        if (walk + 1 < afterLast) {
          valueFirst = walk + 1;
          valueAfter = walk + 1;
        }
      }
      break;

    default:
      break;
    }
  }

  if (valueFirst != NULL) {
    /* Must be key/value pair */
    valueAfter = walk;
  } else {
    /* Must be key only */
    keyAfter = walk;
  }

  if (URI_FUNC(AppendQueryItem)(prevNext, itemsAppended, keyFirst, keyAfter,
      valueFirst, valueAfter, plusToSpace, breakConversion)
      == URI_FALSE) {
    /* Free list we built */
    *itemsAppended = 0;
    URI_FUNC(FreeQueryList)(*dest);
    return URI_ERROR_MALLOC;
  }

  return URI_SUCCESS;
}


/* Source: UriRecompose.c */


int URI_FUNC(ToStringCharsRequired)(const URI_TYPE(Uri) * uri,
    int * charsRequired) {
  const int MAX_CHARS = ((unsigned int)-1) >> 1;
  return URI_FUNC(ToStringEngine)(NULL, uri, MAX_CHARS, NULL, charsRequired);
}



int URI_FUNC(ToString)(URI_CHAR * dest, const URI_TYPE(Uri) * uri,
    int maxChars, int * charsWritten) {
  return URI_FUNC(ToStringEngine)(dest, uri, maxChars, charsWritten, NULL);
}



static URI_INLINE int URI_FUNC(ToStringEngine)(URI_CHAR * dest,
    const URI_TYPE(Uri) * uri, int maxChars, int * charsWritten,
    int * charsRequired) {
  int written = 0;
  if ((uri == NULL) || ((dest == NULL) && (charsRequired == NULL))) {
    if (charsWritten != NULL) {
      *charsWritten = 0;
    }
    return URI_ERROR_NULL;
  }

  if (maxChars < 1) {
    if (charsWritten != NULL) {
      *charsWritten = 0;
    }
    return URI_ERROR_TOSTRING_TOO_LONG;
  }
  maxChars--; /* So we don't have to substract 1 for '\0' all the time */

  /* [01/19]  result = "" */
        if (dest != NULL) {
          dest[0] = _UT('\0');
        } else {
          (*charsRequired) = 0;
        }
  /* [02/19]  if defined(scheme) then */
        if (uri->scheme.first != NULL) {
  /* [03/19]    append scheme to result; */
          const int charsToWrite
              = (int)(uri->scheme.afterLast - uri->scheme.first);
          if (dest != NULL) {
            if (written + charsToWrite <= maxChars) {
              memcpy(dest + written, uri->scheme.first,
                  charsToWrite * sizeof(URI_CHAR));
              written += charsToWrite;
            } else {
              dest[0] = _UT('\0');
              if (charsWritten != NULL) {
                *charsWritten = 0;
              }
              return URI_ERROR_TOSTRING_TOO_LONG;
            }
          } else {
            (*charsRequired) += charsToWrite;
          }
  /* [04/19]    append ":" to result; */
          if (dest != NULL) {
            if (written + 1 <= maxChars) {
              memcpy(dest + written, _UT(":"),
                  1 * sizeof(URI_CHAR));
              written += 1;
            } else {
              dest[0] = _UT('\0');
              if (charsWritten != NULL) {
                *charsWritten = 0;
              }
              return URI_ERROR_TOSTRING_TOO_LONG;
            }
          } else {
            (*charsRequired) += 1;
          }
  /* [05/19]  endif; */
        }
  /* [06/19]  if defined(authority) then */
        if (URI_FUNC(IsHostSet)(uri)) {
  /* [07/19]    append "//" to result; */
          if (dest != NULL) {
            if (written + 2 <= maxChars) {
              memcpy(dest + written, _UT("//"),
                  2 * sizeof(URI_CHAR));
              written += 2;
            } else {
              dest[0] = _UT('\0');
              if (charsWritten != NULL) {
                *charsWritten = 0;
              }
              return URI_ERROR_TOSTRING_TOO_LONG;
            }
          } else {
            (*charsRequired) += 2;
          }
  /* [08/19]    append authority to result; */
          /* UserInfo */
          if (uri->userInfo.first != NULL) {
            const int charsToWrite = (int)(uri->userInfo.afterLast - uri->userInfo.first);
            if (dest != NULL) {
              if (written + charsToWrite <= maxChars) {
                memcpy(dest + written, uri->userInfo.first,
                    charsToWrite * sizeof(URI_CHAR));
                written += charsToWrite;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }

              if (written + 1 <= maxChars) {
                memcpy(dest + written, _UT("@"),
                    1 * sizeof(URI_CHAR));
                written += 1;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += charsToWrite + 1;
            }
          }

          /* Host */
          if (uri->hostData.ip4 != NULL) {
            /* IPv4 */
            int i = 0;
            for (; i < 4; i++) {
              const unsigned char value = uri->hostData.ip4->data[i];
              const int charsToWrite = (value > 99) ? 3 : ((value > 9) ? 2 : 1);
              if (dest != NULL) {
                if (written + charsToWrite <= maxChars) {
                  URI_CHAR text[4];
                  if (value > 99) {
                    text[0] = _UT('0') + (value / 100);
                    text[1] = _UT('0') + ((value % 100) / 10);
                    text[2] = _UT('0') + (value % 10);
                  } else if (value > 9)  {
                    text[0] = _UT('0') + (value / 10);
                    text[1] = _UT('0') + (value % 10);
                  } else {
                    text[0] = _UT('0') + value;
                  }
                  text[charsToWrite] = _UT('\0');
                  memcpy(dest + written, text, charsToWrite * sizeof(URI_CHAR));
                  written += charsToWrite;
                } else {
                  dest[0] = _UT('\0');
                  if (charsWritten != NULL) {
                    *charsWritten = 0;
                  }
                  return URI_ERROR_TOSTRING_TOO_LONG;
                }
                if (i < 3) {
                  if (written + 1 <= maxChars) {
                    memcpy(dest + written, _UT("."),
                        1 * sizeof(URI_CHAR));
                    written += 1;
                  } else {
                    dest[0] = _UT('\0');
                    if (charsWritten != NULL) {
                      *charsWritten = 0;
                    }
                    return URI_ERROR_TOSTRING_TOO_LONG;
                  }
                }
              } else {
                (*charsRequired) += charsToWrite + 1;
              }
            }
          } else if (uri->hostData.ip6 != NULL) {
            /* IPv6 */
            int i = 0;
            if (dest != NULL) {
              if (written + 1 <= maxChars) {
                memcpy(dest + written, _UT("["),
                    1 * sizeof(URI_CHAR));
                written += 1;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += 1;
            }

            for (; i < 16; i++) {
              const unsigned char value = uri->hostData.ip6->data[i];
              if (dest != NULL) {
                if (written + 2 <= maxChars) {
                  URI_CHAR text[3];
                  text[0] = URI_FUNC(HexToLetterEx)(value / 16, URI_FALSE);
                  text[1] = URI_FUNC(HexToLetterEx)(value % 16, URI_FALSE);
                  text[2] = _UT('\0');
                  memcpy(dest + written, text, 2 * sizeof(URI_CHAR));
                  written += 2;
                } else {
                  dest[0] = _UT('\0');
                  if (charsWritten != NULL) {
                    *charsWritten = 0;
                  }
                  return URI_ERROR_TOSTRING_TOO_LONG;
                }
              } else {
                (*charsRequired) += 2;
              }
              if (((i & 1) == 1) && (i < 15)) {
                if (dest != NULL) {
                  if (written + 1 <= maxChars) {
                    memcpy(dest + written, _UT(":"),
                        1 * sizeof(URI_CHAR));
                    written += 1;
                  } else {
                    dest[0] = _UT('\0');
                    if (charsWritten != NULL) {
                      *charsWritten = 0;
                    }
                    return URI_ERROR_TOSTRING_TOO_LONG;
                  }
                } else {
                  (*charsRequired) += 1;
                }
              }
            }

            if (dest != NULL) {
              if (written + 1 <= maxChars) {
                memcpy(dest + written, _UT("]"),
                    1 * sizeof(URI_CHAR));
                written += 1;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += 1;
            }
          } else if (uri->hostData.ipFuture.first != NULL) {
            /* IPvFuture */
            const int charsToWrite = (int)(uri->hostData.ipFuture.afterLast
                - uri->hostData.ipFuture.first);
            if (dest != NULL) {
              if (written + 1 <= maxChars) {
                memcpy(dest + written, _UT("["),
                    1 * sizeof(URI_CHAR));
                written += 1;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }

              if (written + charsToWrite <= maxChars) {
                memcpy(dest + written, uri->hostData.ipFuture.first, charsToWrite * sizeof(URI_CHAR));
                written += charsToWrite;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }

              if (written + 1 <= maxChars) {
                memcpy(dest + written, _UT("]"),
                    1 * sizeof(URI_CHAR));
                written += 1;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += 1 + charsToWrite + 1;
            }
          } else if (uri->hostText.first != NULL) {
            /* Regname */
            const int charsToWrite = (int)(uri->hostText.afterLast - uri->hostText.first);
            if (dest != NULL) {
              if (written + charsToWrite <= maxChars) {
                memcpy(dest + written, uri->hostText.first,
                    charsToWrite * sizeof(URI_CHAR));
                written += charsToWrite;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += charsToWrite;
            }
          }

          /* Port */
          if (uri->portText.first != NULL) {
            const int charsToWrite = (int)(uri->portText.afterLast - uri->portText.first);
            if (dest != NULL) {
              /* Leading ':' */
              if (written + 1 <= maxChars) {
                  memcpy(dest + written, _UT(":"),
                      1 * sizeof(URI_CHAR));
                  written += 1;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }

              /* Port number */
              if (written + charsToWrite <= maxChars) {
                memcpy(dest + written, uri->portText.first,
                    charsToWrite * sizeof(URI_CHAR));
                written += charsToWrite;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += 1 + charsToWrite;
            }
          }
  /* [09/19]  endif; */
        }
  /* [10/19]  append path to result; */
        /* Slash needed here? */
        if (uri->absolutePath || ((uri->pathHead != NULL)
            && URI_FUNC(IsHostSet)(uri))) {
          if (dest != NULL) {
            if (written + 1 <= maxChars) {
              memcpy(dest + written, _UT("/"),
                  1 * sizeof(URI_CHAR));
              written += 1;
            } else {
              dest[0] = _UT('\0');
              if (charsWritten != NULL) {
                *charsWritten = 0;
              }
              return URI_ERROR_TOSTRING_TOO_LONG;
            }
          } else {
            (*charsRequired) += 1;
          }
        }

        if (uri->pathHead != NULL) {
          URI_TYPE(PathSegment) * walker = uri->pathHead;
          do {
            const int charsToWrite = (int)(walker->text.afterLast - walker->text.first);
            if (dest != NULL) {
              if (written + charsToWrite <= maxChars) {
                memcpy(dest + written, walker->text.first,
                    charsToWrite * sizeof(URI_CHAR));
                written += charsToWrite;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += charsToWrite;
            }

            /* Not last segment -> append slash */
            if (walker->next != NULL) {
              if (dest != NULL) {
                if (written + 1 <= maxChars) {
                  memcpy(dest + written, _UT("/"),
                      1 * sizeof(URI_CHAR));
                  written += 1;
                } else {
                  dest[0] = _UT('\0');
                  if (charsWritten != NULL) {
                    *charsWritten = 0;
                  }
                  return URI_ERROR_TOSTRING_TOO_LONG;
                }
              } else {
                (*charsRequired) += 1;
              }
            }

            walker = walker->next;
          } while (walker != NULL);
        }
  /* [11/19]  if defined(query) then */
        if (uri->query.first != NULL) {
  /* [12/19]    append "?" to result; */
          if (dest != NULL) {
            if (written + 1 <= maxChars) {
              memcpy(dest + written, _UT("?"),
                  1 * sizeof(URI_CHAR));
              written += 1;
            } else {
              dest[0] = _UT('\0');
              if (charsWritten != NULL) {
                *charsWritten = 0;
              }
              return URI_ERROR_TOSTRING_TOO_LONG;
            }
          } else {
            (*charsRequired) += 1;
          }
  /* [13/19]    append query to result; */
          {
            const int charsToWrite
                = (int)(uri->query.afterLast - uri->query.first);
            if (dest != NULL) {
              if (written + charsToWrite <= maxChars) {
                memcpy(dest + written, uri->query.first,
                    charsToWrite * sizeof(URI_CHAR));
                written += charsToWrite;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += charsToWrite;
            }
          }
  /* [14/19]  endif; */
        }
  /* [15/19]  if defined(fragment) then */
        if (uri->fragment.first != NULL) {
  /* [16/19]    append "#" to result; */
          if (dest != NULL) {
            if (written + 1 <= maxChars) {
              memcpy(dest + written, _UT("#"),
                  1 * sizeof(URI_CHAR));
              written += 1;
            } else {
              dest[0] = _UT('\0');
              if (charsWritten != NULL) {
                *charsWritten = 0;
              }
              return URI_ERROR_TOSTRING_TOO_LONG;
            }
          } else {
            (*charsRequired) += 1;
          }
  /* [17/19]    append fragment to result; */
          {
            const int charsToWrite
                = (int)(uri->fragment.afterLast - uri->fragment.first);
            if (dest != NULL) {
              if (written + charsToWrite <= maxChars) {
                memcpy(dest + written, uri->fragment.first,
                    charsToWrite * sizeof(URI_CHAR));
                written += charsToWrite;
              } else {
                dest[0] = _UT('\0');
                if (charsWritten != NULL) {
                  *charsWritten = 0;
                }
                return URI_ERROR_TOSTRING_TOO_LONG;
              }
            } else {
              (*charsRequired) += charsToWrite;
            }
          }
  /* [18/19]  endif; */
        }
  /* [19/19]  return result; */
        if (dest != NULL) {
          dest[written++] = _UT('\0');
          if (charsWritten != NULL) {
            *charsWritten = written;
          }
        }
        return URI_SUCCESS;
}


/* Source: UriResolve.c */
/* Appends a relative URI to an absolute. The last path segement of
 * the absolute URI is replaced. */
static URI_INLINE UriBool URI_FUNC(MergePath)(URI_TYPE(Uri) * absWork,
    const URI_TYPE(Uri) * relAppend) {
  URI_TYPE(PathSegment) * sourceWalker;
  URI_TYPE(PathSegment) * destPrev;
  if (relAppend->pathHead == NULL) {
    return URI_TRUE;
  }

  /* Replace last segment ("" if trailing slash) with first of append chain */
  if (absWork->pathHead == NULL) {
    URI_TYPE(PathSegment) * const dup = ( URI_TYPE(PathSegment) * const )malloc(sizeof(URI_TYPE(PathSegment)));
    if (dup == NULL) {
      return URI_FALSE; /* Raises malloc error */
    }
    dup->next = NULL;
    absWork->pathHead = dup;
    absWork->pathTail = dup;
  }
  absWork->pathTail->text.first = relAppend->pathHead->text.first;
  absWork->pathTail->text.afterLast = relAppend->pathHead->text.afterLast;

  /* Append all the others */
  sourceWalker = relAppend->pathHead->next;
  if (sourceWalker == NULL) {
    return URI_TRUE;
  }
  destPrev = absWork->pathTail;

  for (;;) {
    URI_TYPE(PathSegment) * const dup = ( URI_TYPE(PathSegment) * const )malloc(sizeof(URI_TYPE(PathSegment)));
    if (dup == NULL) {
      destPrev->next = NULL;
      absWork->pathTail = destPrev;
      return URI_FALSE; /* Raises malloc error */
    }
    dup->text = sourceWalker->text;
    destPrev->next = dup;

    if (sourceWalker->next == NULL) {
      absWork->pathTail = dup;
      absWork->pathTail->next = NULL;
      break;
    }
    destPrev = dup;
    sourceWalker = sourceWalker->next;
  }

  return URI_TRUE;
}



static int URI_FUNC(AddBaseUriImpl)(URI_TYPE(Uri) * absDest,
    const URI_TYPE(Uri) * relSource,
    const URI_TYPE(Uri) * absBase) {
  if (absDest == NULL) {
    return URI_ERROR_NULL;
  }
  URI_FUNC(ResetUri)(absDest);

  if ((relSource == NULL) || (absBase == NULL)) {
    return URI_ERROR_NULL;
  }

  /* absBase absolute? */
  if (absBase->scheme.first == NULL) {
    return URI_ERROR_ADDBASE_REL_BASE;
  }

  /* [01/32]  if defined(R.scheme) then */
        if (relSource->scheme.first != NULL) {
  /* [02/32]    T.scheme = R.scheme; */
          absDest->scheme = relSource->scheme;
  /* [03/32]    T.authority = R.authority; */
          if (!URI_FUNC(CopyAuthority)(absDest, relSource)) {
            return URI_ERROR_MALLOC;
          }
  /* [04/32]    T.path = remove_dot_segments(R.path); */
          if (!URI_FUNC(CopyPath)(absDest, relSource)) {
            return URI_ERROR_MALLOC;
          }
          if (!URI_FUNC(RemoveDotSegmentsAbsolute)(absDest)) {
            return URI_ERROR_MALLOC;
          }
  /* [05/32]    T.query = R.query; */
          absDest->query = relSource->query;
  /* [06/32]  else */
        } else {
  /* [07/32]    if defined(R.authority) then */
          if (URI_FUNC(IsHostSet)(relSource)) {
  /* [08/32]      T.authority = R.authority; */
            if (!URI_FUNC(CopyAuthority)(absDest, relSource)) {
              return URI_ERROR_MALLOC;
            }
  /* [09/32]      T.path = remove_dot_segments(R.path); */
            if (!URI_FUNC(CopyPath)(absDest, relSource)) {
              return URI_ERROR_MALLOC;
            }
            if (!URI_FUNC(RemoveDotSegmentsAbsolute)(absDest)) {
              return URI_ERROR_MALLOC;
            }
  /* [10/32]      T.query = R.query; */
            absDest->query = relSource->query;
  /* [11/32]    else */
          } else {
  /* [28/32]      T.authority = Base.authority; */
            if (!URI_FUNC(CopyAuthority)(absDest, absBase)) {
              return URI_ERROR_MALLOC;
            }
  /* [12/32]      if (R.path == "") then */
            if (relSource->pathHead == NULL) {
  /* [13/32]        T.path = Base.path; */
              if (!URI_FUNC(CopyPath)(absDest, absBase)) {
                return URI_ERROR_MALLOC;
              }
  /* [14/32]        if defined(R.query) then */
              if (relSource->query.first != NULL) {
  /* [15/32]          T.query = R.query; */
                absDest->query = relSource->query;
  /* [16/32]        else */
              } else {
  /* [17/32]          T.query = Base.query; */
                absDest->query = absBase->query;
  /* [18/32]        endif; */
              }
  /* [19/32]      else */
            } else {
  /* [20/32]        if (R.path starts-with "/") then */
              if (relSource->absolutePath) {
  /* [21/32]          T.path = remove_dot_segments(R.path); */
                if (!URI_FUNC(CopyPath)(absDest, relSource)) {
                  return URI_ERROR_MALLOC;
                }
                if (!URI_FUNC(RemoveDotSegmentsAbsolute)(absDest)) {
                  return URI_ERROR_MALLOC;
                }
  /* [22/32]        else */
              } else {
  /* [23/32]          T.path = merge(Base.path, R.path); */
                if (!URI_FUNC(CopyPath)(absDest, absBase)) {
                  return URI_ERROR_MALLOC;
                }
                if (!URI_FUNC(MergePath)(absDest, relSource)) {
                  return URI_ERROR_MALLOC;
                }
  /* [24/32]          T.path = remove_dot_segments(T.path); */
                if (!URI_FUNC(RemoveDotSegmentsAbsolute)(absDest)) {
                  return URI_ERROR_MALLOC;
                }

                if (!URI_FUNC(FixAmbiguity)(absDest)) {
                  return URI_ERROR_MALLOC;
                }
  /* [25/32]        endif; */
              }
  /* [26/32]        T.query = R.query; */
              absDest->query = relSource->query;
  /* [27/32]      endif; */
            }
            URI_FUNC(FixEmptyTrailSegment)(absDest);
  /* [29/32]    endif; */
          }
  /* [30/32]    T.scheme = Base.scheme; */
          absDest->scheme = absBase->scheme;
  /* [31/32]  endif; */
        }
  /* [32/32]  T.fragment = R.fragment; */
        absDest->fragment = relSource->fragment;

  return URI_SUCCESS;

}



int URI_FUNC(AddBaseUri)(URI_TYPE(Uri) * absDest,
    const URI_TYPE(Uri) * relSource, const URI_TYPE(Uri) * absBase) {
  const int res = URI_FUNC(AddBaseUriImpl)(absDest, relSource, absBase);
  if ((res != URI_SUCCESS) && (absDest != NULL)) {
    URI_FUNC(FreeUriMembers)(absDest);
  }
  return res;
}


/* Source: UriShorten.c */

static URI_INLINE UriBool URI_FUNC(AppendSegment)(URI_TYPE(Uri) * uri,
    const URI_CHAR * first, const URI_CHAR * afterLast) {
  /* Create segment */
  URI_TYPE(PathSegment) * segment = ( URI_TYPE(PathSegment) * const )malloc(1 * sizeof(URI_TYPE(PathSegment)));
  if (segment == NULL) {
    return URI_FALSE; /* Raises malloc error */
  }
  segment->next = NULL;
  segment->text.first = first;
  segment->text.afterLast = afterLast;

  /* Put into chain */
  if (uri->pathTail == NULL) {
    uri->pathHead = segment;
  } else {
    uri->pathTail->next = segment;
  }
  uri->pathTail = segment;

  return URI_TRUE;
}



static URI_INLINE UriBool URI_FUNC(EqualsAuthority)(const URI_TYPE(Uri) * first,
    const URI_TYPE(Uri) * second) {
  /* IPv4 */
  if (first->hostData.ip4 != NULL) {
    return ((second->hostData.ip4 != NULL)
        && !memcmp(first->hostData.ip4->data,
          second->hostData.ip4->data, 4)) ? URI_TRUE : URI_FALSE;
  }

  /* IPv6 */
  if (first->hostData.ip6 != NULL) {
    return ((second->hostData.ip6 != NULL)
        && !memcmp(first->hostData.ip6->data,
          second->hostData.ip6->data, 16)) ? URI_TRUE : URI_FALSE;
  }

  /* IPvFuture */
  if (first->hostData.ipFuture.first != NULL) {
    return ((second->hostData.ipFuture.first != NULL)
        && !URI_STRNCMP(first->hostData.ipFuture.first,
          second->hostData.ipFuture.first,
          first->hostData.ipFuture.afterLast
          - first->hostData.ipFuture.first))
            ? URI_TRUE : URI_FALSE;
  }

  if (first->hostText.first != NULL) {
    return ((second->hostText.first != NULL)
        && !URI_STRNCMP(first->hostText.first,
          second->hostText.first,
          first->hostText.afterLast
          - first->hostText.first)) ? URI_TRUE : URI_FALSE;
  }

  return (second->hostText.first == NULL);
}



int URI_FUNC(RemoveBaseUriImpl)(URI_TYPE(Uri) * dest,
    const URI_TYPE(Uri) * absSource,
    const URI_TYPE(Uri) * absBase,
    UriBool domainRootMode) {
  if (dest == NULL) {
    return URI_ERROR_NULL;
  }
  URI_FUNC(ResetUri)(dest);

  if ((absSource == NULL) || (absBase == NULL)) {
    return URI_ERROR_NULL;
  }

  /* absBase absolute? */
  if (absBase->scheme.first == NULL) {
    return URI_ERROR_REMOVEBASE_REL_BASE;
  }

  /* absSource absolute? */
  if (absSource->scheme.first == NULL) {
    return URI_ERROR_REMOVEBASE_REL_SOURCE;
  }

  /* [01/50]  if (A.scheme != Base.scheme) then */
        if (URI_STRNCMP(absSource->scheme.first, absBase->scheme.first,
            absSource->scheme.afterLast - absSource->scheme.first)) {
  /* [02/50]     T.scheme    = A.scheme; */
          dest->scheme = absSource->scheme;
  /* [03/50]     T.authority = A.authority; */
          if (!URI_FUNC(CopyAuthority)(dest, absSource)) {
            return URI_ERROR_MALLOC;
          }
  /* [04/50]     T.path      = A.path; */
          if (!URI_FUNC(CopyPath)(dest, absSource)) {
            return URI_ERROR_MALLOC;
          }
  /* [05/50]  else */
        } else {
  /* [06/50]     undef(T.scheme); */
          /* NOOP */
  /* [07/50]     if (A.authority != Base.authority) then */
          if (!URI_FUNC(EqualsAuthority)(absSource, absBase)) {
  /* [08/50]        T.authority = A.authority; */
            if (!URI_FUNC(CopyAuthority)(dest, absSource)) {
              return URI_ERROR_MALLOC;
            }
  /* [09/50]        T.path      = A.path; */
            if (!URI_FUNC(CopyPath)(dest, absSource)) {
              return URI_ERROR_MALLOC;
            }
  /* [10/50]     else */
          } else {
  /* [11/50]        if domainRootMode then */
            if (domainRootMode == URI_TRUE) {
  /* [12/50]           undef(T.authority); */
              /* NOOP */
  /* [13/50]           if (first(A.path) == "") then */
              /* GROUPED */
  /* [14/50]              T.path   = "/." + A.path; */
                /* GROUPED */
  /* [15/50]           else */
                /* GROUPED */
  /* [16/50]              T.path   = A.path; */
                /* GROUPED */
  /* [17/50]           endif; */
              if (!URI_FUNC(CopyPath)(dest, absSource)) {
                return URI_ERROR_MALLOC;
              }
              dest->absolutePath = URI_TRUE;

              if (!URI_FUNC(FixAmbiguity)(dest)) {
                return URI_ERROR_MALLOC;
              }
  /* [18/50]        else */
            } else {
              const URI_TYPE(PathSegment) * sourceSeg = absSource->pathHead;
              const URI_TYPE(PathSegment) * baseSeg = absBase->pathHead;
  /* [19/50]           bool pathNaked = true; */
              UriBool pathNaked = URI_TRUE;
  /* [20/50]           undef(last(Base.path)); */
              /* NOOP */
  /* [21/50]           T.path = ""; */
              dest->absolutePath = URI_FALSE;
  /* [22/50]           while (first(A.path) == first(Base.path)) do */
              while ((sourceSeg != NULL) && (baseSeg != NULL)
                  && !URI_STRNCMP(sourceSeg->text.first, baseSeg->text.first,
                  sourceSeg->text.afterLast - sourceSeg->text.first)
                  && !((sourceSeg->text.first == sourceSeg->text.afterLast)
                    && ((sourceSeg->next == NULL) != (baseSeg->next == NULL)))) {
  /* [23/50]              A.path++; */
                sourceSeg = sourceSeg->next;
  /* [24/50]              Base.path++; */
                baseSeg = baseSeg->next;
  /* [25/50]           endwhile; */
              }
  /* [26/50]           while defined(first(Base.path)) do */
              while ((baseSeg != NULL) && (baseSeg->next != NULL)) {
  /* [27/50]              Base.path++; */
                baseSeg = baseSeg->next;
  /* [28/50]              T.path += "../"; */
                if (!URI_FUNC(AppendSegment)(dest, URI_FUNC(ConstParent),
                    URI_FUNC(ConstParent) + 2)) {
                  return URI_ERROR_MALLOC;
                }
  /* [29/50]              pathNaked = false; */
                pathNaked = URI_FALSE;
  /* [30/50]           endwhile; */
              }
  /* [31/50]           while defined(first(A.path)) do */
              while (sourceSeg != NULL) {
  /* [32/50]              if pathNaked then */
                if (pathNaked == URI_TRUE) {
  /* [33/50]                 if (first(A.path) contains ":") then */
                  UriBool containsColon = URI_FALSE;
                  const URI_CHAR * ch = sourceSeg->text.first;
                  for (; ch < sourceSeg->text.afterLast; ch++) {
                    if (*ch == _UT(':')) {
                      containsColon = URI_TRUE;
                      break;
                    }
                  }

                  if (containsColon) {
  /* [34/50]                    T.path += "./"; */
                    if (!URI_FUNC(AppendSegment)(dest, URI_FUNC(ConstPwd),
                        URI_FUNC(ConstPwd) + 1)) {
                      return URI_ERROR_MALLOC;
                    }
  /* [35/50]                 elseif (first(A.path) == "") then */
                  } else if (sourceSeg->text.first == sourceSeg->text.afterLast) {
  /* [36/50]                    T.path += "/."; */
                    if (!URI_FUNC(AppendSegment)(dest, URI_FUNC(ConstPwd),
                        URI_FUNC(ConstPwd) + 1)) {
                      return URI_ERROR_MALLOC;
                    }
  /* [37/50]                 endif; */
                  }
  /* [38/50]              endif; */
                }
  /* [39/50]              T.path += first(A.path); */
                if (!URI_FUNC(AppendSegment)(dest, sourceSeg->text.first,
                    sourceSeg->text.afterLast)) {
                  return URI_ERROR_MALLOC;
                }
  /* [40/50]              pathNaked = false; */
                pathNaked = URI_FALSE;
  /* [41/50]              A.path++; */
                sourceSeg = sourceSeg->next;
  /* [42/50]              if defined(first(A.path)) then */
                /* NOOP */
  /* [43/50]                 T.path += + "/"; */
                /* NOOP */
  /* [44/50]              endif; */
                /* NOOP */
  /* [45/50]           endwhile; */
              }
  /* [46/50]        endif; */
            }
  /* [47/50]     endif; */
          }
  /* [48/50]  endif; */
        }
  /* [49/50]  T.query     = A.query; */
        dest->query = absSource->query;
  /* [50/50]  T.fragment  = A.fragment; */
        dest->fragment = absSource->fragment;

  return URI_SUCCESS;
}



int URI_FUNC(RemoveBaseUri)(URI_TYPE(Uri) * dest,
    const URI_TYPE(Uri) * absSource,
    const URI_TYPE(Uri) * absBase,
    UriBool domainRootMode) {
  const int res = URI_FUNC(RemoveBaseUriImpl)(dest, absSource,
      absBase, domainRootMode);
  if ((res != URI_SUCCESS) && (dest != NULL)) {
    URI_FUNC(FreeUriMembers)(dest);
  }
  return res;
}


// yajl.c
/*
 * Copyright 2007, Lloyd Hilaiel.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the name of Lloyd Hilaiel nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */



#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>


/* Source: yajl.c */
const char *
yajl_status_to_string(yajl_status stat)
{
    const char * statStr = "unknown";
    switch (stat) {
        case yajl_status_ok:
            statStr = "ok, no error";
            break;
        case yajl_status_client_canceled:
            statStr = "client canceled parse";
            break;
        case yajl_status_insufficient_data:
            statStr = "eof was met before the parse could complete";
            break;
        case yajl_status_error:
            statStr = "parse error";
            break;
    }
    return statStr;
}

yajl_handle
yajl_alloc(const yajl_callbacks * callbacks,
           const yajl_parser_config * config,
           void * ctx)
{
    unsigned int allowComments = 0;
    unsigned int validateUTF8 = 0;
    yajl_handle hand = (yajl_handle) malloc(sizeof(struct yajl_handle_t));

    if (config != NULL) {
        allowComments = config->allowComments;
        validateUTF8 = config->checkUTF8;
    }

    hand->callbacks = callbacks;
    hand->ctx = ctx;
    hand->lexer = yajl_lex_alloc(allowComments, validateUTF8);
    hand->errorOffset = 0;
    hand->decodeBuf = yajl_buf_alloc();
    hand->stateBuf = yajl_buf_alloc();

    yajl_state_push(hand, yajl_state_start);

    return hand;
}

void
yajl_free(yajl_handle handle)
{
    yajl_buf_free(handle->stateBuf);
    yajl_buf_free(handle->decodeBuf);
    yajl_lex_free(handle->lexer);
    free(handle);
}

yajl_status
yajl_parse(yajl_handle hand, const unsigned char * jsonText,
           unsigned int jsonTextLen)
{
    unsigned int offset = 0;
    yajl_status status;
    status = yajl_do_parse(hand, &offset, jsonText, jsonTextLen);
    return status;
}

unsigned char *
yajl_get_error(yajl_handle hand, int verbose,
               const unsigned char * jsonText, unsigned int jsonTextLen)
{
    return yajl_render_error_string(hand, jsonText, jsonTextLen, verbose);
}

void
yajl_free_error(unsigned char * str)
{
    /* XXX: use memory allocation functions if set */
    free(str);
}


/* yajl_buf.c */
#define YAJL_BUF_INIT_SIZE 2048

struct yajl_buf_t {
    unsigned int len;
    unsigned int used;
    unsigned char * data;
};

static
void yajl_buf_ensure_available(yajl_buf buf, unsigned int want)
{
    unsigned int need;

    assert(buf != NULL);

    /* first call */
    if (buf->data == NULL) {
        buf->len = YAJL_BUF_INIT_SIZE;
        buf->data = (unsigned char *) malloc(buf->len);
        buf->data[0] = 0;
    }

    need = buf->len;

    while (want >= (need - buf->used)) need <<= 1;

    if (need != buf->len) {
        buf->data = (unsigned char *) realloc(buf->data, need);
        buf->len = need;
    }
}

yajl_buf yajl_buf_alloc(void)
{
    return (yajl_buf) calloc(1, sizeof(struct yajl_buf_t));
}

void yajl_buf_free(yajl_buf buf)
{
    assert(buf != NULL);
    if (buf->data) free(buf->data);
    free(buf);
}

void yajl_buf_append(yajl_buf buf, const void * data, unsigned int len)
{
    yajl_buf_ensure_available(buf, len);
    if (len > 0) {
        assert(data != NULL);
        memcpy(buf->data + buf->used, data, len);
        buf->used += len;
        buf->data[buf->used] = 0;
    }
}

void yajl_buf_clear(yajl_buf buf)
{
    buf->used = 0;
    if (buf->data) buf->data[buf->used] = 0;
}

const unsigned char * yajl_buf_data(yajl_buf buf)
{
    return buf->data;
}

unsigned int yajl_buf_len(yajl_buf buf)
{
    return buf->used;
}

void
yajl_buf_truncate(yajl_buf buf, unsigned int len)
{
    assert(len <= buf->used);
    buf->used = len;
}


/* yajl_encode.c */
static void CharToHex(unsigned char c, char * hexBuf)
{
    const char * hexchar = "0123456789ABCDEF";
    hexBuf[0] = hexchar[c >> 4];
    hexBuf[1] = hexchar[c & 0x0F];
}

void
yajl_string_encode(yajl_buf buf, const unsigned char * str,
                   unsigned int len)
{
    unsigned int beg = 0;
    unsigned int end = 0;
    char hexBuf[7];
    hexBuf[0] = '\\'; hexBuf[1] = 'u'; hexBuf[2] = '0'; hexBuf[3] = '0';
    hexBuf[6] = 0;

    while (end < len) {
        char * escaped = NULL;
        switch (str[end]) {
            case '\r': escaped = "\\r"; break;
            case '\n': escaped = "\\n"; break;
            case '\\': escaped = "\\\\"; break;
            /* case '/': escaped = "\\/"; break; */
            case '"': escaped = "\\\""; break;
            case '\f': escaped = "\\f"; break;
            case '\b': escaped = "\\b"; break;
            case '\t': escaped = "\\t"; break;
            default:
                if ((unsigned char) str[end] < 32) {
                    CharToHex(str[end], hexBuf + 4);
                    escaped = hexBuf;
                }
                break;
        }
        if (escaped != NULL) {
            yajl_buf_append(buf, str + beg, end - beg);
            yajl_buf_append(buf, escaped, strlen(escaped));
            beg = ++end;
        } else {
            ++end;
        }
    }
    yajl_buf_append(buf, str + beg, end - beg);
}

static void hexToDigit(unsigned int * val, const unsigned char * hex)
{
    unsigned int i;
    for (i=0;i<4;i++) {
        unsigned char c = hex[i];
        if (c >= 'A') c = (c & ~0x20) - 7;
        c -= '0';
        assert(!(c & 0xF0));
        *val = (*val << 4) | c;
    }
}

static void Utf32toUtf8(unsigned int codepoint, char * utf8Buf)
{
    if (codepoint < 0x80) {
        utf8Buf[0] = (char) codepoint;
        utf8Buf[1] = 0;
    } else if (codepoint < 0x0800) {
        utf8Buf[0] = (char) ((codepoint >> 6) | 0xC0);
        utf8Buf[1] = (char) ((codepoint & 0x3F) | 0x80);
        utf8Buf[2] = 0;
    } else if (codepoint < 0x10000) {
        utf8Buf[0] = (char) ((codepoint >> 12) | 0xE0);
        utf8Buf[1] = (char) (((codepoint >> 6) & 0x3F) | 0x80);
        utf8Buf[2] = (char) ((codepoint & 0x3F) | 0x80);
        utf8Buf[3] = 0;
    } else if (codepoint < 0x200000) {
        utf8Buf[0] =(char)((codepoint >> 18) | 0xF0);
        utf8Buf[1] =(char)(((codepoint >> 12) & 0x3F) | 0x80);
        utf8Buf[2] =(char)(((codepoint >> 6) & 0x3F) | 0x80);
        utf8Buf[3] =(char)((codepoint & 0x3F) | 0x80);
        utf8Buf[4] = 0;
    } else {
        utf8Buf[0] = '?';
        utf8Buf[1] = 0;
    }
}

void yajl_string_decode(yajl_buf buf, const unsigned char * str,
                        unsigned int len)
{
    unsigned int beg = 0;
    unsigned int end = 0;

    while (end < len) {
        if (str[end] == '\\') {
            char utf8Buf[5];
            const char * unescaped = "?";
            yajl_buf_append(buf, str + beg, end - beg);
            switch (str[++end]) {
                case 'r': unescaped = "\r"; break;
                case 'n': unescaped = "\n"; break;
                case '\\': unescaped = "\\"; break;
                case '/': unescaped = "/"; break;
                case '"': unescaped = "\""; break;
                case 'f': unescaped = "\f"; break;
                case 'b': unescaped = "\b"; break;
                case 't': unescaped = "\t"; break;
                case 'u': {
                    unsigned int codepoint = 0;
                    hexToDigit(&codepoint, str + ++end);
                    end+=3;
                    /* check if this is a surrogate */
                    if ((codepoint & 0xFC00) == 0xD800) {
                        end++;
                        if (str[end] == '\\' && str[end + 1] == 'u') {
                            unsigned int surrogate = 0;
                            hexToDigit(&surrogate, str + end + 2);
                            codepoint =
                                (((codepoint & 0x3F) << 10) |
                                 ((((codepoint >> 6) & 0xF) + 1) << 16) |
                                 (surrogate & 0x3FF));
                            end += 5;
                        } else {
                            unescaped = "?";
                            break;
                        }
                    }

                    Utf32toUtf8(codepoint, utf8Buf);
                    unescaped = utf8Buf;
                    break;
                }
                default:
                    assert("this should never happen" == NULL);
            }
            yajl_buf_append(buf, unescaped, strlen(unescaped));
            beg = ++end;
        } else {
            end++;
        }
    }
    yajl_buf_append(buf, str + beg, end - beg);
}


/* Source: yajl_gen.c */
typedef enum {
    yajl_gen_start,
    yajl_gen_map_start,
    yajl_gen_map_key,
    yajl_gen_map_val,
    yajl_gen_array_start,
    yajl_gen_in_array,
    yajl_gen_complete,
    yajl_gen_error
} yajl_gen_state;

struct yajl_gen_t
{
    unsigned int depth;
    unsigned int pretty;
    const char * indentString;
    yajl_gen_state state[YAJL_MAX_DEPTH];
    yajl_buf buf;
};

yajl_gen
yajl_gen_alloc(const yajl_gen_config * config)
{
    yajl_gen g = (yajl_gen) malloc(sizeof(struct yajl_gen_t));
    memset((void *) g, 0, sizeof(struct yajl_gen_t));
    if (config) {
        g->pretty = config->beautify;
        g->indentString = config->indentString ? config->indentString : "  ";
    }
    g->buf = yajl_buf_alloc();
    return g;
}

void
yajl_gen_free(yajl_gen g)
{
    yajl_buf_free(g->buf);
    free(g);
}

#define INSERT_SEP \
    if (g->state[g->depth] == yajl_gen_map_key ||               \
        g->state[g->depth] == yajl_gen_in_array) {              \
        yajl_buf_append(g->buf, ",", 1);                        \
        if (g->pretty) yajl_buf_append(g->buf, "\n", 1);        \
    } else if (g->state[g->depth] == yajl_gen_map_val) {        \
        yajl_buf_append(g->buf, ":", 1);                        \
        if (g->pretty) yajl_buf_append(g->buf, " ", 1);         \
   }

#define INSERT_WHITESPACE                                              \
    if (g->pretty) {                                                    \
        if (g->state[g->depth] != yajl_gen_map_val) {                   \
            unsigned int i;                                             \
            for (i=0;i<g->depth;i++)                                    \
                yajl_buf_append(g->buf, g->indentString,                \
                                strlen(g->indentString));               \
        }                                                               \
    }

#define ENSURE_NOT_KEY \
    if (g->state[g->depth] == yajl_gen_map_key) {   \
        return yajl_gen_keys_must_be_strings;       \
    }                                               \

/* check that we're not complete, or in error state.  in a valid state
 * to be generating */
#define ENSURE_VALID_STATE \
    if (g->state[g->depth] == yajl_gen_error) {   \
        return yajl_gen_in_error_state;\
    } else if (g->state[g->depth] == yajl_gen_complete) {   \
        return yajl_gen_generation_complete;                \
    }

#define INCREMENT_DEPTH \
    if (++(g->depth) >= YAJL_MAX_DEPTH) return yajl_max_depth_exceeded;

#define APPENDED_ATOM \
    switch (g->state[g->depth]) {                   \
        case yajl_gen_start:                        \
            g->state[g->depth] = yajl_gen_complete; \
            break;                                  \
        case yajl_gen_map_start:                    \
        case yajl_gen_map_key:                      \
            g->state[g->depth] = yajl_gen_map_val;  \
            break;                                  \
        case yajl_gen_array_start:                  \
            g->state[g->depth] = yajl_gen_in_array; \
            break;                                  \
        case yajl_gen_map_val:                      \
            g->state[g->depth] = yajl_gen_map_key;  \
            break;                                  \
        default:                                    \
            break;                                  \
    }                                               \

#define FINAL_NEWLINE                                        \
    if (g->pretty && g->state[g->depth] == yajl_gen_complete) \
        yajl_buf_append(g->buf, "\n", 1);

yajl_gen_status
yajl_gen_integer(yajl_gen g, long int number)
{
    char i[32];
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    sprintf(i, "%ld", number);
    yajl_buf_append(g->buf, i, strlen(i));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_double(yajl_gen g, double number)
{
    char i[32];
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    sprintf(i, "%g", number);
    yajl_buf_append(g->buf, i, strlen(i));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_number(yajl_gen g, const char * s, unsigned int l)
{
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    yajl_buf_append(g->buf, s, l);
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_string(yajl_gen g, const unsigned char * str,
                unsigned int len)
{
    ENSURE_VALID_STATE; INSERT_SEP; INSERT_WHITESPACE;
    yajl_buf_append(g->buf, "\"", 1);
    yajl_string_encode(g->buf, str, len);
    yajl_buf_append(g->buf, "\"", 1);
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_null(yajl_gen g)
{
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    yajl_buf_append(g->buf, "null", strlen("null"));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_bool(yajl_gen g, int boolean)
{
    const char * val = boolean ? "true" : "false";

	ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    yajl_buf_append(g->buf, val, strlen(val));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_map_open(yajl_gen g)
{
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    INCREMENT_DEPTH;

    g->state[g->depth] = yajl_gen_map_start;
    yajl_buf_append(g->buf, "{", 1);
    if (g->pretty) yajl_buf_append(g->buf, "\n", 1);
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_map_close(yajl_gen g)
{
    ENSURE_VALID_STATE;
    (g->depth)--;
    if (g->pretty) yajl_buf_append(g->buf, "\n", 1);
    APPENDED_ATOM;
    INSERT_WHITESPACE;
    yajl_buf_append(g->buf, "}", 1);
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_array_open(yajl_gen g)
{
    ENSURE_VALID_STATE; ENSURE_NOT_KEY; INSERT_SEP; INSERT_WHITESPACE;
    INCREMENT_DEPTH;
    g->state[g->depth] = yajl_gen_array_start;
    yajl_buf_append(g->buf, "[", 1);
    if (g->pretty) yajl_buf_append(g->buf, "\n", 1);
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_array_close(yajl_gen g)
{
    ENSURE_VALID_STATE;
    if (g->pretty) yajl_buf_append(g->buf, "\n", 1);
    (g->depth)--;
    APPENDED_ATOM;
    INSERT_WHITESPACE;
    yajl_buf_append(g->buf, "]", 1);
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status
yajl_gen_get_buf(yajl_gen g, const unsigned char ** buf,
                 unsigned int * len)
{
    *buf = yajl_buf_data(g->buf);
    *len = yajl_buf_len(g->buf);
    return yajl_gen_status_ok;
}

void
yajl_gen_clear(yajl_gen g)
{
    yajl_buf_clear(g->buf);
}


/* yajl_lex.c */
#ifdef YAJL_LEXER_DEBUG
static const char *
tokToStr(yajl_tok tok)
{
    switch (tok) {
        case yajl_tok_bool: return "bool";
        case yajl_tok_colon: return "colon";
        case yajl_tok_comma: return "comma";
        case yajl_tok_eof: return "eof";
        case yajl_tok_error: return "error";
        case yajl_tok_left_brace: return "brace";
        case yajl_tok_left_bracket: return "bracket";
        case yajl_tok_null: return "null";
        case yajl_tok_integer: return "integer";
        case yajl_tok_double: return "double";
        case yajl_tok_right_brace: return "brace";
        case yajl_tok_right_bracket: return "bracket";
        case yajl_tok_string: return "string";
        case yajl_tok_string_with_escapes: return "string_with_escapes";
    }
    return "unknown";
}
#endif

/* Impact of the stream parsing feature on the lexer:
 *
 * YAJL support stream parsing.  That is, the ability to parse the first
 * bits of a chunk of JSON before the last bits are available (still on
 * the network or disk).  This makes the lexer more complex.  The
 * responsibility of the lexer is to handle transparently the case where
 * a chunk boundary falls in the middle of a token.  This is
 * accomplished is via a buffer and a character reading abstraction.
 *
 * Overview of implementation
 *
 * When we lex to end of input string before end of token is hit, we
 * copy all of the input text composing the token into our lexBuf.
 *
 * Every time we read a character, we do so through the readChar function.
 * readChar's responsibility is to handle pulling all chars from the buffer
 * before pulling chars from input text
 */

struct yajl_lexer_t {
    /* the overal line and char offset into the data */
    unsigned int lineOff;
    unsigned int charOff;

    /* error */
    yajl_lex_error error;

    /* a input buffer to handle the case where a token is spread over
     * multiple chunks */
    yajl_buf buf;

    /* in the case where we have data in the lexBuf, bufOff holds
     * the current offset into the lexBuf. */
    unsigned int bufOff;

    /* are we using the lex buf? */
    unsigned int bufInUse;

    /* shall we allow comments? */
    unsigned int allowComments;

    /* shall we validate utf8 inside strings? */
    unsigned int validateUTF8;
};

static unsigned char
readChar(yajl_lexer lxr, const unsigned char * txt, unsigned int *off)
{
    if (lxr->bufInUse && yajl_buf_len(lxr->buf) &&
        lxr->bufOff < yajl_buf_len(lxr->buf))
    {
        return *((unsigned char *) yajl_buf_data(lxr->buf) + (lxr->bufOff)++);
    }
    return txt[(*off)++];
}

static void
unreadChar(yajl_lexer lxr, unsigned int *off)
{
    if (*off > 0) (*off)--;
    else (lxr->bufOff)--;
}

yajl_lexer
yajl_lex_alloc(unsigned int allowComments, unsigned int validateUTF8)
{
    yajl_lexer lxr = (yajl_lexer) calloc(1, sizeof(struct yajl_lexer_t));
    lxr->buf = yajl_buf_alloc();
    lxr->allowComments = allowComments;
    lxr->validateUTF8 = validateUTF8;
    return lxr;
}

void
yajl_lex_free(yajl_lexer lxr)
{
    yajl_buf_free(lxr->buf);
    free(lxr);
    return;
}

/* a lookup table which lets us quickly determine three things:
 * VEC - valid escaped conrol char
 * IJC - invalid json char
 * VHC - valid hex char
 * note.  the solidus '/' may be escaped or not.
 * note.  the
 */
#define VEC 1
#define IJC 2
#define VHC 4
const static char charLookupTable[256] =
{
/*00*/ IJC    , IJC    , IJC    , IJC    , IJC    , IJC    , IJC    , IJC    ,
/*08*/ IJC    , IJC    , IJC    , IJC    , IJC    , IJC    , IJC    , IJC    ,
/*10*/ IJC    , IJC    , IJC    , IJC    , IJC    , IJC    , IJC    , IJC    ,
/*18*/ IJC    , IJC    , IJC    , IJC    , IJC    , IJC    , IJC    , IJC    ,

/*20*/ 0      , 0      , VEC|IJC, 0      , 0      , 0      , 0      , 0      ,
/*28*/ 0      , 0      , 0      , 0      , 0      , 0      , 0      , VEC    ,
/*30*/ VHC    , VHC    , VHC    , VHC    , VHC    , VHC    , VHC    , VHC    ,
/*38*/ VHC    , VHC    , 0      , 0      , 0      , 0      , 0      , 0      ,

/*40*/ 0      , VHC    , VHC    , VHC    , VHC    , VHC    , VHC    , 0      ,
/*48*/ 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
/*50*/ 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
/*58*/ 0      , 0      , 0      , 0      , VEC|IJC, 0      , 0      , 0      ,

/*60*/ 0      , VHC    , VEC|VHC, VHC    , VHC    , VHC    , VEC|VHC, 0      ,
/*68*/ 0      , 0      , 0      , 0      , 0      , 0      , VEC    , 0      ,
/*70*/ 0      , 0      , VEC    , 0      , VEC    , 0      , 0      , 0      ,
/*78*/ 0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,

/* include these so we don't have to always check the range of the char */
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,

       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,

       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,

       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0      ,
       0      , 0      , 0      , 0      , 0      , 0      , 0      , 0
};

/** process a variable length utf8 encoded codepoint.
 *
 *  returns:
 *    yajl_tok_string - if valid utf8 char was parsed and offset was
 *                      advanced
 *    yajl_tok_eof - if end of input was hit before validation could
 *                   complete
 *    yajl_tok_error - if invalid utf8 was encountered
 *
 *  NOTE: on error the offset will point to the first char of the
 *  invalid utf8 */
#define UTF8_CHECK_EOF if (*offset >= jsonTextLen) { return yajl_tok_eof; }

static yajl_tok
yajl_lex_utf8_char(yajl_lexer lexer, const unsigned char * jsonText,
                   unsigned int jsonTextLen, unsigned int * offset,
                   unsigned char curChar)
{
    if (curChar <= 0x7f) {
        /* single byte */
        return yajl_tok_string;
    } else if ((curChar >> 5) == 0x6) {
        /* two byte */
        UTF8_CHECK_EOF;
        curChar = readChar(lexer, jsonText, offset);
        if ((curChar >> 6) == 0x2) return yajl_tok_string;
    } else if ((curChar >> 4) == 0x0d) {
        /* three byte */
        UTF8_CHECK_EOF;
        curChar = readChar(lexer, jsonText, offset);
        if ((curChar >> 6) == 0x2) {
            UTF8_CHECK_EOF;
            curChar = readChar(lexer, jsonText, offset);
            if ((curChar >> 6) == 0x2) return yajl_tok_string;
        }
    } else if ((curChar >> 3) == 0x1d) {
        /* four byte */
        UTF8_CHECK_EOF;
        curChar = readChar(lexer, jsonText, offset);
        if ((curChar >> 6) == 0x2) {
            UTF8_CHECK_EOF;
            curChar = readChar(lexer, jsonText, offset);
            if ((curChar >> 6) == 0x2) {
                UTF8_CHECK_EOF;
                curChar = readChar(lexer, jsonText, offset);
                if ((curChar >> 6) == 0x2) return yajl_tok_string;
            }
        }
    }

    return yajl_tok_error;
}

/* lex a string.  input is the lexer, pointer to beginning of
 * json text, and start of string (offset).
 * a token is returned which has the following meanings:
 * yajl_tok_string: lex of string was successful.  offset points to
 *                  terminating '"'.
 * yajl_tok_eof: end of text was encountered before we could complete
 *               the lex.
 * yajl_tok_error: embedded in the string were unallowable chars.  offset
 *               points to the offending char
 */
#define STR_CHECK_EOF \
if (*offset >= jsonTextLen) { \
   tok = yajl_tok_eof; \
   goto finish_string_lex; \
}

static yajl_tok
yajl_lex_string(yajl_lexer lexer, const unsigned char * jsonText,
                unsigned int jsonTextLen, unsigned int * offset)
{
    yajl_tok tok = yajl_tok_error;
    int hasEscapes = 0;

    for (;;) {
		unsigned char curChar;

		STR_CHECK_EOF;

        curChar = readChar(lexer, jsonText, offset);

        /* quote terminates */
        if (curChar == '"') {
            tok = yajl_tok_string;
            break;
        }
        /* backslash escapes a set of control chars, */
        else if (curChar == '\\') {
            hasEscapes = 1;
            STR_CHECK_EOF;

            /* special case \u */
            curChar = readChar(lexer, jsonText, offset);
            if (curChar == 'u') {
                unsigned int i = 0;

                for (i=0;i<4;i++) {
                    STR_CHECK_EOF;
                    curChar = readChar(lexer, jsonText, offset);
                    if (!(charLookupTable[curChar] & VHC)) {
                        /* back up to offending char */
                        unreadChar(lexer, offset);
                        lexer->error = yajl_lex_string_invalid_hex_char;
                        goto finish_string_lex;
                    }
                }
            } else if (!(charLookupTable[curChar] & VEC)) {
                /* back up to offending char */
                unreadChar(lexer, offset);
                lexer->error = yajl_lex_string_invalid_escaped_char;
                goto finish_string_lex;
            }
        }
        /* when not validating UTF8 it's a simple table lookup to determine
         * if the present character is invalid */
        else if(charLookupTable[curChar] & IJC) {
            /* back up to offending char */
            unreadChar(lexer, offset);
            lexer->error = yajl_lex_string_invalid_json_char;
            goto finish_string_lex;
        }
        /* when in validate UTF8 mode we need to do some extra work */
        else if (lexer->validateUTF8) {
            yajl_tok t = yajl_lex_utf8_char(lexer, jsonText, jsonTextLen,
                                            offset, curChar);

            if (t == yajl_tok_eof) {
                tok = yajl_tok_eof;
                goto finish_string_lex;
            } else if (t == yajl_tok_error) {
                lexer->error = yajl_lex_string_invalid_utf8;
                goto finish_string_lex;
            }
        }
        /* accept it, and move on */
    }
  finish_string_lex:
    /* tell our buddy, the parser, wether he needs to process this string
     * again */
    if (hasEscapes && tok == yajl_tok_string) {
        tok = yajl_tok_string_with_escapes;
    }

    return tok;
}

#define RETURN_IF_EOF if (*offset >= jsonTextLen) return yajl_tok_eof;

static yajl_tok
yajl_lex_number(yajl_lexer lexer, const unsigned char * jsonText,
                unsigned int jsonTextLen, unsigned int * offset)
{
    /** XXX: numbers are the only entities in json that we must lex
     *       _beyond_ in order to know that they are complete.  There
     *       is an ambiguous case for integers at EOF. */

    unsigned char c;

    yajl_tok tok = yajl_tok_integer;

    RETURN_IF_EOF;
    c = readChar(lexer, jsonText, offset);

    /* optional leading minus */
    if (c == '-') {
        RETURN_IF_EOF;
        c = readChar(lexer, jsonText, offset);
    }

    /* a single zero, or a series of integers */
    if (c == '0') {
        RETURN_IF_EOF;
        c = readChar(lexer, jsonText, offset);
    } else if (c >= '1' && c <= '9') {
        do {
            RETURN_IF_EOF;
            c = readChar(lexer, jsonText, offset);
        } while (c >= '0' && c <= '9');
    } else {
        unreadChar(lexer, offset);
        lexer->error = yajl_lex_missing_integer_after_minus;
        return yajl_tok_error;
    }

    /* optional fraction (indicates this is floating point) */
    if (c == '.') {
        int numRd = 0;

        RETURN_IF_EOF;
        c = readChar(lexer, jsonText, offset);

        while (c >= '0' && c <= '9') {
            numRd++;
            RETURN_IF_EOF;
            c = readChar(lexer, jsonText, offset);
        }

        if (!numRd) {
            unreadChar(lexer, offset);
            lexer->error = yajl_lex_missing_integer_after_decimal;
            return yajl_tok_error;
        }
        tok = yajl_tok_double;
    }

    /* optional exponent (indicates this is floating point) */
    if (c == 'e' || c == 'E') {
        RETURN_IF_EOF;
        c = readChar(lexer, jsonText, offset);

        /* optional sign */
        if (c == '+' || c == '-') {
            RETURN_IF_EOF;
            c = readChar(lexer, jsonText, offset);
        }

        if (c >= '0' && c <= '9') {
            do {
                RETURN_IF_EOF;
                c = readChar(lexer, jsonText, offset);
            } while (c >= '0' && c <= '9');
        } else {
            unreadChar(lexer, offset);
            lexer->error = yajl_lex_missing_integer_after_exponent;
            return yajl_tok_error;
        }
        tok = yajl_tok_double;
    }

    /* we always go "one too far" */
    unreadChar(lexer, offset);

    return tok;
}

static yajl_tok
yajl_lex_comment(yajl_lexer lexer, const unsigned char * jsonText,
                 unsigned int jsonTextLen, unsigned int * offset)
{
    unsigned char c;

    yajl_tok tok = yajl_tok_comment;

    RETURN_IF_EOF;
    c = readChar(lexer, jsonText, offset);

    /* either slash or star expected */
    if (c == '/') {
        /* now we throw away until end of line */
        do {
            RETURN_IF_EOF;
            c = readChar(lexer, jsonText, offset);
        } while (c != '\n');
    } else if (c == '*') {
        /* now we throw away until end of comment */
        for (;;) {
            RETURN_IF_EOF;
            c = readChar(lexer, jsonText, offset);
            if (c == '*') {
                RETURN_IF_EOF;
                c = readChar(lexer, jsonText, offset);
                if (c == '/') {
                    break;
                } else {
                    unreadChar(lexer, offset);
                }
            }
        }
    } else {
        lexer->error = yajl_lex_invalid_char;
        tok = yajl_tok_error;
    }

    return tok;
}

yajl_tok
yajl_lex_lex(yajl_lexer lexer, const unsigned char * jsonText,
             unsigned int jsonTextLen, unsigned int * context,
             const unsigned char ** outBuf, unsigned int * outLen)
{
    yajl_tok tok = yajl_tok_error;
    unsigned char c;
    unsigned int startCtx = *context;

    *outBuf = NULL;
    *outLen = 0;

    for (;;) {
        assert(*context <= jsonTextLen);

        if (*context >= jsonTextLen) {
            tok = yajl_tok_eof;
            goto lexed;
        }

        c = readChar(lexer, jsonText, context);

        switch (c) {
            case '{':
                tok = yajl_tok_left_bracket;
                goto lexed;
            case '}':
                tok = yajl_tok_right_bracket;
                goto lexed;
            case '[':
                tok = yajl_tok_left_brace;
                goto lexed;
            case ']':
                tok = yajl_tok_right_brace;
                goto lexed;
            case ',':
                tok = yajl_tok_comma;
                goto lexed;
            case ':':
                tok = yajl_tok_colon;
                goto lexed;
            case '\t': case '\n': case '\v': case '\f': case '\r': case ' ':
                startCtx++;
                break;
            case 't': {
                const char * want = "rue";
                do {
                    if (*context >= jsonTextLen) {
                        tok = yajl_tok_eof;
                        goto lexed;
                    }
                    c = readChar(lexer, jsonText, context);
                    if (c != *want) {
                        unreadChar(lexer, context);
                        lexer->error = yajl_lex_invalid_string;
                        tok = yajl_tok_error;
                        goto lexed;
                    }
                } while (*(++want));
                tok = yajl_tok_bool;
                goto lexed;
            }
            case 'f': {
                const char * want = "alse";
                do {
                    if (*context >= jsonTextLen) {
                        tok = yajl_tok_eof;
                        goto lexed;
                    }
                    c = readChar(lexer, jsonText, context);
                    if (c != *want) {
                        unreadChar(lexer, context);
                        lexer->error = yajl_lex_invalid_string;
                        tok = yajl_tok_error;
                        goto lexed;
                    }
                } while (*(++want));
                tok = yajl_tok_bool;
                goto lexed;
            }
            case 'n': {
                const char * want = "ull";
                do {
                    if (*context >= jsonTextLen) {
                        tok = yajl_tok_eof;
                        goto lexed;
                    }
                    c = readChar(lexer, jsonText, context);
                    if (c != *want) {
                        unreadChar(lexer, context);
                        lexer->error = yajl_lex_invalid_string;
                        tok = yajl_tok_error;
                        goto lexed;
                    }
                } while (*(++want));
                tok = yajl_tok_null;
                goto lexed;
            }
            case '"': {
                tok = yajl_lex_string(lexer, (unsigned char *) jsonText,
                                      jsonTextLen, context);
                goto lexed;
            }
            case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                /* integer parsing wants to start from the beginning */
                unreadChar(lexer, context);
                tok = yajl_lex_number(lexer, (unsigned char *) jsonText,
                                      jsonTextLen, context);
                goto lexed;
            }
            case '/':
                /* hey, look, a probable comment!  If comments are disabled
                 * it's an error. */
                if (!lexer->allowComments) {
                    unreadChar(lexer, context);
                    lexer->error = yajl_lex_unallowed_comment;
                    tok = yajl_tok_error;
                    goto lexed;
                }
                /* if comments are enabled, then we should try to lex
                 * the thing.  possible outcomes are
                 * - successful lex (tok_comment, which means continue),
                 * - malformed comment opening (slash not followed by
                 *   '*' or '/') (tok_error)
                 * - eof hit. (tok_eof) */
                tok = yajl_lex_comment(lexer, (unsigned char *) jsonText,
                                       jsonTextLen, context);
                if (tok == yajl_tok_comment) {
                    /* "error" is silly, but that's the initial
                     * state of tok.  guilty until proven innocent. */
                    tok = yajl_tok_error;
                    yajl_buf_clear(lexer->buf);
                    lexer->bufInUse = 0;
                    startCtx = *context;
                    break;
                }
                /* hit error or eof, bail */
                goto lexed;
            default:
                lexer->error = yajl_lex_invalid_char;
                tok = yajl_tok_error;
                goto lexed;
        }
    }


  lexed:
    /* need to append to buffer if the buffer is in use or
     * if it's an EOF token */
    if (tok == yajl_tok_eof || lexer->bufInUse) {
        if (!lexer->bufInUse) yajl_buf_clear(lexer->buf);
        lexer->bufInUse = 1;
        yajl_buf_append(lexer->buf, jsonText + startCtx, *context - startCtx);
        lexer->bufOff = 0;

        if (tok != yajl_tok_eof) {
            *outBuf = yajl_buf_data(lexer->buf);
            *outLen = yajl_buf_len(lexer->buf);
            lexer->bufInUse = 0;
        }
    } else if (tok != yajl_tok_error) {
        *outBuf = jsonText + startCtx;
        *outLen = *context - startCtx;
    }

    /* special case for strings. skip the quotes. */
    if (tok == yajl_tok_string || tok == yajl_tok_string_with_escapes)
    {
        assert(*outLen >= 2);
        (*outBuf)++;
        *outLen -= 2;
    }


#ifdef YAJL_LEXER_DEBUG
    if (tok == yajl_tok_error) {
        printf("lexical error: %s\n",
               yajl_lex_error_to_string(yajl_lex_get_error(lexer)));
    } else if (tok == yajl_tok_eof) {
        printf("EOF hit\n");
    } else {
        printf("lexed %s: '", tokToStr(tok));
        fwrite(*outBuf, 1, *outLen, stdout);
        printf("'\n");
    }
#endif

    return tok;
}

const char *
yajl_lex_error_to_string(yajl_lex_error error)
{
    switch (error) {
        case yajl_lex_e_ok:
            return "ok, no error";
        case yajl_lex_string_invalid_utf8:
            return "invalid bytes in UTF8 string.";
        case yajl_lex_string_invalid_escaped_char:
            return "inside a string, '\\' occurs before a character "
                   "which it may not.";
        case yajl_lex_string_invalid_json_char:
            return "invalid character inside string.";
        case yajl_lex_string_invalid_hex_char:
            return "invalid (non-hex) character occurs after '\\u' inside "
                   "string.";
        case yajl_lex_invalid_char:
            return "invalid char in json text.";
        case yajl_lex_invalid_string:
            return "invalid string in json text.";
        case yajl_lex_missing_integer_after_exponent:
            return "malformed number, a digit is required after the exponent.";
        case yajl_lex_missing_integer_after_decimal:
            return "malformed number, a digit is required after the "
                   "decimal point.";
        case yajl_lex_missing_integer_after_minus:
            return "malformed number, a digit is required after the "
                   "minus sign.";
        case yajl_lex_unallowed_comment:
            return "probable comment found in input text, comments are "
                   "not enabled.";
    }
    return "unknown error code";
}


/** allows access to more specific information about the lexical
 *  error when yajl_lex_lex returns yajl_tok_error. */
yajl_lex_error
yajl_lex_get_error(yajl_lexer lexer)
{
    if (lexer == NULL) return (yajl_lex_error) -1;
    return lexer->error;
}

unsigned int yajl_lex_current_line(yajl_lexer lexer)
{
    return lexer->lineOff;
}

unsigned int yajl_lex_current_char(yajl_lexer lexer)
{
    return lexer->charOff;
}

yajl_tok yajl_lex_peek(yajl_lexer lexer, const unsigned char * jsonText,
                       unsigned int jsonTextLen, unsigned int offset)
{
    const unsigned char * outBuf;
    unsigned int outLen;
    unsigned int bufLen = yajl_buf_len(lexer->buf);
    unsigned int bufOff = lexer->bufOff;
    unsigned int bufInUse = lexer->bufInUse;
    yajl_tok tok;

    tok = yajl_lex_lex(lexer, jsonText, jsonTextLen, &offset,
                       &outBuf, &outLen);

    lexer->bufOff = bufOff;
    lexer->bufInUse = bufInUse;
    yajl_buf_truncate(lexer->buf, bufLen);

    return tok;
}


/* Source: yajl_parser.c */
unsigned char *
yajl_render_error_string(yajl_handle hand, const unsigned char * jsonText,
                         unsigned int jsonTextLen, int verbose)
{
    unsigned int offset = hand->errorOffset;
    unsigned char * str;
    const char * errorType = NULL;
    const char * errorText = NULL;
    char text[72];
    const char * arrow = "                     (right here) ------^\n";

    if (yajl_state_current(hand) == yajl_state_parse_error) {
        errorType = "parse";
        errorText = hand->parseError;
    } else if (yajl_state_current(hand) == yajl_state_lexical_error) {
        errorType = "lexical";
        errorText = yajl_lex_error_to_string(yajl_lex_get_error(hand->lexer));
    } else {
        errorType = "unknown";
    }

    {
        unsigned int memneeded = 0;
        memneeded += strlen(errorType);
        memneeded += strlen(" error");
        if (errorText != NULL) {
            memneeded += strlen(": ");
            memneeded += strlen(errorText);
        }
        str = (unsigned char *) malloc(memneeded + 2);
        str[0] = 0;
        strcat((char *) str, errorType);
        strcat((char *) str, " error");
        if (errorText != NULL) {
            strcat((char *) str, ": ");
            strcat((char *) str, errorText);
        }
        strcat((char *) str, "\n");
    }

    /* now we append as many spaces as needed to make sure the error
     * falls at char 41, if verbose was specified */
    if (verbose) {
        unsigned int start, end, i;
        unsigned int spacesNeeded;

        spacesNeeded = (offset < 30 ? 40 - offset : 10);
        start = (offset >= 30 ? offset - 30 : 0);
        end = (offset + 30 > jsonTextLen ? jsonTextLen : offset + 30);

        for (i=0;i<spacesNeeded;i++) text[i] = ' ';

        for (;start < end;start++, i++) {
            if (jsonText[start] != '\n' && jsonText[start] != '\r')
            {
                text[i] = jsonText[start];
            }
            else
            {
                text[i] = ' ';
            }
        }
        assert(i <= 71);
        text[i++] = '\n';
        text[i] = 0;
        {
            char * newStr = (char *) malloc(strlen((char *) str) +
                                            strlen((char *) text) +
                                            strlen(arrow) + 1);
            newStr[0] = 0;
            strcat((char *) newStr, (char *) str);
            strcat((char *) newStr, text);
            strcat((char *) newStr, arrow);
            free(str);
            str = (unsigned char *) newStr;
        }
    }
    return str;
}

/* check for client cancelation */
#define _CC_CHK(x)                                                \
    if (!(x)) {                                                   \
        yajl_state_set(hand, yajl_state_parse_error);             \
        hand->parseError =                                        \
            "client cancelled parse via callback return value";   \
        return yajl_status_client_canceled;                       \
    }


yajl_status
yajl_do_parse(yajl_handle hand, unsigned int * offset,
              const unsigned char * jsonText, unsigned int jsonTextLen)
{
    yajl_tok tok;
    const unsigned char * buf;
    unsigned int bufLen;

  around_again:
    switch (yajl_state_current(hand)) {
        case yajl_state_parse_complete:
            return yajl_status_ok;
        case yajl_state_lexical_error:
        case yajl_state_parse_error:
            hand->errorOffset = *offset;
            return yajl_status_error;
        case yajl_state_start:
        case yajl_state_map_need_val:
        case yajl_state_array_need_val:
        case yajl_state_array_start: {
            /* for arrays and maps, we advance the state for this
             * depth, then push the state of the next depth.
             * If an error occurs during the parsing of the nesting
             * enitity, the state at this level will not matter.
             * a state that needs pushing will be anything other
             * than state_start */
            yajl_state stateToPush = yajl_state_start;

            tok = yajl_lex_lex(hand->lexer, jsonText, jsonTextLen,
                               offset, &buf, &bufLen);

            switch (tok) {
                case yajl_tok_eof:
                    return yajl_status_insufficient_data;
                case yajl_tok_error:
                    yajl_state_set(hand, yajl_state_lexical_error);
                    goto around_again;
                case yajl_tok_string:
                    if (hand->callbacks && hand->callbacks->yajl_string) {
                        _CC_CHK(hand->callbacks->yajl_string(hand->ctx,
                                                             buf, bufLen));
                    }
                    break;
                case yajl_tok_string_with_escapes:
                    if (hand->callbacks && hand->callbacks->yajl_string) {
                        yajl_buf_clear(hand->decodeBuf);
                        yajl_string_decode(hand->decodeBuf, buf, bufLen);
                        _CC_CHK(hand->callbacks->yajl_string(
                                    hand->ctx, yajl_buf_data(hand->decodeBuf),
                                    yajl_buf_len(hand->decodeBuf)));
                    }
                    break;
                case yajl_tok_bool:
                    if (hand->callbacks && hand->callbacks->yajl_boolean) {
                        _CC_CHK(hand->callbacks->yajl_boolean(hand->ctx,
                                                              *buf == 't'));
                    }
                    break;
                case yajl_tok_null:
                    if (hand->callbacks && hand->callbacks->yajl_null) {
                        _CC_CHK(hand->callbacks->yajl_null(hand->ctx));
                    }
                    break;
                case yajl_tok_left_bracket:
                    if (hand->callbacks && hand->callbacks->yajl_start_map) {
                        _CC_CHK(hand->callbacks->yajl_start_map(hand->ctx));
                    }
                    stateToPush = yajl_state_map_start;
                    break;
                case yajl_tok_left_brace:
                    if (hand->callbacks && hand->callbacks->yajl_start_array) {
                        _CC_CHK(hand->callbacks->yajl_start_array(hand->ctx));
                    }
                    stateToPush = yajl_state_array_start;
                    break;
                case yajl_tok_integer:
                    /*
                     * note.  strtol does not respect the length of
                     * the lexical token.  in a corner case where the
                     * lexed number is a integer with a trailing zero,
                     * immediately followed by the end of buffer,
                     * sscanf could run off into oblivion and cause a
                     * crash.  for this reason we copy the integer
                     * (and doubles), into our parse buffer (the same
                     * one used for unescaping strings), before
                     * calling strtol.  yajl_buf ensures null padding,
                     * so we're safe.
                     */
                    if (hand->callbacks) {
                        if (hand->callbacks->yajl_number) {
                            _CC_CHK(hand->callbacks->yajl_number(hand->ctx,
                                                                 (char *) buf,
                                                                 bufLen));
                        } else if (hand->callbacks->yajl_integer) {
                            long int i = 0;
                            yajl_buf_clear(hand->decodeBuf);
                            yajl_buf_append(hand->decodeBuf, buf, bufLen);
                            buf = yajl_buf_data(hand->decodeBuf);
                            i = strtol((char *) buf, NULL, 10);
                            if ((i == LONG_MIN || i == LONG_MAX) &&
                                errno == ERANGE)
                            {
                                yajl_state_set(hand, yajl_state_parse_error);
                                hand->parseError = "integer overflow" ;
                                /* try to restore error offset */
                                if (*offset >= bufLen) *offset -= bufLen;
                                else *offset = 0;
                                goto around_again;
                            }
                            _CC_CHK(hand->callbacks->yajl_integer(hand->ctx,
                                                                  i));
                        }
                    }
                    break;
                case yajl_tok_double:
                    if (hand->callbacks) {
                        if (hand->callbacks->yajl_number) {
                            _CC_CHK(hand->callbacks->yajl_number(hand->ctx,
                                                                 (char *) buf,
                                                                 bufLen));
                        } else if (hand->callbacks->yajl_double) {
                            double d = 0.0;
                            yajl_buf_clear(hand->decodeBuf);
                            yajl_buf_append(hand->decodeBuf, buf, bufLen);
                            buf = yajl_buf_data(hand->decodeBuf);
                            d = strtod((char *) buf, NULL);
                            if ((d == HUGE_VAL || d == -HUGE_VAL) &&
                                errno == ERANGE)
                            {
                                yajl_state_set(hand, yajl_state_parse_error);
                                hand->parseError = "numeric (floating point) "
                                    "overflow";
                                /* try to restore error offset */
                                if (*offset >= bufLen) *offset -= bufLen;
                                else *offset = 0;
                                goto around_again;
                            }
                            _CC_CHK(hand->callbacks->yajl_double(hand->ctx,
                                                                 d));
                        }
                    }
                    break;
                case yajl_tok_right_brace: {
                    if (yajl_state_current(hand) == yajl_state_array_start) {
                        if (hand->callbacks &&
                            hand->callbacks->yajl_end_array)
                        {
                            _CC_CHK(hand->callbacks->yajl_end_array(hand->ctx));
                        }
                        (void) yajl_state_pop(hand);
                        goto around_again;
                    }
                    /* intentional fall-through */
                }
                case yajl_tok_colon:
                case yajl_tok_comma:
                case yajl_tok_right_bracket:
                    yajl_state_set(hand, yajl_state_parse_error);
                    hand->parseError =
                        "unallowed token at this point in JSON text";
                    goto around_again;
                default:
                    yajl_state_set(hand, yajl_state_parse_error);
                    hand->parseError = "invalid token, internal error";
                    goto around_again;
            }
            /* got a value.  transition depends on the state we're in. */
            {
                yajl_state s = yajl_state_current(hand);
                if (s == yajl_state_start) {
                    yajl_state_set(hand, yajl_state_parse_complete);
                } else if (s == yajl_state_map_need_val) {
                    yajl_state_set(hand, yajl_state_map_got_val);
                } else {
                    yajl_state_set(hand, yajl_state_array_got_val);
                }
            }
            if (stateToPush != yajl_state_start) {
                yajl_state_push(hand, stateToPush);
            }

            goto around_again;
        }
        case yajl_state_map_start:
        case yajl_state_map_need_key: {
            /* only difference between these two states is that in
             * start '}' is valid, whereas in need_key, we've parsed
             * a comma, and a string key _must_ follow */
            tok = yajl_lex_lex(hand->lexer, jsonText, jsonTextLen,
                               offset, &buf, &bufLen);
            switch (tok) {
                case yajl_tok_eof:
                    return yajl_status_insufficient_data;
                case yajl_tok_error:
                    yajl_state_set(hand, yajl_state_lexical_error);
                    goto around_again;
                case yajl_tok_string_with_escapes:
                    if (hand->callbacks && hand->callbacks->yajl_map_key) {
                        yajl_buf_clear(hand->decodeBuf);
                        yajl_string_decode(hand->decodeBuf, buf, bufLen);
                        buf = yajl_buf_data(hand->decodeBuf);
                        bufLen = yajl_buf_len(hand->decodeBuf);
                    }
                    /* intentional fall-through */
                case yajl_tok_string:
                    if (hand->callbacks && hand->callbacks->yajl_map_key) {
                        _CC_CHK(hand->callbacks->yajl_map_key(hand->ctx, buf,
                                                              bufLen));
                    }
                    yajl_state_set(hand, yajl_state_map_sep);
                    goto around_again;
                case yajl_tok_right_bracket:
                    if (yajl_state_current(hand) == yajl_state_map_start) {
                        if (hand->callbacks && hand->callbacks->yajl_end_map) {
                            _CC_CHK(hand->callbacks->yajl_end_map(hand->ctx));
                        }
                        (void) yajl_state_pop(hand);
                        goto around_again;
                    }
                default:
                    yajl_state_set(hand, yajl_state_parse_error);
                    hand->parseError =
                        "invalid object key (must be a string)";
                    goto around_again;
            }
        }
        case yajl_state_map_sep: {
            tok = yajl_lex_lex(hand->lexer, jsonText, jsonTextLen,
                               offset, &buf, &bufLen);
            switch (tok) {
                case yajl_tok_colon:
                    yajl_state_set(hand, yajl_state_map_need_val);
                    goto around_again;
                case yajl_tok_eof:
                    return yajl_status_insufficient_data;
                case yajl_tok_error:
                    yajl_state_set(hand, yajl_state_lexical_error);
                    goto around_again;
                default:
                    yajl_state_set(hand, yajl_state_parse_error);
                    hand->parseError = "object key and value must "
                        "be separated by a colon (':')";
                    goto around_again;
            }
        }
        case yajl_state_map_got_val: {
            tok = yajl_lex_lex(hand->lexer, jsonText, jsonTextLen,
                               offset, &buf, &bufLen);
            switch (tok) {
                case yajl_tok_right_bracket:
                    if (hand->callbacks && hand->callbacks->yajl_end_map) {
                        _CC_CHK(hand->callbacks->yajl_end_map(hand->ctx));
                    }
                    (void) yajl_state_pop(hand);
                    goto around_again;
                case yajl_tok_comma:
                    yajl_state_set(hand, yajl_state_map_need_key);
                    goto around_again;
                case yajl_tok_eof:
                    return yajl_status_insufficient_data;
                case yajl_tok_error:
                    yajl_state_set(hand, yajl_state_lexical_error);
                    goto around_again;
                default:
                    yajl_state_set(hand, yajl_state_parse_error);
                    hand->parseError = "after key and value, inside map, "
                                       "I expect ',' or '}'";
                    /* try to restore error offset */
                    if (*offset >= bufLen) *offset -= bufLen;
                    else *offset = 0;
                    goto around_again;
            }
        }
        case yajl_state_array_got_val: {
            tok = yajl_lex_lex(hand->lexer, jsonText, jsonTextLen,
                               offset, &buf, &bufLen);
            switch (tok) {
                case yajl_tok_right_brace:
                    if (hand->callbacks && hand->callbacks->yajl_end_array) {
                        _CC_CHK(hand->callbacks->yajl_end_array(hand->ctx));
                    }
                    (void) yajl_state_pop(hand);
                    goto around_again;
                case yajl_tok_comma:
                    yajl_state_set(hand, yajl_state_array_need_val);
                    goto around_again;
                case yajl_tok_eof:
                    return yajl_status_insufficient_data;
                case yajl_tok_error:
                    yajl_state_set(hand, yajl_state_lexical_error);
                    goto around_again;
                default:
                    yajl_state_set(hand, yajl_state_parse_error);
                    hand->parseError =
                        "after array element, I expect ',' or ']'";
                    goto around_again;
            }
        }
    }

    abort();
    return yajl_status_error;
}

/* state stack maintenence routines */
yajl_state
yajl_state_current(yajl_handle h)
{
    assert(yajl_buf_len(h->stateBuf) > 0);
    return (yajl_state) *(yajl_buf_data(h->stateBuf) +
                          yajl_buf_len(h->stateBuf) - 1);
}

void yajl_state_push(yajl_handle h, yajl_state s)
{
    unsigned char c = (unsigned char) s;
    yajl_buf_append(h->stateBuf, &c, sizeof(c));
}

yajl_state yajl_state_pop(yajl_handle h)
{
    yajl_state s;
    unsigned int len = yajl_buf_len(h->stateBuf);
    /* start state is never popped */
    assert(len > 1);
    s = (yajl_state) *(yajl_buf_data(h->stateBuf) + len - 1);
    yajl_buf_truncate(h->stateBuf, len - 1);
    return s;
}

unsigned int yajl_parse_depth(yajl_handle h)
{
    assert(yajl_buf_len(h->stateBuf) > 0);
    return (yajl_buf_len(h->stateBuf) - 1);
}

void yajl_state_set(yajl_handle h, yajl_state state)
{
    assert(yajl_buf_len(h->stateBuf) > 0);
    *(unsigned char *) (yajl_buf_data(h->stateBuf) +
                        yajl_buf_len(h->stateBuf) - 1) = (unsigned char) state;
}


// http_client.cpp
using yidl::runtime::HeapBuffer;
using yidl::runtime::BufferedMarshaller;
using yidl::runtime::StackBuffer;
using yield::concurrency::ResponseQueue;
using yield::platform::File;
using yield::platform::Stat;
using yield::platform::Volume;


HTTPClient::HTTPClient
(
  uint16_t concurrency_level,
  Log* error_log,
  IOQueue& io_queue,
  const Time& operation_timeout,
  SocketAddress& peername,
  uint16_t reconnect_tries_max,
  TCPSocketFactory& tcp_socket_factory,
  Log* trace_log
)
  : SocketClient
    (
      error_log,
      io_queue,
      operation_timeout,
      peername,
      reconnect_tries_max,
      trace_log
    )
{
  uint16_t connection_i = 0;
  for ( ; connection_i < concurrency_level; connection_i++ )
  {
    TCPSocket* tcp_socket = tcp_socket_factory.createTCPSocket();
    if ( tcp_socket != NULL )
    {
      if ( tcp_socket->associate( io_queue ) )
        connections.enqueue( new Connection( *this, *tcp_socket ) );
      else
      {
        TCPSocket::dec_ref( *tcp_socket );
        break;
      }
    }
    else
      break;
  }

#ifdef _DEBUG
  if ( connection_i == 0 ) DebugBreak();
#endif
  TCPSocketFactory::dec_ref( tcp_socket_factory );
}

HTTPClient&
HTTPClient::create
(
  const URI& absolute_uri,
  uint16_t concurrency_level,
  Log* error_log,
  const Time& operation_timeout,
  uint16_t reconnect_tries_max,
  SSLContext* ssl_context,
  Log* trace_log
)
{
  URI checked_absolute_uri( absolute_uri );
  if ( checked_absolute_uri.get_port() == 0 )
  {
    if ( absolute_uri.get_scheme() == "https" )
      checked_absolute_uri.set_port( 443 );
    else
      checked_absolute_uri.set_port( 80 );
  }

  SocketAddress& peername = createSocketAddress( checked_absolute_uri );

  IOQueue* io_queue;
  TCPSocketFactory* tcp_socket_factory;
#ifdef YIELD_IPC_HAVE_OPENSSL
  if ( absolute_uri.get_scheme() == "https" )
  {
    io_queue = &yield::platform::NBIOQueue::create();
    if ( ssl_context == NULL )
      ssl_context = &SSLContext::create( SSLv23_client_method() );
    tcp_socket_factory = new SSLSocketFactory( *ssl_context );
    SSLContext::dec_ref( *ssl_context );
  }
  else
#endif
  {
    io_queue = &createIOQueue();
    if ( trace_log != NULL )
      tcp_socket_factory = new TracingTCPSocketFactory( *trace_log );
    else
      tcp_socket_factory = new TCPSocketFactory;
  }

  return *new HTTPClient
              (
                concurrency_level,
                error_log,
                *io_queue,
                operation_timeout,
                peername,
                reconnect_tries_max,
                *tcp_socket_factory,
                trace_log
              );
}

HTTPResponse& HTTPClient::GET( const URI& absolute_uri )
{
  return sendHTTPRequest( "GET", absolute_uri, NULL );
}

void HTTPClient::handleRequest( Request& request )
{
  if ( request.get_type_id() == HTTPRequest::TYPE_ID )
  {
    HTTPRequest& http_request = static_cast<HTTPRequest&>( request );
    Connection* connection = connections.dequeue();
    connection->send( http_request );
  }
  else
    DebugBreak();
}

HTTPResponse& HTTPClient::PUT( const URI& absolute_uri, Buffer& body )
{
  return sendHTTPRequest( "PUT", absolute_uri, &body );
}

HTTPResponse&
HTTPClient::PUT
(
  const URI& absolute_uri,
  const Path& body_file_path
)
{
  File* file = Volume().open( body_file_path );

  if ( file != NULL )
  {
    Stat* stbuf = file->stat();
    if ( stbuf != NULL )
    {
      size_t file_size = static_cast<size_t>( stbuf->get_size() );
      Stat::dec_ref( *stbuf );
      Buffer* body = new HeapBuffer( file_size );
      file->read( *body, file_size );
      File::dec_ref( *file );
      return sendHTTPRequest( "PUT", absolute_uri, body );
    }
    else
      throw Exception();
  }
  else
    throw Exception();
}

HTTPResponse&
HTTPClient::sendHTTPRequest
(
  const char* method,
  const URI& absolute_uri,
  Buffer* body
)
{
  HTTPClient& http_client = HTTPClient::create( absolute_uri );

  HTTPRequest* http_request;
  if ( body != NULL )
    http_request = new HTTPRequest( method, absolute_uri, *body );
  else
    http_request = new HTTPRequest( method, absolute_uri );

  ResponseQueue<HTTPResponse> http_response_queue;
  http_request->set_response_target( &http_response_queue );

  http_client.send( *http_request );

  try
  {
    HTTPResponse& http_response = http_response_queue.dequeue();
    HTTPClient::dec_ref( http_client );
    return http_response;
  }
  catch ( Exception& )
  {
    HTTPClient::dec_ref( http_client );
    throw;
  }
}


HTTPClient::Connection::Connection
(
  HTTPClient& http_client,
  TCPSocket& tcp_socket
)
  : SocketClient::Connection( tcp_socket, http_client )
{ }

void HTTPClient::Connection::handleRequest( Request& request )
{
  if ( request.get_type_id() == HTTPRequest::TYPE_ID )
  {
    HTTPRequest& http_request = static_cast<HTTPRequest&>( request );

    BufferedMarshaller buffered_marshaller;
    http_request.marshal( buffered_marshaller );

    if ( get_socket().is_connected() )
    {
      Buffers& send_buffers = buffered_marshaller.get_buffers().inc_ref();
      get_socket().aio_sendmsg( send_buffers, 0, *this, &http_request );
    }
    else
    {
      Buffer& send_buffer = buffered_marshaller.get_buffers().join();
      get_socket().aio_connect
      (
        get_peername(),
        *this,
        &http_request,
        &send_buffer
      );
    }
  }
  else
    Request::dec_ref( request );
}

void HTTPClient::Connection::onConnectCompletion
(
  size_t bytes_written,
  void* context
)
{
  SocketClient::Connection::onConnectCompletion( bytes_written, context );
  get_socket().aio_recv( *new StackBuffer<1024>, 0, *this, NULL );
  onWriteCompletion( bytes_written, context );
}

void
HTTPClient::Connection::onConnectError
(
  uint32_t error_code,
  void* context
)
{
  SocketClient::Connection::onConnectError( error_code, context );
}

void HTTPClient::Connection::onReadCompletion( Buffer& buffer, void* context )
{
  SocketClient::Connection::onReadCompletion( buffer, context );

  vector<HTTPResponse*> http_responses;
  HTTPResponseParser::ParseStatus parse_status
    = HTTPResponseParser::parse( buffer, http_responses );

  if ( !http_responses.empty() )
  {
    for
    (
      vector<HTTPResponse*>::iterator http_response_i = http_responses.begin();
      http_response_i != http_responses.end();
      ++http_response_i
    )
    {
      HTTPRequest* http_request = outstanding_http_requests.front();
      outstanding_http_requests.pop_front();
      http_request->respond( **http_response_i );
    }
  }

  if ( parse_status != HTTPResponseParser::PARSE_ERROR )
    get_socket().aio_recv( *new StackBuffer<1024>, 0, *this, NULL );
  else
    onReadError( 0, NULL );
}

void HTTPClient::Connection::onReadError( uint32_t error_code, void* context )
{
  SocketClient::Connection::onReadError( error_code, context );

  while ( !outstanding_http_requests.empty() )
  {
    HTTPRequest::dec_ref( *outstanding_http_requests.back() );
    outstanding_http_requests.pop_back();
  }

  DebugBreak(); // TODO: reconnect
}

void
HTTPClient::Connection::onWriteCompletion
(
  size_t bytes_written,
  void* context
)
{
  SocketClient::Connection::onWriteCompletion( bytes_written, context );

  HTTPRequest* http_request = static_cast<HTTPRequest*>( context );
  outstanding_http_requests.push_back( http_request );

  get_socket().aio_recv( *new StackBuffer<1024>, 0, *this, NULL );
}

void HTTPClient::Connection::onWriteError( uint32_t error_code, void* context )
{
  SocketClient::Connection::onWriteError( error_code, context );

  HTTPRequest* http_request = static_cast<HTTPRequest*>( context );
  http_request->respond( *new ExceptionResponse( error_code ) );
  HTTPRequest::dec_ref( *http_request );

  while ( !outstanding_http_requests.empty() )
  {
    HTTPRequest::dec_ref( *outstanding_http_requests.back() );
    outstanding_http_requests.pop_back();
  }

  DebugBreak(); // TODO: reconnect
}


// http_message.cpp
using yidl::runtime::StringBuffer;

#ifdef _WIN32
#include <windows.h> // For SYSTEMTIME
#endif


HTTPMessage::HTTPMessage()
  : body( NULL )
{
  header = new Buffers;
}

HTTPMessage::HTTPMessage
(
  Buffer& header,
  const FieldOffsets& field_offsets,
  Buffer* body
)
  : body( body ),
    header( new Buffers( header ) ),
    field_offsets( field_offsets )
{ }

HTTPMessage::HTTPMessage( Buffer& body )
 : body( &body )
{
  header = new Buffers;
}

HTTPMessage::~HTTPMessage()
{
  Buffer::dec_ref( body );
  Buffers::dec_ref( header );
}

const char*
HTTPMessage::get_field
(
  const char* name,
  const char* default_value
) const
{
  Buffer& header = this->header->join();
  const char* value = get_field( header, field_offsets, name, default_value );
  Buffer::dec_ref( header );
  return value;
}

const char*
HTTPMessage::get_field
(
  Buffer& header,
  const FieldOffsets& field_offsets,
  const char* name,
  const char* default_value
)
{
  if ( !field_offsets.empty() )
  {
    uint16_t namelen = static_cast<uint16_t>( strnlen( name, UINT16_MAX ) );

    uint16_t field_offsets_base = field_offsets[0].first;
    const char* fields_base
      = static_cast<const char*>( header ) + field_offsets_base;

    size_t field_offsets_size = field_offsets.size();

    for ( uint32_t fo_i = 0; fo_i < field_offsets_size; fo_i++ )
    {
      const FieldOffset& field_offset = field_offsets[fo_i];

      if
      (
        ( field_offset.second - field_offset.first ) >= namelen
        &&
        strcmp
        (
          fields_base + ( field_offset.first - field_offsets_base ),
          name
        ) == 0
      )
        return fields_base + ( field_offset.second - field_offsets_base );
    }
  }

  return const_cast<char*>( default_value );
}

Time HTTPMessage::get_time_field( const char* name ) const
{
  return HTTPMessageParser<HTTPResponseParser, HTTPResponse>::parse_http_date
         (
           get_field( name, NULL )
         );
}

void HTTPMessage::marshal( Marshaller& marshaller ) const
{
  marshaller.write( "header", *header );

  if ( body != NULL )
  {
    if ( get_field( "Content-Length", NULL ) == NULL )
    {
      char cl[64];
#ifdef _WIN32
      int cl_len = sprintf_s( cl, 64, "Content-Length: %u\r\n", body->size() );
#else
      int cl_len = snprintf( cl, 64, "Content-Length: %zu\r\n", body->size() );
#endif
      marshaller.write( "content_length", cl, cl_len );
    }

    marshaller.write( "crlf", "\r\n", 2 );

    marshaller.write( "body", *body );
  }
  else
    marshaller.write( "crlf", "\r\n" );

}

void HTTPMessage::set_body( Buffer* body )
{
  Buffer::dec_ref( this->body );
  this->body = body;
}

void HTTPMessage::set_field( const char* name, const char* value )
{
  header->push_back( name );
  header->push_back( ": ", 2 );
  header->push_back( value );
  header->push_back( "\r\n" );
}

void HTTPMessage::set_field( const char* name, char* value )
{
  header->push_back( name );
  header->push_back( ": ", 2 );
  header->push_back( value );
  header->push_back( "\r\n", 2 );
}

void HTTPMessage::set_field( char* name, char* value )
{
  header->push_back( name );
  header->push_back( ": ", 2 );
  header->push_back( value );
  header->push_back( "\r\n", 2 );
}

void HTTPMessage::set_field( const string& name, const string& value )
{
  header->push_back( name );
  header->push_back( ": ", 2 );
  header->push_back( value );
  header->push_back( "\r\n", 2 );
}

void HTTPMessage::set_field( const char* name, const Time& value )
{
  static const char* HTTPWeekDays[]
    = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

  static const char* HTTPMonths[]
    = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
      };

  char http_date[30];

#ifdef _WIN32
  SYSTEMTIME utc_system_time = value.as_utc_SYSTEMTIME();

  _snprintf_s
  (
    http_date,
    30,
    _TRUNCATE,
    "%s, %02d %s %04d %02d:%02d:%02d GMT",
    HTTPWeekDays[utc_system_time.wDayOfWeek],
    utc_system_time.wDay,
    HTTPMonths[utc_system_time.wMonth-1],
    utc_system_time.wYear,
    utc_system_time.wHour,
    utc_system_time.wMinute,
    utc_system_time.wSecond
  );
#else
  struct tm utc_struct_tm = value.as_utc_struct_tm();

  snprintf
  (
    http_date,
    30,
    "%s, %02d %s %04d %02d:%02d:%02d GMT",
    HTTPWeekDays[utc_struct_tm.tm_wday],
    utc_struct_tm.tm_mday,
    HTTPMonths[utc_struct_tm.tm_mon],
    utc_struct_tm.tm_year + 1900,
    utc_struct_tm.tm_hour,
    utc_struct_tm.tm_min,
    utc_struct_tm.tm_sec
  );
#endif

  set_field( name, http_date );
}


// http_message_parser.cpp
using yidl::runtime::HeapBuffer;
using yidl::runtime::StringBuffer;
using yidl::runtime::SubBuffer;

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4702 )
#endif














template <class HTTPMessageParserType, class HTTPMessageType>
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::HTTPMessageParser()
{
  header = NULL;
  body = NULL;
}

template <class HTTPMessageParserType, class HTTPMessageType>
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::~HTTPMessageParser()
{
  Buffer::dec_ref( header );
  Buffer::dec_ref( body );
}

template <class HTTPMessageParserType, class HTTPMessageType>
HTTPMessageType*
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::createHTTPMessage
(
  Buffer* body
)
{
  return static_cast<HTTPMessageParserType*>( this )
           ->createHTTPMessage( *header, field_offsets, body );
}

template <class HTTPMessageParserType, class HTTPMessageType>
size_t
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::get_content_length
(
  Buffer& header,
  const HTTPMessage::FieldOffsets&
)
{
  const char* content_length_string
    = HTTPMessage::get_field
      (
        header,
        field_offsets,
        "Content-Length",
        NULL
      );

  if ( content_length_string == NULL )
  {
    content_length_string
      = HTTPMessage::get_field
        (
          header,
          field_offsets,
          "Content-length"
        );
  }

  return static_cast<size_t>( atoi( content_length_string ) );
}

template <class HTTPMessageParserType, class HTTPMessageType>
HTTPMessageType*
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::parse
(
  const string& buffer
)
{
  Buffer* string_buffer = new StringBuffer( buffer );
  HTTPMessageType* http_message = parse( *string_buffer );
  Buffer::dec_ref( *string_buffer );
  return http_message;
}

template <class HTTPMessageParserType, class HTTPMessageType>
HTTPMessageType*
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::parse
(
  Buffer& buffer
)
{
  vector<HTTPMessageType*> messages;
  parse( buffer, messages );
  if ( messages.size() == 1 )
    return messages[0];
  else
  {
    while ( !messages.empty() )
    {
      HTTPMessageType::dec_ref( *messages.back() );
      messages.pop_back();
    }

    return NULL;
  }
}

template <class HTTPMessageParserType, class HTTPMessageType>
typename HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::ParseStatus
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::parse
(
  const string& buffer,
  vector<HTTPMessageType*>& out_http_messages
)
{
  Buffer* string_buffer = new StringBuffer( buffer );
  ParseStatus parse_status = parse( *string_buffer, out_http_messages );
  Buffer::dec_ref( *string_buffer );
  return parse_status;
}

template <class HTTPMessageParserType, class HTTPMessageType>
typename HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::ParseStatus
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::parse
(
  Buffer& buffer,
  vector<HTTPMessageType*>& out_http_messages
)
{
#ifdef _DEBUG
  if ( buffer.position() == buffer.size() ) DebugBreak();
#endif

  if ( body == NULL ) // Reading header
  {
    if ( header == NULL )
    {
      switch ( parse_header( buffer ) )
      {
        case PARSE_COMPLETE:
        {
          header = &buffer.inc_ref();
        }
        break;

        case PARSE_WANT_READ:
        {
          header = new StringBuffer( buffer );
          return PARSE_WANT_READ;
        }

        case PARSE_ERROR: return PARSE_ERROR;
      }
    }
    else
    {
      header->put( buffer ); // header must be a StringBuffer

      switch ( parse_header( *header ) )
      {
        case PARSE_COMPLETE: break;
        case PARSE_WANT_READ: return PARSE_WANT_READ;
        case PARSE_ERROR:
        {
          Buffer::dec_ref( *header );
          header = NULL;
          return PARSE_ERROR;
        }
      }
    }

    // Here the header has been parsed successfully and header has been
    // set to the header buffer. This is a "new" reference that must be
    // dec_ref'd.

    size_t content_length = get_content_length( *header, field_offsets );

    if ( content_length == 0 ) // No body, done reading this message
    {
      const char* transfer_encoding
        = HTTPMessage::get_field
          (
            *header,
            field_offsets,
            "Transfer-Encoding",
            NULL
          );

      if ( transfer_encoding == NULL ) // No body
      {
        out_http_messages.push_back( createHTTPMessage() );
        field_offsets.clear();

        if ( header->position() == header->size() )
        {
          Buffer::dec_ref( *header );
          header = NULL;
          return PARSE_COMPLETE;
        }
        else
        {
          Buffer* buffer = header;
          header = NULL;
          return parse( *buffer, out_http_messages );
        }
      }
      else if ( strcmp( transfer_encoding, "chunked" ) == 0 )
      {
        body = new StringBuffer;
        chunked_body = true;

        if ( header->position() == header->size() )
          return PARSE_WANT_READ;
        else
          return parse( *header, out_http_messages );
      }
      else
        return PARSE_ERROR;
    }
    else // Have a body
    {
      chunked_body = false;

      if ( header->position() < header->size() )
      {
        // At least some of the body is in header
        if ( header->size() - header->position() >= content_length )
        {
          // All of the body is in header

          out_http_messages.push_back
          (
            createHTTPMessage
            (
              new SubBuffer
                  (
                    *header,
                    header->position(),
                    content_length,
                    content_length
                  )
            )
          );

          header->position( header->position() + content_length );

          if ( header->position() == header->size() ) // Consumed header
          {
            reset();
            return PARSE_COMPLETE;
          }
          else // There may be another message in header
          {
            field_offsets.clear();
            Buffer* buffer = header;
            header = NULL;
            return parse( *buffer, out_http_messages );
          }
        }
        else
        {
          // Only part of the body is in header
          body = new HeapBuffer( content_length );
          body->put( *header ); // Copies from position()
          // header and field_offsets stay set
          return PARSE_WANT_READ;
        }
      }
      else // None of the body is in header
      {
        body = new HeapBuffer( content_length );
        // header and field_offsets stay set
        return PARSE_WANT_READ;
      }
    } // else have a body
  }
  else // Reading body
  {
#ifdef _DEBUG
    if ( body->size() == body->capacity() ) DebugBreak();
    if ( header == NULL || field_offsets.empty() ) DebugBreak();
#endif

    if ( &buffer != body )
      body->put( buffer );

    if ( chunked_body )
    {
      // Variables used by Ragel
      int cs;
      char *p = static_cast<char*>( *body ) + body->position(),
           *ps = p,
           *pe = p + ( body->size() - body->position() ),
           *eof = pe;

      // Variables used by machine actions;
      char *chunk_data_p = NULL, *chunk_size_p = NULL;
      size_t chunk_size = 0, seen_chunk_size = 0;
      uint16_t field_name_offset = 0, field_value_offset = 0;

static const char _http_chunk_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1,
	3, 1, 4, 1, 5, 1, 6, 1,
	8, 1, 9, 2, 0, 6, 2, 2,
	3, 2, 7, 5
};

static const char _http_chunk_parser_cond_offsets[] = {
	0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 2, 3, 4, 5,
	5, 6, 7, 7, 7, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 9,
	9
};

static const char _http_chunk_parser_cond_lengths[] = {
	0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 0,
	1, 1, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0,
	1
};

static const short _http_chunk_parser_cond_keys[] = {
	0u, 255u, 0u, 255u, 0u, 255u, 0u, 255u,
	0u, 255u, 0u, 255u, 0u, 255u, 0u, 255u,
	0u, 255u, 0u, 255u, 0
};

static const char _http_chunk_parser_cond_spaces[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0
};

static const short _http_chunk_parser_key_offsets[] = {
	0, 0, 7, 15, 16, 54, 55, 70,
	72, 73, 74, 89, 93, 99, 139, 147,
	148, 154, 160, 168, 169, 173, 187, 204,
	219, 235, 256, 261, 263, 265, 272, 291,
	313, 331, 345, 362, 377, 393, 414, 419,
	421, 423, 430, 449, 471, 489, 489, 493,
	508
};

static const short _http_chunk_parser_trans_keys[] = {
	48u, 49u, 57u, 65u, 70u, 97u, 102u, 13u,
	59u, 48u, 57u, 65u, 70u, 97u, 102u, 10u,
	269u, 380u, 382u, 525u, 556u, 559u, 635u, 637u,
	289u, 294u, 298u, 299u, 301u, 302u, 304u, 313u,
	321u, 346u, 350u, 378u, 512u, 544u, 545u, 550u,
	551u, 553u, 554u, 569u, 570u, 576u, 577u, 602u,
	603u, 605u, 606u, 638u, 639u, 767u, 10u, 58u,
	124u, 126u, 33u, 38u, 42u, 43u, 45u, 46u,
	48u, 57u, 65u, 90u, 94u, 122u, 13u, 32u,
	13u, 10u, 13u, 124u, 126u, 33u, 38u, 42u,
	43u, 45u, 46u, 48u, 57u, 65u, 90u, 94u,
	122u, 269u, 525u, 512u, 767u, 266u, 269u, 522u,
	525u, 512u, 767u, 269u, 314u, 380u, 382u, 525u,
	556u, 559u, 570u, 635u, 637u, 289u, 294u, 298u,
	299u, 301u, 302u, 304u, 313u, 321u, 346u, 350u,
	378u, 512u, 544u, 545u, 550u, 551u, 553u, 554u,
	569u, 571u, 576u, 577u, 602u, 603u, 605u, 606u,
	638u, 639u, 767u, 269u, 288u, 525u, 544u, 256u,
	511u, 512u, 767u, 10u, 269u, 525u, 256u, 511u,
	512u, 767u, 266u, 269u, 522u, 525u, 512u, 767u,
	13u, 59u, 48u, 57u, 65u, 70u, 97u, 102u,
	10u, 269u, 525u, 512u, 767u, 124u, 126u, 33u,
	38u, 42u, 43u, 45u, 46u, 48u, 57u, 65u,
	90u, 94u, 122u, 13u, 59u, 61u, 124u, 126u,
	33u, 38u, 42u, 43u, 45u, 46u, 48u, 57u,
	65u, 90u, 94u, 122u, 34u, 124u, 126u, 33u,
	38u, 42u, 43u, 45u, 46u, 48u, 57u, 65u,
	90u, 94u, 122u, 13u, 59u, 124u, 126u, 33u,
	38u, 42u, 43u, 45u, 46u, 48u, 57u, 65u,
	90u, 94u, 122u, 13u, 34u, 59u, 92u, 124u,
	126u, 127u, 0u, 31u, 33u, 38u, 42u, 43u,
	45u, 46u, 48u, 57u, 65u, 90u, 94u, 122u,
	34u, 92u, 127u, 0u, 31u, 13u, 59u, 34u,
	92u, 13u, 34u, 59u, 92u, 127u, 0u, 31u,
	34u, 92u, 124u, 126u, 127u, 0u, 31u, 33u,
	38u, 42u, 43u, 45u, 46u, 48u, 57u, 65u,
	90u, 94u, 122u, 13u, 34u, 59u, 61u, 92u,
	124u, 126u, 127u, 0u, 31u, 33u, 38u, 42u,
	43u, 45u, 46u, 48u, 57u, 65u, 90u, 94u,
	122u, 92u, 124u, 126u, 127u, 0u, 31u, 33u,
	38u, 42u, 43u, 45u, 46u, 48u, 57u, 65u,
	90u, 94u, 122u, 124u, 126u, 33u, 38u, 42u,
	43u, 45u, 46u, 48u, 57u, 65u, 90u, 94u,
	122u, 13u, 59u, 61u, 124u, 126u, 33u, 38u,
	42u, 43u, 45u, 46u, 48u, 57u, 65u, 90u,
	94u, 122u, 34u, 124u, 126u, 33u, 38u, 42u,
	43u, 45u, 46u, 48u, 57u, 65u, 90u, 94u,
	122u, 13u, 59u, 124u, 126u, 33u, 38u, 42u,
	43u, 45u, 46u, 48u, 57u, 65u, 90u, 94u,
	122u, 13u, 34u, 59u, 92u, 124u, 126u, 127u,
	0u, 31u, 33u, 38u, 42u, 43u, 45u, 46u,
	48u, 57u, 65u, 90u, 94u, 122u, 34u, 92u,
	127u, 0u, 31u, 13u, 59u, 34u, 92u, 13u,
	34u, 59u, 92u, 127u, 0u, 31u, 34u, 92u,
	124u, 126u, 127u, 0u, 31u, 33u, 38u, 42u,
	43u, 45u, 46u, 48u, 57u, 65u, 90u, 94u,
	122u, 13u, 34u, 59u, 61u, 92u, 124u, 126u,
	127u, 0u, 31u, 33u, 38u, 42u, 43u, 45u,
	46u, 48u, 57u, 65u, 90u, 94u, 122u, 92u,
	124u, 126u, 127u, 0u, 31u, 33u, 38u, 42u,
	43u, 45u, 46u, 48u, 57u, 65u, 90u, 94u,
	122u, 269u, 525u, 512u, 767u, 13u, 124u, 126u,
	33u, 38u, 42u, 43u, 45u, 46u, 48u, 57u,
	65u, 90u, 94u, 122u, 269u, 380u, 382u, 525u,
	556u, 559u, 635u, 637u, 289u, 294u, 298u, 299u,
	301u, 302u, 304u, 313u, 321u, 346u, 350u, 378u,
	512u, 544u, 545u, 550u, 551u, 553u, 554u, 569u,
	570u, 576u, 577u, 602u, 603u, 605u, 606u, 638u,
	639u, 767u, 0
};

static const char _http_chunk_parser_single_lengths[] = {
	0, 1, 2, 1, 8, 1, 3, 2,
	1, 1, 3, 2, 4, 10, 4, 1,
	2, 4, 2, 1, 2, 2, 5, 3,
	4, 7, 3, 2, 2, 5, 5, 8,
	4, 2, 5, 3, 4, 7, 3, 2,
	2, 5, 5, 8, 4, 0, 2, 3,
	8
};

static const char _http_chunk_parser_range_lengths[] = {
	0, 3, 3, 0, 15, 0, 6, 0,
	0, 0, 6, 1, 1, 15, 2, 0,
	2, 1, 3, 0, 1, 6, 6, 6,
	6, 7, 1, 0, 0, 1, 7, 7,
	7, 6, 6, 6, 6, 7, 1, 0,
	0, 1, 7, 7, 7, 0, 1, 6,
	15
};

static const short _http_chunk_parser_index_offsets[] = {
	0, 0, 5, 11, 13, 37, 39, 49,
	52, 54, 56, 66, 70, 76, 102, 109,
	111, 116, 122, 128, 130, 134, 143, 155,
	165, 176, 191, 196, 199, 202, 209, 222,
	238, 250, 259, 271, 281, 292, 307, 312,
	315, 318, 325, 338, 354, 366, 367, 371,
	381
};

static const char _http_chunk_parser_indicies[] = {
	1, 2, 2, 2, 0, 3, 5, 4,
	4, 4, 0, 6, 0, 7, 8, 8,
	10, 9, 9, 9, 9, 8, 8, 8,
	8, 8, 8, 9, 11, 9, 11, 9,
	11, 9, 11, 9, 0, 12, 0, 14,
	13, 13, 13, 13, 13, 13, 13, 13,
	0, 16, 17, 15, 19, 18, 20, 0,
	21, 8, 8, 8, 8, 8, 8, 8,
	8, 0, 21, 23, 22, 0, 12, 21,
	24, 23, 22, 0, 21, 14, 13, 13,
	23, 22, 22, 26, 22, 22, 13, 13,
	13, 13, 13, 13, 22, 25, 22, 25,
	22, 25, 22, 25, 22, 0, 27, 17,
	29, 30, 15, 28, 0, 31, 0, 32,
	34, 18, 33, 0, 31, 21, 35, 23,
	22, 0, 36, 37, 4, 4, 4, 0,
	38, 0, 7, 10, 9, 0, 39, 39,
	39, 39, 39, 39, 39, 39, 0, 40,
	41, 42, 39, 39, 39, 39, 39, 39,
	39, 39, 0, 44, 43, 43, 43, 43,
	43, 43, 43, 43, 0, 40, 41, 43,
	43, 43, 43, 43, 43, 43, 43, 0,
	40, 43, 46, 47, 44, 44, 0, 0,
	44, 44, 44, 44, 44, 44, 45, 48,
	47, 0, 0, 45, 40, 41, 0, 49,
	47, 45, 40, 48, 46, 47, 0, 0,
	45, 39, 47, 50, 50, 0, 0, 50,
	50, 50, 50, 50, 50, 45, 40, 39,
	46, 51, 47, 50, 50, 0, 0, 50,
	50, 50, 50, 50, 50, 45, 47, 44,
	44, 0, 0, 44, 44, 44, 44, 44,
	44, 45, 52, 52, 52, 52, 52, 52,
	52, 52, 0, 53, 54, 55, 52, 52,
	52, 52, 52, 52, 52, 52, 0, 57,
	56, 56, 56, 56, 56, 56, 56, 56,
	0, 53, 54, 56, 56, 56, 56, 56,
	56, 56, 56, 0, 53, 56, 59, 60,
	57, 57, 0, 0, 57, 57, 57, 57,
	57, 57, 58, 61, 60, 0, 0, 58,
	53, 54, 0, 62, 60, 58, 53, 61,
	59, 60, 0, 0, 58, 52, 60, 63,
	63, 0, 0, 63, 63, 63, 63, 63,
	63, 58, 53, 52, 59, 64, 60, 63,
	63, 0, 0, 63, 63, 63, 63, 63,
	63, 58, 60, 57, 57, 0, 0, 57,
	57, 57, 57, 57, 57, 58, 0, 21,
	23, 22, 0, 21, 8, 8, 8, 8,
	8, 8, 8, 8, 0, 21, 8, 8,
	23, 22, 22, 22, 22, 8, 8, 8,
	8, 8, 8, 22, 65, 22, 65, 22,
	65, 22, 65, 22, 0, 0
};

static const char _http_chunk_parser_trans_targs[] = {
	0, 2, 18, 3, 18, 33, 4, 5,
	6, 11, 12, 13, 45, 6, 7, 8,
	9, 7, 8, 9, 10, 5, 11, 12,
	46, 13, 14, 15, 16, 17, 14, 47,
	15, 16, 17, 48, 19, 21, 20, 22,
	19, 21, 23, 24, 25, 26, 30, 28,
	27, 29, 31, 32, 34, 3, 33, 35,
	36, 37, 38, 42, 40, 39, 41, 43,
	44, 13
};

static const char _http_chunk_parser_trans_actions[] = {
	17, 9, 9, 25, 0, 25, 0, 13,
	1, 13, 13, 19, 15, 0, 3, 5,
	22, 5, 0, 7, 0, 0, 0, 0,
	15, 0, 3, 22, 5, 22, 5, 15,
	7, 0, 7, 15, 11, 11, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1
};

static const char _http_chunk_parser_eof_actions[] = {
	0, 17, 17, 17, 17, 17, 17, 17,
	17, 17, 17, 17, 17, 17, 17, 17,
	17, 17, 17, 17, 17, 17, 17, 17,
	17, 17, 17, 17, 17, 17, 17, 17,
	17, 17, 17, 17, 17, 17, 17, 17,
	17, 17, 17, 17, 17, 0, 0, 0,
	0
};

static const int http_chunk_parser_start = 1;
static const int http_chunk_parser_first_final = 45;
static const int http_chunk_parser_error = 0;

static const int http_chunk_parser_en_main = 1;


	{
	cs = http_chunk_parser_start;
	}

	{
	int _klen;
	unsigned int _trans;
	short _widec;
	const char *_acts;
	unsigned int _nacts;
	const short *_keys;

	if ( cs == 0 )
		goto _out;
_resume:
	_widec = (*p);
	_klen = _http_chunk_parser_cond_lengths[cs];
	_keys = _http_chunk_parser_cond_keys + (_http_chunk_parser_cond_offsets[cs]*2);
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( _widec < _mid[0] )
				_upper = _mid - 2;
			else if ( _widec > _mid[1] )
				_lower = _mid + 2;
			else {
				switch ( _http_chunk_parser_cond_spaces[_http_chunk_parser_cond_offsets[cs] + ((_mid - _keys)>>1)] ) {
	case 0: {
		_widec = (short)(256u + ((*p) - 0u));
		if (
 seen_chunk_size++ < chunk_size  ) _widec += 256;
		break;
	}
				}
				break;
			}
		}
	}

	_keys = _http_chunk_parser_trans_keys + _http_chunk_parser_key_offsets[cs];
	_trans = _http_chunk_parser_index_offsets[cs];

	_klen = _http_chunk_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( _widec < *_mid )
				_upper = _mid - 1;
			else if ( _widec > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _http_chunk_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( _widec < _mid[0] )
				_upper = _mid - 2;
			else if ( _widec > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _http_chunk_parser_indicies[_trans];
	cs = _http_chunk_parser_trans_targs[_trans];

	if ( _http_chunk_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _http_chunk_parser_actions + _http_chunk_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
	{ field_name_offset = static_cast<uint16_t>( p - ps ); }
	break;
	case 1:
	{ *p = 0; }
	break;
	case 2:
	{ field_value_offset = static_cast<uint16_t>( p - ps ); }
	break;
	case 3:
	{
                       *p = 0;
                       field_offsets.push_back
                       (
                         HTTPMessage::FieldOffset
                         (
                           field_name_offset,
                           field_value_offset
                         )
                       );
                     }
	break;
	case 4:
	{ chunk_size_p = p; }
	break;
	case 5:
	{
                          char* chunk_size_pe = p;
                          chunk_size
                            = static_cast<size_t>
                              (
                                strtol
                                (
                                  chunk_size_p,
                                  &chunk_size_pe,
                                  16
                                )
                              );
                        }
	break;
	case 6:
	{ chunk_data_p = p; }
	break;
	case 7:
	{ chunk_size = 0; }
	break;
	case 8:
	{ {p++; goto _out; } }
	break;
	case 9:
	{ return PARSE_ERROR; }
	break;
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	p += 1;
	goto _resume;
	if ( p == eof )
	{
	const char *__acts = _http_chunk_parser_actions + _http_chunk_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 9:
	{ return PARSE_ERROR; }
	break;
		}
	}
	}

	_out: {}
	}



      if ( cs != http_chunk_parser_error )
      {
        if ( chunk_size > 0 )
        {
          // Cut off the chunk size + extension  + CRLF before
          // the chunk data and the CRLF after
          out_http_messages.push_back
          (
            createHTTPMessage
            (
              new SubBuffer
                  (
                    *body,
                    chunk_data_p - ps,
                    p - chunk_data_p - 2,
                    p - chunk_data_p - 2
                  )
            )
          );

          body->position( body->position() + ( p - ps ) );

          if ( body->position() == body->size() )
          {
            Buffer::dec_ref( *body );
            body = new StringBuffer;
            return PARSE_WANT_READ;
          }
          else
            return parse( *body, out_http_messages );
        }
        else
        {
          out_http_messages.push_back( createHTTPMessage( new StringBuffer ) );
          Buffer::dec_ref( *body );
          reset();
          return PARSE_COMPLETE;
        }
      }
      else
        return PARSE_WANT_READ;
    }
    else
    {
      if ( body->size() == body->capacity() )
      {
        out_http_messages.push_back( createHTTPMessage( body ) );
        reset();

        if ( buffer.position() == buffer.size() )
          return PARSE_COMPLETE;
        else
          return parse( buffer, out_http_messages );
      }
      else
        return PARSE_WANT_READ;
    }
  }
}

template <class HTTPMessageParserType, class HTTPMessageType>
typename HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::ParseStatus
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::parse_header
(
  Buffer& buffer
)
{
  char *p = static_cast<char*>( buffer ) + buffer.position();
  const char *ps = p,
             *pe = p + ( buffer.size() - buffer.position() ),
             *eof = pe;

  ParseStatus parse_first_header_line_status
    = static_cast<HTTPMessageParserType*>( this )
        ->parse_first_header_line( &p, pe );

  if ( parse_first_header_line_status == PARSE_COMPLETE )
  {
    // Variables used by Ragel
    int cs;

    // Variables used by machine actions
    uint16_t field_name_offset = 0, field_value_offset = 0;


static const char _http_field_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1,
	3, 1, 4, 1, 5, 2, 2, 3

};

static const char _http_field_parser_key_offsets[] = {
	0, 0, 15, 16, 31, 33, 34, 35
};

static const unsigned char _http_field_parser_trans_keys[] = {
	13u, 124u, 126u, 33u, 38u, 42u, 43u, 45u,
	46u, 48u, 57u, 65u, 90u, 94u, 122u, 10u,
	58u, 124u, 126u, 33u, 38u, 42u, 43u, 45u,
	46u, 48u, 57u, 65u, 90u, 94u, 122u, 13u,
	32u, 13u, 10u, 0
};

static const char _http_field_parser_single_lengths[] = {
	0, 3, 1, 3, 2, 1, 1, 0
};

static const char _http_field_parser_range_lengths[] = {
	0, 6, 0, 6, 0, 0, 0, 0
};

static const char _http_field_parser_index_offsets[] = {
	0, 0, 10, 12, 22, 25, 27, 29
};

static const char _http_field_parser_indicies[] = {
	1, 2, 2, 2, 2, 2, 2, 2,
	2, 0, 3, 0, 5, 4, 4, 4,
	4, 4, 4, 4, 4, 0, 7, 8,
	6, 10, 9, 11, 0, 0, 0
};

static const char _http_field_parser_trans_targs[] = {
	0, 2, 3, 7, 3, 4, 5, 6,
	4, 5, 6, 1
};

static const char _http_field_parser_trans_actions[] = {
	11, 0, 1, 9, 0, 3, 5, 13,
	5, 0, 7, 0
};

static const char _http_field_parser_eof_actions[] = {
	0, 11, 11, 11, 11, 11, 11, 0
};

static const int http_field_parser_start = 1;
static const int http_field_parser_first_final = 7;
static const int http_field_parser_error = 0;

static const int http_field_parser_en_main = 1;


	{
	cs = http_field_parser_start;
	}

	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const unsigned char *_keys;

	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _http_field_parser_trans_keys + _http_field_parser_key_offsets[cs];
	_trans = _http_field_parser_index_offsets[cs];

	_klen = _http_field_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _http_field_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _http_field_parser_indicies[_trans];
	cs = _http_field_parser_trans_targs[_trans];

	if ( _http_field_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _http_field_parser_actions + _http_field_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
	{ field_name_offset = static_cast<uint16_t>( p - ps ); }
	break;
	case 1:
	{ *p = 0; }
	break;
	case 2:
	{ field_value_offset = static_cast<uint16_t>( p - ps ); }
	break;
	case 3:
	{
                       *p = 0;
                       field_offsets.push_back
                       (
                         HTTPMessage::FieldOffset
                         (
                           field_name_offset,
                           field_value_offset
                         )
                       );
                     }
	break;
	case 4:
	{ {p++; goto _out; } }
	break;
	case 5:
	{ return PARSE_ERROR; }
	break;
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	p += 1;
	goto _resume;
	if ( p == eof )
	{
	const char *__acts = _http_field_parser_actions + _http_field_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 5:
	{ return PARSE_ERROR; }
	break;
		}
	}
	}

	_out: {}
	}



    if ( cs != http_field_parser_error ) // Done reading the header
    {
      buffer.position( buffer.position() + ( p - ps ) );
      return PARSE_COMPLETE;
    }
    else
      return PARSE_WANT_READ;
  }
  else
    return parse_first_header_line_status;
}

template <class HTTPMessageParserType, class HTTPMessageType>
Time
HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::parse_http_date
(
  const char* http_date
)
{
  if ( http_date != NULL && http_date[0] != 0 )
  {
    // Variables used by Ragel
    int cs;
    const char *p = http_date,
               *ps = p,
               *pe = p + strnlen( http_date, UINT8_MAX ),
               *eof = pe;

    // Variables used by machine actions
    int hour = 0, minute = 0, second = 0;
    int day = 0, month = 0, year = 0;


static const char _http_date_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1,
	3, 1, 4, 1, 5, 1, 6, 1,
	7, 1, 8, 1, 9
};

static const unsigned char _http_date_parser_key_offsets[] = {
	0, 0, 5, 6, 7, 10, 18, 20,
	21, 22, 25, 27, 28, 30, 32, 33,
	35, 37, 38, 40, 42, 43, 45, 47,
	49, 51, 53, 54, 55, 56, 57, 58,
	59, 61, 62, 64, 65, 67, 68, 69,
	70, 71, 72, 73, 74, 76, 78, 79,
	87, 89, 90, 91, 93, 95, 97, 99,
	100, 102, 104, 105, 107, 109, 110, 112,
	114, 115, 116, 117, 118, 119, 120, 121,
	122, 123, 124, 126, 127, 129, 130, 132,
	133, 134, 135, 136, 137, 138, 139, 140,
	141, 142, 144, 146, 147, 155, 157, 158,
	159, 161, 162, 163, 164, 165, 166, 167,
	169, 170, 172, 173, 175, 176, 177, 178,
	179, 180, 181, 182, 183, 185, 186, 189,
	190, 191, 193, 194, 197, 198, 199, 202,
	203, 204, 207, 208
};

static const char _http_date_parser_trans_keys[] = {
	70, 77, 83, 84, 87, 114, 105, 32,
	44, 100, 65, 68, 70, 74, 77, 78,
	79, 83, 112, 117, 114, 32, 32, 48,
	57, 48, 57, 32, 48, 57, 48, 57,
	58, 48, 57, 48, 57, 58, 48, 57,
	48, 57, 32, 48, 57, 48, 57, 48,
	57, 48, 57, 48, 57, 103, 101, 99,
	32, 101, 98, 97, 117, 110, 108, 110,
	97, 114, 121, 111, 118, 99, 116, 101,
	112, 32, 48, 57, 48, 57, 32, 65,
	68, 70, 74, 77, 78, 79, 83, 112,
	117, 114, 32, 48, 57, 48, 57, 48,
	57, 48, 57, 32, 48, 57, 48, 57,
	58, 48, 57, 48, 57, 58, 48, 57,
	48, 57, 32, 71, 77, 84, 103, 101,
	99, 32, 101, 98, 97, 117, 110, 108,
	110, 97, 114, 121, 111, 118, 99, 116,
	101, 112, 97, 121, 44, 32, 48, 57,
	48, 57, 45, 65, 68, 70, 74, 77,
	78, 79, 83, 112, 117, 114, 45, 48,
	57, 103, 101, 99, 45, 101, 98, 97,
	117, 110, 108, 110, 97, 114, 121, 111,
	118, 99, 116, 101, 112, 111, 110, 97,
	117, 116, 32, 44, 117, 114, 100, 104,
	117, 117, 32, 44, 114, 115, 101, 32,
	44, 115, 101, 100, 32, 44, 110, 101,
	0
};

static const char _http_date_parser_single_lengths[] = {
	0, 5, 1, 1, 3, 8, 2, 1,
	1, 1, 0, 1, 0, 0, 1, 0,
	0, 1, 0, 0, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1,
	2, 1, 2, 1, 2, 1, 1, 1,
	1, 1, 1, 1, 0, 0, 1, 8,
	2, 1, 1, 0, 0, 0, 0, 1,
	0, 0, 1, 0, 0, 1, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 2, 1, 2, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 1, 8, 2, 1, 1,
	0, 1, 1, 1, 1, 1, 1, 2,
	1, 2, 1, 2, 1, 1, 1, 1,
	1, 1, 1, 1, 2, 1, 3, 1,
	1, 2, 1, 3, 1, 1, 3, 1,
	1, 3, 1, 0
};

static const char _http_date_parser_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 1,
	1, 0, 1, 1, 0, 1, 1, 1,
	1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 0,
	1, 1, 0, 1, 1, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

static const short _http_date_parser_index_offsets[] = {
	0, 0, 6, 8, 10, 14, 23, 26,
	28, 30, 33, 35, 37, 39, 41, 43,
	45, 47, 49, 51, 53, 55, 57, 59,
	61, 63, 65, 67, 69, 71, 73, 75,
	77, 80, 82, 85, 87, 90, 92, 94,
	96, 98, 100, 102, 104, 106, 108, 110,
	119, 122, 124, 126, 128, 130, 132, 134,
	136, 138, 140, 142, 144, 146, 148, 150,
	152, 154, 156, 158, 160, 162, 164, 166,
	168, 170, 172, 175, 177, 180, 182, 185,
	187, 189, 191, 193, 195, 197, 199, 201,
	203, 205, 207, 209, 211, 220, 223, 225,
	227, 229, 231, 233, 235, 237, 239, 241,
	244, 246, 249, 251, 254, 256, 258, 260,
	262, 264, 266, 268, 270, 273, 275, 279,
	281, 283, 286, 288, 292, 294, 296, 300,
	302, 304, 308, 310
};

static const unsigned char _http_date_parser_indicies[] = {
	1, 2, 3, 4, 5, 0, 6, 0,
	7, 0, 8, 9, 10, 0, 11, 12,
	13, 14, 15, 16, 17, 18, 0, 19,
	20, 0, 21, 0, 22, 0, 23, 24,
	0, 25, 0, 26, 0, 27, 0, 28,
	0, 29, 0, 30, 0, 31, 0, 32,
	0, 33, 0, 34, 0, 35, 0, 36,
	0, 37, 0, 38, 0, 39, 0, 40,
	0, 21, 0, 41, 0, 42, 0, 43,
	0, 44, 0, 21, 0, 45, 46, 0,
	21, 0, 21, 21, 0, 47, 0, 21,
	21, 0, 48, 0, 21, 0, 49, 0,
	21, 0, 50, 0, 21, 0, 51, 0,
	52, 0, 53, 0, 54, 0, 55, 56,
	57, 58, 59, 60, 61, 62, 0, 63,
	64, 0, 65, 0, 66, 0, 67, 0,
	68, 0, 69, 0, 70, 0, 71, 0,
	72, 0, 73, 0, 74, 0, 75, 0,
	76, 0, 77, 0, 78, 0, 79, 0,
	80, 0, 81, 0, 82, 0, 39, 0,
	65, 0, 83, 0, 84, 0, 85, 0,
	86, 0, 65, 0, 87, 88, 0, 65,
	0, 65, 65, 0, 89, 0, 65, 65,
	0, 90, 0, 65, 0, 91, 0, 65,
	0, 92, 0, 65, 0, 93, 0, 94,
	0, 95, 0, 96, 0, 97, 0, 98,
	0, 99, 0, 100, 101, 102, 103, 104,
	105, 106, 107, 0, 108, 109, 0, 110,
	0, 111, 0, 112, 0, 110, 0, 113,
	0, 114, 0, 115, 0, 116, 0, 110,
	0, 117, 118, 0, 110, 0, 110, 110,
	0, 119, 0, 110, 110, 0, 120, 0,
	110, 0, 121, 0, 110, 0, 122, 0,
	110, 0, 123, 0, 7, 0, 124, 123,
	0, 125, 0, 8, 9, 126, 0, 127,
	0, 10, 0, 128, 129, 0, 130, 0,
	8, 9, 131, 0, 127, 0, 132, 0,
	8, 9, 127, 0, 133, 0, 134, 0,
	8, 9, 135, 0, 131, 0, 0, 0
};

static const unsigned char _http_date_parser_trans_targs[] = {
	0, 2, 114, 116, 121, 127, 3, 4,
	5, 43, 85, 6, 27, 30, 32, 35,
	37, 39, 41, 7, 26, 8, 9, 10,
	25, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 131,
	11, 28, 29, 9, 31, 33, 34, 36,
	38, 40, 42, 44, 45, 46, 47, 48,
	69, 72, 74, 77, 79, 81, 83, 49,
	68, 50, 51, 52, 53, 54, 55, 56,
	57, 58, 59, 60, 61, 62, 63, 64,
	65, 66, 67, 70, 71, 51, 73, 75,
	76, 78, 80, 82, 84, 86, 87, 88,
	89, 90, 91, 92, 93, 98, 101, 103,
	106, 108, 110, 112, 94, 97, 95, 96,
	54, 99, 100, 96, 102, 104, 105, 107,
	109, 111, 113, 115, 117, 118, 119, 120,
	122, 125, 123, 124, 126, 128, 129, 130
};

static const char _http_date_parser_trans_actions[] = {
	19, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 11, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	9, 7, 0, 1, 0, 0, 3, 0,
	0, 5, 0, 0, 17, 0, 0, 0,
	0, 0, 0, 13, 0, 0, 0, 0,
	0, 0, 0, 0, 9, 0, 0, 0,
	11, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 17, 0, 0, 0, 0,
	1, 0, 0, 3, 0, 0, 5, 0,
	0, 0, 0, 0, 0, 13, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 9, 0, 0, 0, 11, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	15, 0, 0, 13, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static const char _http_date_parser_eof_actions[] = {
	0, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 0
};

static const int http_date_parser_start = 1;
static const int http_date_parser_first_final = 131;
static const int http_date_parser_error = 0;

static const int http_date_parser_en_main = 1;


	{
	cs = http_date_parser_start;
	}

	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _http_date_parser_trans_keys + _http_date_parser_key_offsets[cs];
	_trans = _http_date_parser_index_offsets[cs];

	_klen = _http_date_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _http_date_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _http_date_parser_indicies[_trans];
	cs = _http_date_parser_trans_targs[_trans];

	if ( _http_date_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _http_date_parser_actions + _http_date_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
	{ hour = atoi( p ); }
	break;
	case 1:
	{ minute = atoi( p ); }
	break;
	case 2:
	{ second = atoi( p ); }
	break;
	case 3:
	{ day = atoi( p ); }
	break;
	case 4:
	{ day = atoi( p ); }
	break;
	case 5:
	{ ps = p; }
	break;
	case 6:
	{
                   if ( *ps == 'J' ) month = 1;
                   else if ( *ps == 'F' ) month = 2;
                   else if ( *ps == 'M' )
                   {
                     if ( *(p+1) == 'a' ) month = 3;
                     else month = 5;
                   }
                   else if ( *ps == 'A' )
                   {
                     if ( *(ps+1) == 'p' ) month = 4;
                     else month = 8;
                   }
                   else if ( *ps == 'J' )
                   {
                     if ( *(ps+2) == 'n') month = 6;
                     else month = 7;
                   }
                   else if ( *ps == 'S' ) month = 9;
                   else if ( *ps == 'O' ) month = 10;
                   else if ( *ps == 'N' ) month = 11;
                   else if ( *ps == 'D' ) month = 12;
                   else DebugBreak();
                 }
	break;
	case 7:
	{ year = atoi( p ); }
	break;
	case 8:
	{ year = atoi( p ); }
	break;
	case 9:
	{ return Time::INVALID_TIME; }
	break;
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	p += 1;
	goto _resume;
	if ( p == eof )
	{
	const char *__acts = _http_date_parser_actions + _http_date_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 9:
	{ return Time::INVALID_TIME; }
	break;
		}
	}
	}

	_out: {}
	}



    if ( cs != http_date_parser_error )
    {
      if ( year < 100 ) year += 2000;
      return Time( second, minute, hour, day, month - 1, year - 1900, false );
    }
  }

  return Time::INVALID_TIME;
}

template <class HTTPMessageParserType, class HTTPMessageType>
void HTTPMessageParser<HTTPMessageParserType, HTTPMessageType>::reset()
{
  body = NULL;
  field_offsets.clear();
  Buffer::dec_ref( *header );
  header = NULL;
}

template class HTTPMessageParser<HTTPRequestParser, HTTPRequest>;
template class HTTPMessageParser<HTTPResponseParser, HTTPResponse>;

#ifdef _WIN32
#pragma warning( pop )
#endif


// http_request.cpp
HTTPRequest::HTTPRequest
(
  Buffer& header,
  uint16_t method_offset,
  uint16_t uri_offset,
  uint16_t http_version_offset,
  const FieldOffsets& field_offsets,
  Buffer* body
)
  : HTTPMessage( header, field_offsets, body ),
    method_offset( method_offset ),
    uri_offset( uri_offset ),
    http_version_offset( http_version_offset )
{
  parsed_uri = NULL;
}

HTTPRequest::HTTPRequest( const char* method, const char* uri )
{
  init( method, uri );
}

HTTPRequest::HTTPRequest( const char* method, const char* uri, Buffer& body )
{
  init( method, uri );
}

HTTPRequest::HTTPRequest( const char* method, const URI& uri )
{
  init( method, uri.get_resource().c_str() );
}

HTTPRequest::HTTPRequest( const char* method, const URI& uri, Buffer& body )
{
  init( method, uri.get_resource().c_str() );
}

HTTPRequest::~HTTPRequest()
{
  URI::dec_ref( parsed_uri );
}

double HTTPRequest::get_http_version() const
{
  const char* http_version_string
    = static_cast<const char*>( get_header() ) + http_version_offset;

  double http_version = atof( http_version_string );

  if ( http_version != 0 )
    return http_version;
  else
    return 1.1;
}

const char* HTTPRequest::get_method() const
{
  return static_cast<const char*>( get_header() ) + method_offset;
}

URI& HTTPRequest::get_parsed_uri()
{
  if ( parsed_uri != NULL )
    return *parsed_uri;
  else
  {
    parsed_uri = URI::parse( get_uri() );
#ifdef _DEBUG
    if ( parsed_uri == NULL ) DebugBreak();
#endif
    return *parsed_uri;
  }
}

const char* HTTPRequest::get_uri() const
{
  return static_cast<const char*>( get_header() ) + uri_offset;
}

void HTTPRequest::init( const char* method, const char* uri )
{
  get_header().push_back( const_cast<char*>( method ) );
  get_header().push_back( " ", 1 );
  get_header().push_back( const_cast<char*>( uri ) );
  get_header().push_back( " HTTP/1.1\r\n", 11 );
  parsed_uri = NULL;
}

void HTTPRequest::marshal( Marshaller& marshaller ) const
{
  HTTPMessage::marshal( marshaller );
}

void HTTPRequest::respond( HTTPResponse& http_response )
{
  Request::respond( http_response );
}

void HTTPRequest::respond( uint16_t status_code )
{
  Request::respond( *new HTTPResponse( status_code ) );
}

void HTTPRequest::respond( uint16_t status_code, Buffer& body )
{
  Request::respond( *new HTTPResponse( status_code, body ) );
}

void HTTPRequest::respond( ExceptionResponse& exception_response )
{
  Request::respond( exception_response );
}


// http_request_parser.cpp
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4702 )
#endif

HTTPRequestParser::HTTPRequestParser()
{
  method_offset = 0;
  uri_offset = 0;
  http_version_offset = 0;
}

HTTPRequest*
HTTPRequestParser::createHTTPMessage
(
  Buffer& header,
  const HTTPMessage::FieldOffsets& field_offsets,
  Buffer* body
)
{
  return new HTTPRequest
             (
               header,
               method_offset,
               uri_offset,
               http_version_offset,
               field_offsets,
               body
             );
}

HTTPRequestParser::ParseStatus
HTTPRequestParser::parse_first_header_line
(
  char** inout_p,
  const char* pe
)
{
  // Variables used by the Ragel parser
  int cs;
  char* p = *inout_p;
  const char *ps = p, *eof = pe;

  // Variables used by machine actions
  method_offset = 0; uri_offset = 0; http_version_offset = 0;


static const char _http_request_line_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1,
	3, 1, 4, 1, 5, 1, 6, 1,
	7
};

static const unsigned char _http_request_line_parser_key_offsets[] = {
	0, 0, 2, 6, 9, 11, 12, 13,
	14, 15, 16, 18, 21, 23, 27, 28,
	41, 47, 53, 67, 73, 79, 80, 81,
	82, 84, 85, 86, 92, 100, 107, 115,
	121, 133, 140, 142, 147, 153, 161, 167,
	175, 181, 193, 194
};

static const unsigned char _http_request_line_parser_trans_keys[] = {
	65u, 90u, 0u, 32u, 65u, 90u, 42u, 47u,
	104u, 0u, 32u, 72u, 84u, 84u, 80u, 47u,
	48u, 57u, 46u, 48u, 57u, 48u, 57u, 0u,
	13u, 48u, 57u, 10u, 0u, 32u, 33u, 37u,
	61u, 63u, 95u, 36u, 59u, 64u, 90u, 97u,
	122u, 48u, 57u, 65u, 70u, 97u, 102u, 48u,
	57u, 65u, 70u, 97u, 102u, 0u, 32u, 33u,
	37u, 61u, 95u, 36u, 46u, 48u, 59u, 64u,
	90u, 97u, 122u, 48u, 57u, 65u, 70u, 97u,
	102u, 48u, 57u, 65u, 70u, 97u, 102u, 116u,
	116u, 112u, 58u, 115u, 47u, 47u, 48u, 57u,
	65u, 90u, 97u, 122u, 45u, 46u, 48u, 57u,
	65u, 90u, 97u, 122u, 45u, 48u, 57u, 65u,
	90u, 97u, 122u, 45u, 46u, 48u, 57u, 65u,
	90u, 97u, 122u, 48u, 57u, 65u, 90u, 97u,
	122u, 0u, 32u, 45u, 46u, 47u, 58u, 48u,
	57u, 65u, 90u, 97u, 122u, 45u, 48u, 57u,
	65u, 90u, 97u, 122u, 48u, 57u, 0u, 32u,
	47u, 48u, 57u, 48u, 57u, 65u, 90u, 97u,
	122u, 45u, 46u, 48u, 57u, 65u, 90u, 97u,
	122u, 48u, 57u, 65u, 90u, 97u, 122u, 45u,
	46u, 48u, 57u, 65u, 90u, 97u, 122u, 48u,
	57u, 65u, 90u, 97u, 122u, 0u, 32u, 45u,
	46u, 47u, 58u, 48u, 57u, 65u, 90u, 97u,
	122u, 58u, 0
};

static const char _http_request_line_parser_single_lengths[] = {
	0, 0, 2, 3, 2, 1, 1, 1,
	1, 1, 0, 1, 0, 2, 1, 7,
	0, 0, 6, 0, 0, 1, 1, 1,
	2, 1, 1, 0, 2, 1, 2, 0,
	6, 1, 0, 3, 0, 2, 0, 2,
	0, 6, 1, 0
};

static const char _http_request_line_parser_range_lengths[] = {
	0, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 3,
	3, 3, 4, 3, 3, 0, 0, 0,
	0, 0, 0, 3, 3, 3, 3, 3,
	3, 3, 1, 1, 3, 3, 3, 3,
	3, 3, 0, 0
};

static const unsigned char _http_request_line_parser_index_offsets[] = {
	0, 0, 2, 6, 10, 13, 15, 17,
	19, 21, 23, 25, 28, 30, 34, 36,
	47, 51, 55, 66, 70, 74, 76, 78,
	80, 83, 85, 87, 91, 97, 102, 108,
	112, 122, 127, 129, 134, 138, 144, 148,
	154, 158, 168, 170
};

static const char _http_request_line_parser_indicies[] = {
	1, 0, 2, 2, 3, 0, 4, 5,
	6, 0, 7, 7, 0, 8, 0, 9,
	0, 10, 0, 11, 0, 12, 0, 13,
	0, 14, 15, 0, 16, 0, 17, 17,
	16, 0, 18, 0, 7, 7, 19, 20,
	19, 21, 19, 19, 19, 19, 0, 22,
	22, 22, 0, 19, 19, 19, 0, 7,
	7, 21, 23, 21, 21, 21, 21, 21,
	21, 0, 24, 24, 24, 0, 21, 21,
	21, 0, 25, 0, 26, 0, 27, 0,
	28, 29, 0, 30, 0, 31, 0, 32,
	33, 33, 0, 34, 35, 32, 36, 36,
	0, 34, 36, 36, 36, 0, 34, 37,
	36, 36, 36, 0, 36, 33, 33, 0,
	7, 7, 38, 37, 19, 39, 33, 33,
	33, 0, 38, 33, 33, 33, 0, 40,
	0, 7, 7, 19, 40, 0, 41, 33,
	33, 0, 34, 42, 41, 36, 36, 0,
	43, 33, 33, 0, 34, 44, 43, 36,
	36, 0, 45, 33, 33, 0, 7, 7,
	34, 37, 19, 39, 45, 36, 36, 0,
	28, 0, 0, 0
};

static const char _http_request_line_parser_trans_targs[] = {
	0, 2, 3, 2, 4, 15, 21, 5,
	6, 7, 8, 9, 10, 11, 12, 11,
	13, 14, 43, 15, 16, 18, 17, 19,
	20, 22, 23, 24, 25, 42, 26, 27,
	28, 32, 29, 36, 30, 31, 33, 34,
	35, 37, 38, 39, 40, 41
};

static const char _http_request_line_parser_trans_actions[] = {
	15, 1, 3, 0, 5, 5, 5, 7,
	0, 0, 0, 0, 0, 9, 0, 0,
	0, 11, 13, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0
};

static const char _http_request_line_parser_eof_actions[] = {
	0, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 0
};

static const int http_request_line_parser_start = 1;
static const int http_request_line_parser_first_final = 43;
static const int http_request_line_parser_error = 0;

static const int http_request_line_parser_en_main = 1;


	{
	cs = http_request_line_parser_start;
	}

	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const unsigned char *_keys;

	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _http_request_line_parser_trans_keys + _http_request_line_parser_key_offsets[cs];
	_trans = _http_request_line_parser_index_offsets[cs];

	_klen = _http_request_line_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _http_request_line_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _http_request_line_parser_indicies[_trans];
	cs = _http_request_line_parser_trans_targs[_trans];

	if ( _http_request_line_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _http_request_line_parser_actions + _http_request_line_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
	{ method_offset = static_cast<uint16_t>( p - ps ); }
	break;
	case 1:
	{ *p = 0; }
	break;
	case 2:
	{ uri_offset = static_cast<uint16_t>( p - ps ); }
	break;
	case 3:
	{ *p = 0; }
	break;
	case 4:
	{ http_version_offset = static_cast<uint16_t>( p - ps ); }
	break;
	case 5:
	{ *p = 0; }
	break;
	case 6:
	{ {p++; goto _out; } }
	break;
	case 7:
	{ return PARSE_ERROR; }
	break;
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	p += 1;
	goto _resume;
	if ( p == eof )
	{
	const char *__acts = _http_request_line_parser_actions + _http_request_line_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 7:
	{ return PARSE_ERROR; }
	break;
		}
	}
	}

	_out: {}
	}



  if ( cs != http_request_line_parser_error )
  {
    *inout_p = p;
    return PARSE_COMPLETE;
  }
  else
    return PARSE_WANT_READ;
}

#ifdef _WIN32
#pragma warning( pop )
#endif


// http_response.cpp
HTTPResponse::HTTPResponse
(
  Buffer& header,
  const FieldOffsets& field_offsets,
  uint16_t status_code,
  Buffer* body
)
  : HTTPMessage( header, field_offsets, body )
{
  init( status_code );
}

HTTPResponse::HTTPResponse( uint16_t status_code )
{
  init( status_code );
}

HTTPResponse::HTTPResponse( uint16_t status_code, Buffer& body )
 : HTTPMessage( body )
{
  init( status_code );
}

void HTTPResponse::init( uint16_t status_code )
{
  this->status_code = status_code;

  const char* status_line;
  size_t status_line_len;

  switch ( status_code )
  {
    case 100: status_line = "HTTP/1.1 100 Continue\r\n"; status_line_len = 23; break;
    case 200: status_line = "HTTP/1.1 200 OK\r\n"; status_line_len = 17; break;
    case 201: status_line = "HTTP/1.1 201 Created\r\n"; status_line_len = 22; break;
    case 202: status_line = "HTTP/1.1 202 Accepted\r\n"; status_line_len = 23; break;
    case 203: status_line = "HTTP/1.1 203 Non-Authoritative Information\r\n"; status_line_len = 44; break;
    case 204: status_line = "HTTP/1.1 204 No Content\r\n"; status_line_len = 25; break;
    case 205: status_line = "HTTP/1.1 205 Reset Content\r\n"; status_line_len = 28; break;
    case 206: status_line = "HTTP/1.1 206 Partial Content\r\n"; status_line_len = 30; break;
    case 207: status_line = "HTTP/1.1 207 Multi-Status\r\n"; status_line_len = 27; break;
    case 300: status_line = "HTTP/1.1 300 Multiple Choices\r\n"; status_line_len = 31; break;
    case 301: status_line = "HTTP/1.1 301 Moved Permanently\r\n"; status_line_len = 32; break;
    case 302: status_line = "HTTP/1.1 302 Found\r\n"; status_line_len = 20; break;
    case 303: status_line = "HTTP/1.1 303 See Other\r\n"; status_line_len = 24; break;
    case 304: status_line = "HTTP/1.1 304 Not Modified\r\n"; status_line_len = 27; break;
    case 305: status_line = "HTTP/1.1 305 Use Proxy\r\n"; status_line_len = 24; break;
    case 307: status_line = "HTTP/1.1 307 Temporary Redirect\r\n"; status_line_len = 33; break;
    case 400: status_line = "HTTP/1.1 400 Bad Request\r\n"; status_line_len = 26; break;
    case 401: status_line = "HTTP/1.1 401 Unauthorized\r\n"; status_line_len = 27; break;
    case 403: status_line = "HTTP/1.1 403 Forbidden\r\n"; status_line_len = 24; break;
    case 404: status_line = "HTTP/1.1 404 Not Found\r\n"; status_line_len = 24; break;
    case 405: status_line = "HTTP/1.1 405 Method Not Allowed\r\n"; status_line_len = 33; break;
    case 406: status_line = "HTTP/1.1 406 Not Acceptable\r\n"; status_line_len = 29; break;
    case 407: status_line = "HTTP/1.1 407 Proxy Authentication Required\r\n"; status_line_len = 44; break;
    case 408: status_line = "HTTP/1.1 408 Request Timeout\r\n"; status_line_len = 30; break;
    case 409: status_line = "HTTP/1.1 409 Conflict\r\n"; status_line_len = 23; break;
    case 410: status_line = "HTTP/1.1 410 Gone\r\n"; status_line_len = 19; break;
    case 411: status_line = "HTTP/1.1 411 Length Required\r\n"; status_line_len = 30; break;
    case 412: status_line = "HTTP/1.1 412 Precondition Failed\r\n"; status_line_len = 34; break;
    case 413: status_line = "HTTP/1.1 413 Request Entity Too Large\r\n"; status_line_len = 39; break;
    case 414: status_line = "HTTP/1.1 414 Request-URI Too Long\r\n"; status_line_len = 35; break;
    case 415: status_line = "HTTP/1.1 415 Unsupported Media Type\r\n"; status_line_len = 37; break;
    case 416: status_line = "HTTP/1.1 416 Request Range Not Satisfiable\r\n"; status_line_len = 44; break;
    case 417: status_line = "HTTP/1.1 417 Expectation Failed\r\n"; status_line_len = 33; break;
    case 422: status_line = "HTTP/1.1 422 Unprocessable Entitiy\r\n"; status_line_len = 36; break;
    case 423: status_line = "HTTP/1.1 423 Locked\r\n"; status_line_len = 21; break;
    case 424: status_line = "HTTP/1.1 424 Failed Dependency\r\n"; status_line_len = 32; break;
    case 500: status_line = "HTTP/1.1 500 Internal Server Error\r\n"; status_line_len = 36; break;
    case 501: status_line = "HTTP/1.1 501 Not Implemented\r\n"; status_line_len = 30; break;
    case 502: status_line = "HTTP/1.1 502 Bad Gateway\r\n"; status_line_len = 26; break;
    case 503: status_line = "HTTP/1.1 503 Service Unavailable\r\n"; status_line_len = 34; break;
    case 504: status_line = "HTTP/1.1 504 Gateway Timeout\r\n"; status_line_len = 30; break;
    case 505: status_line = "HTTP/1.1 505 HTTP Version Not Supported\r\n"; status_line_len = 41; break;
    case 507: status_line = "HTTP/1.1 507 Insufficient Storage\r\n"; status_line_len = 35; break;
    default: status_line = "HTTP/1.1 500 Internal Server Error\r\n"; status_line_len = 36; break;
  }

  get_header().push_back( status_line, status_line_len );

  set_field( "Date", Time() );
}

void HTTPResponse::marshal( Marshaller& marshaller ) const
{
  HTTPMessage::marshal( marshaller );
}


// http_response_parser.cpp
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4702 )
#endif

HTTPResponseParser::HTTPResponseParser()
{
  status_code = 0;
}

HTTPResponse*
HTTPResponseParser::createHTTPMessage
(
  Buffer& header,
  const HTTPMessage::FieldOffsets& field_offsets,
  Buffer* body
)
{
  return new HTTPResponse( header, field_offsets, status_code, body );
}

HTTPResponseParser::ParseStatus
HTTPResponseParser::parse_first_header_line
(
  char** inout_p,
  const char* pe
)
{
  // Variables used by the Ragel parser
  int cs;
  char* p = *inout_p;
  const char *ps = p, *eof = pe;

  // Variables used by machine actions
  uint16_t status_code_offset = 0;


static const char _http_status_line_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1,
	3
};

static const char _http_status_line_parser_key_offsets[] = {
	0, 0, 1, 2, 3, 4, 5, 7,
	10, 12, 15, 17, 21, 26, 32, 33
};

static const unsigned char _http_status_line_parser_trans_keys[] = {
	72u, 84u, 84u, 80u, 47u, 48u, 57u, 46u,
	48u, 57u, 48u, 57u, 32u, 48u, 57u, 48u,
	57u, 0u, 32u, 48u, 57u, 32u, 65u, 90u,
	97u, 122u, 13u, 32u, 65u, 90u, 97u, 122u,
	10u, 0
};

static const char _http_status_line_parser_single_lengths[] = {
	0, 1, 1, 1, 1, 1, 0, 1,
	0, 1, 0, 2, 1, 2, 1, 0
};

static const char _http_status_line_parser_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 1, 1,
	1, 1, 1, 1, 2, 2, 0, 0
};

static const char _http_status_line_parser_index_offsets[] = {
	0, 0, 2, 4, 6, 8, 10, 12,
	15, 17, 20, 22, 26, 30, 35, 37
};

static const char _http_status_line_parser_indicies[] = {
	1, 0, 2, 0, 3, 0, 4, 0,
	5, 0, 6, 0, 7, 6, 0, 8,
	0, 9, 8, 0, 10, 0, 11, 11,
	12, 0, 13, 13, 13, 0, 14, 13,
	13, 13, 0, 15, 0, 0, 0
};

static const char _http_status_line_parser_trans_targs[] = {
	0, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 11, 13, 14, 15
};

static const char _http_status_line_parser_trans_actions[] = {
	7, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 3, 0, 0, 0, 5
};

static const char _http_status_line_parser_eof_actions[] = {
	0, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 0
};

static const int http_status_line_parser_start = 1;
static const int http_status_line_parser_first_final = 15;
static const int http_status_line_parser_error = 0;

static const int http_status_line_parser_en_main = 1;


	{
	cs = http_status_line_parser_start;
	}

	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const unsigned char *_keys;

	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _http_status_line_parser_trans_keys + _http_status_line_parser_key_offsets[cs];
	_trans = _http_status_line_parser_index_offsets[cs];

	_klen = _http_status_line_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _http_status_line_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _http_status_line_parser_indicies[_trans];
	cs = _http_status_line_parser_trans_targs[_trans];

	if ( _http_status_line_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _http_status_line_parser_actions + _http_status_line_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
	{ status_code_offset = static_cast<uint16_t>( p - ps ); }
	break;
	case 1:
	{
                       *p = 0;

                       status_code
                         = static_cast<uint16_t>
                           (
                             atoi( ps + status_code_offset )
                           );

                       if ( status_code == 0 )
                        return PARSE_ERROR;
                     }
	break;
	case 2:
	{ {p++; goto _out; } }
	break;
	case 3:
	{ return PARSE_ERROR; }
	break;
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	p += 1;
	goto _resume;
	if ( p == eof )
	{
	const char *__acts = _http_status_line_parser_actions + _http_status_line_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 3:
	{ return PARSE_ERROR; }
	break;
		}
	}
	}

	_out: {}
	}



  if ( cs != http_status_line_parser_error )
  {
    *inout_p = p;
    return PARSE_COMPLETE;
  }
  else
    return PARSE_WANT_READ;
}

#ifdef _WIN32
#pragma warning( pop )
#endif


// http_server.cpp
using yidl::runtime::BufferedMarshaller;
using yidl::runtime::StackBuffer;
using yield::platform::File;
using yield::platform::Volume;

#ifdef _WIN32
#include <windows.h>
#endif


HTTPServer::HTTPServer
(
  AccessLog* access_log,
  Log* error_log,
  EventTarget& http_request_target,
  IOQueue& io_queue,
  TCPSocket& listen_tcp_socket,
  Log* trace_log
)
  : TCPSocketServer
    (
      error_log,
      io_queue,
      listen_tcp_socket,
      http_request_target,
      trace_log
    ),
    access_log( Object::inc_ref( access_log ) )
{
  get_listen_tcp_socket().aio_accept( *this, NULL, new StackBuffer<1024> );
}

HTTPServer&
HTTPServer::create
(
  const URI& absolute_uri,
  EventTarget& http_request_target,
  AccessLog* access_log,
  Log* error_log,
  SSLContext* ssl_context,
  Log* trace_log
)
{
  URI checked_absolute_uri( absolute_uri );
  if ( checked_absolute_uri.get_port() == 0 )
  {
    if ( absolute_uri.get_scheme() == "https" )
      checked_absolute_uri.set_port( 443 );
    else
      checked_absolute_uri.set_port( 80 );
  }

  IOQueue& io_queue = createIOQueue( ssl_context != NULL );

  return *new HTTPServer
             (
               access_log,
               error_log,
               http_request_target,
               io_queue,
               createListenTCPSocket
               (
                 absolute_uri,
                 io_queue,
                 ssl_context,
                 trace_log
               ),
               trace_log
             );
}

void
HTTPServer::onAcceptCompletion
(
  TCPSocket& accepted_tcp_socket,
  void* context,
  Buffer* recv_buffer
)
{
  TCPSocketServer::onAcceptCompletion
  (
    accepted_tcp_socket,
    context,
    recv_buffer
  );

  Connection* connection = new Connection( accepted_tcp_socket, *this );
  connection->onReadCompletion( *recv_buffer, NULL );

  get_listen_tcp_socket().aio_accept( *this, NULL, new StackBuffer<1024> );
}


HTTPServer::AccessLog::AccessLog( Format& format )
  : format( format )
{ }

HTTPServer::AccessLog::~AccessLog()
{
  delete &format;
}


string
HTTPServer::AccessLog::CombinedFormat::operator()
(
  const HTTPRequest& http_request,
  const HTTPResponse& http_response
) const
{
  string common_entry
    = CommonFormat::operator()( http_request, http_response );

  return common_entry;
}


string
HTTPServer::AccessLog::CommonFormat::operator()
(
  const HTTPRequest& http_request,
  const HTTPResponse& http_response
) const
{
  string client_ip;
  {
    SocketAddress* peername = SocketAddress::create( "127.0.0.1" );
    if ( peername != NULL )
    {
      if ( !peername->getnameinfo( client_ip ) )
        client_ip = "-";
      SocketAddress::dec_ref( *peername );
    }
    else
      client_ip = "-";
  }

  char request_creation_time[32];
  {
    static const char* Months[]
      = {
          "Jan", "Feb", "Mar", "Apr", "May", "Jun",
          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

#ifdef _WIN32
    SYSTEMTIME local_system_time = http_request.get_creation_time();
    TIME_ZONE_INFORMATION time_zone_information;
    GetTimeZoneInformation( &time_zone_information );

    // 10/Oct/2000:13:55:36 -0700
    _snprintf_s
    (
      request_creation_time,
      32,
      _TRUNCATE,
      "[%02d/%s/%04d:%02d:%02d:%02d %+0.4d]",
      local_system_time.wDay,
      Months[local_system_time.wMonth-1],
      local_system_time.wYear,
      local_system_time.wHour,
      local_system_time.wMinute,
      local_system_time.wSecond,
      ( time_zone_information.Bias / 60 ) * -100
    );
#else
    struct tm local_tm = Time();

    snprintf
    (
      request_creation_time,
      32,
      "[%02d/%s/%04d:%02d:%02d:%02d %d]",
      local_tm.tm_mday,
      Months[local_tm.tm_mon],
      local_tm.tm_year + 1900,
      local_tm.tm_hour,
      local_tm.tm_min,
      local_tm.tm_sec,
      0
    );
#endif
  }

  const char* request_http_version;
  if ( http_request.get_http_version() == 1.0 )
    request_http_version = "1.0";
  else if ( http_request.get_http_version() == 1.1 )
    request_http_version = "1.1";
  else
    DebugBreak();

  ostringstream entry;
  entry << client_ip << " ";
  entry << "- "; // RFC 1413 identity
  entry << "- "; // HTTP auth userid
  entry << request_creation_time << " ";
  entry << "\"";
  entry << http_request.get_method() << " ";
  entry << http_request.get_uri() << " ";
  entry << "HTTP/" << request_http_version;
  entry << "\" ";
  entry << http_response.get_status_code() << " ";
  if ( http_response.get_body() != NULL )
    entry << http_response.get_body()->size();
  else
    entry << "-";
  entry << "\r\n";

  return entry.str();
}


class HTTPServer::ostreamAccessLog : public AccessLog
{
public:
  ostreamAccessLog( ostream& os, Format& format )
    : AccessLog( format ), os( os )
  { }

  // AccessLog
  void
  write
  (
    const HTTPRequest& http_request,
    const HTTPResponse& http_response
  )
  {
    string entry = get_format()( http_request, http_response );
    os << entry;
  }

private:
  ostream& os;
};

HTTPServer::AccessLog&
HTTPServer::AccessLog::open
(
  ostream& os,
  Format* format
)
{
  if ( format == NULL )
    format = new CombinedFormat;

  return *new ostreamAccessLog( os, *format );
}


class HTTPServer::FileAccessLog : public AccessLog
{
public:
  FileAccessLog( File& file, Format& format )
    : AccessLog( format ), file( &file )
  { }

  FileAccessLog( const Path& file_path, Format& format ) // Lazy open
    : AccessLog( format ), file_path( file_path )
  { }

  ~FileAccessLog()
  {
    File::dec_ref( file );
  }

  // AccessLog
  void
  write
  (
    const HTTPRequest& http_request,
    const HTTPResponse& http_response
  )
  {
    if ( file == NULL ) // Lazy open
    {
      file = Volume().open( file_path, O_CREAT|O_WRONLY|O_APPEND );
      if ( file == NULL )
        return;
    }

    string entry = get_format()( http_request, http_response );

    file->write( entry.c_str(), entry.size() );
  }

private:
  File* file;
  Path file_path;
};

HTTPServer::AccessLog&
HTTPServer::AccessLog::open
(
  const Path& file_path,
  Format* format,
  bool lazy_open
)
{
  if ( format == NULL )
    format = new CombinedFormat;

  if ( file_path.empty() || file_path == "-" )
    return *new ostreamAccessLog( cout, *format );
  else if ( lazy_open )
    return *new FileAccessLog( file_path, *format );
  else
  {
    File* file = Volume().open( file_path, O_CREAT|O_WRONLY|O_APPEND );
    if ( file != NULL )
      return *new FileAccessLog( *file, *format );
    else
      throw Exception();
  }
}


HTTPServer::Connection::Connection
(
  TCPSocket& accepted_tcp_socket,
  HTTPServer& http_server
)
  : TCPSocketServer::Connection( accepted_tcp_socket, http_server )
{ }

void HTTPServer::Connection::onReadCompletion( Buffer& buffer, void* )
{
  vector<HTTPRequest*> http_requests;
  HTTPRequestParser::ParseStatus parse_status
    = HTTPRequestParser::parse( buffer, http_requests );

  if ( !http_requests.empty() )
  {
    for
    (
      vector<HTTPRequest*>::iterator http_request_i = http_requests.begin();
      http_request_i != http_requests.end();
      ++http_request_i
    )
    {
      HTTPRequest* http_request = *http_request_i;
      http_request->set_response_target( this );
      get_socket_server().get_request_target().send( *http_request );
    }
  }

  if ( parse_status != HTTPRequestParser::PARSE_ERROR )
    get_socket().aio_recv( *new StackBuffer<1024>, 0, *this, NULL );
  else
    onReadError( 0, NULL );
}

void HTTPServer::Connection::onWriteCompletion
(
  size_t bytes_written,
  void* context
)
{
  TCPSocketServer::Connection::onWriteCompletion( bytes_written, context );
  HTTPResponse* http_response = static_cast<HTTPResponse*>( context );
  HTTPResponse::dec_ref( *http_response );
}

void HTTPServer::Connection::onWriteError( uint32_t error_code, void* context )
{
  HTTPResponse* http_response = static_cast<HTTPResponse*>( context );
  HTTPResponse::dec_ref( *http_response );
  TCPSocketServer::Connection::onWriteError( error_code, context );
}

void HTTPServer::Connection::send( Response& response )
{
  if ( response.get_type_id() == HTTPResponse::TYPE_ID )
  {
    HTTPResponse& http_response = static_cast<HTTPResponse&>( response );

    BufferedMarshaller buffered_marshaller;
    http_response.marshal( buffered_marshaller );
    Buffers& buffers = buffered_marshaller.get_buffers().inc_ref();

    get_socket().aio_sendmsg( buffers, 0, *this, &http_response );
  }
  else
    DebugBreak();
}


// json_marshaller.cpp
using yidl::runtime::StringBuffer;

extern "C"
{
};


JSONMarshaller::JSONMarshaller( bool write_empty_strings )
: write_empty_strings( write_empty_strings )
{
  buffer = new StringBuffer;
  root_key = NULL;
  writer = yajl_gen_alloc( NULL );
}

JSONMarshaller::JSONMarshaller
(
  JSONMarshaller& parent_json_marshaller,
  const Key& root_key
)
  : buffer( parent_json_marshaller.buffer ),
    root_key( &root_key ),
    write_empty_strings( parent_json_marshaller.write_empty_strings ),
    writer( parent_json_marshaller.writer )
{ }

JSONMarshaller::~JSONMarshaller()
{
  Buffer::dec_ref( *buffer );
//  if ( root_key == NULL ) // This is the root JSONMarshaller
//    yajl_gen_free( writer );
}

void JSONMarshaller::flushYAJLBuffer()
{
  const unsigned char* buffer;
  unsigned int len;
  yajl_gen_get_buf( writer, &buffer, &len );
  this->buffer->put( buffer, len );
  yajl_gen_clear( writer );
}

void JSONMarshaller::write( const Key& key, bool value )
{
  write( key );
  yajl_gen_bool( writer, static_cast<int>( value ) );
  flushYAJLBuffer();
}

void JSONMarshaller::write( const Key& key, double value )
{
  write( key );
  yajl_gen_double( writer, value );
  flushYAJLBuffer();
}

void JSONMarshaller::write( const Key& key, int64_t value )
{
  write( key );
  yajl_gen_integer( writer, static_cast<long>( value ) );
  flushYAJLBuffer();
}

void JSONMarshaller::write( const Key& key )
{
  if ( in_map )
  {
    Key::Type key_type = key.get_type();
    if ( key_type == Key::TYPE_DOUBLE )
      yajl_gen_double( writer, static_cast<const DoubleKey&>( key ) );
    else if ( key_type == Key::TYPE_INT32 )
      yajl_gen_integer( writer, static_cast<const Int32Key&>( key ) );
    else if ( key_type == Key::TYPE_INT64 )
    {
      yajl_gen_integer
      (
        writer,
        static_cast<long>( static_cast<const Int64Key&>( key ) )
      );
    }
    else if ( key_type == Key::TYPE_STRING )
    {
      yajl_gen_string
      (
        writer,
        reinterpret_cast<const unsigned char*>
        (
          static_cast<const StringKey&>( key ).c_str()
        ),
        static_cast<const StringKey&>( key ).size()
      );
    }
    else if ( key_type == Key::TYPE_STRING_LITERAL )
    {
      yajl_gen_string
      (
        writer,
        reinterpret_cast<const unsigned char*>
        (
          static_cast<const char*>
          (
            static_cast<const StringLiteralKey&>( key )
          )
        ),
        strnlen( static_cast<const StringLiteralKey&>( key ), UINT16_MAX )
      );
    }
  }
}

void JSONMarshaller::write( const Key& key, const MarshallableObject& value )
{
  write( key );
  JSONMarshaller( *this, key ).write( &value );
}

void JSONMarshaller::write( const MarshallableObject* value )
{
  yajl_gen_map_open( writer );
  in_map = true;
  if ( value != NULL )
    value->marshal( *this );
  yajl_gen_map_close( writer );
  flushYAJLBuffer();
}

void JSONMarshaller::write( const Key& key, const Sequence& value )
{
  write( key );
  JSONMarshaller( *this, key ).write( &value );
}

void JSONMarshaller::write( const Sequence* value )
{
  yajl_gen_array_open( writer );
  in_map = false;
  if ( value != NULL )
    value->marshal( *this );
  yajl_gen_array_close( writer );
  flushYAJLBuffer();
}

void
JSONMarshaller::write
(
  const Key& key,
  const char* value,
  size_t value_len
)
{
  if ( value_len > 0 || write_empty_strings )
  {
    write( key );
    yajl_gen_string
    (
      writer,
      reinterpret_cast<const unsigned char*>( value ),
      static_cast<unsigned int>( value_len )
    );
    flushYAJLBuffer();
  }
}


// json_parser.cpp
using yidl::runtime::StringBuffer;

extern "C"
{
};


JSONParser::JSONValue JSONParser::JSONfalse;
JSONParser::JSONValue JSONParser::JSONnull;
JSONParser::JSONValue JSONParser::JSONtrue;


JSONParser::JSONParser()
{
  buffer = NULL;

  static yajl_callbacks JSONParser_yajl_callbacks
   = {
       handle_yajl_null,
       handle_yajl_boolean,
       handle_yajl_integer,
       handle_yajl_double,
       NULL,
       handle_yajl_string,
       handle_yajl_start_map,
       handle_yajl_map_key,
       handle_yajl_end_map,
       handle_yajl_start_array,
       handle_yajl_end_array
    };

  reader = yajl_alloc( &JSONParser_yajl_callbacks, NULL, this );
}

JSONParser::~JSONParser()
{
  Buffer::dec_ref( buffer );
  yajl_free( reader );
}

void JSONParser::handleJSONValue( JSONValue& json_value )
{
  if ( !json_value_stack.empty() )
  {
    switch ( json_value_stack.top()->get_type() )
    {
      case JSONValue::TYPE_ARRAY:
      {
        static_cast<JSONArray*>( json_value_stack.top() )->
          push_back( &json_value );
      }
      break;

      case JSONValue::TYPE_OBJECT:
      {
#ifdef _DEBUG
        if ( next_map_key == NULL ) DebugBreak();
#endif
        static_cast<JSONObject*>( json_value_stack.top() )->
          push_back( make_pair( next_map_key, &json_value ) );
        next_map_key = NULL;
      }
      break;

      default: DebugBreak();
    }
  }
  else
    json_value_stack.push( &json_value );
}

int JSONParser::handle_yajl_boolean( void* this__, int value )
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );

  if ( value )
    this_->handleJSONValue( JSONtrue.inc_ref() );
  else
    this_->handleJSONValue( JSONfalse.inc_ref() );

  return 1;
}

int JSONParser::handle_yajl_double( void* this__, double value )
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
  this_->handleJSONValue( *new JSONNumber( value ) );
  return 1;
}

int JSONParser::handle_yajl_end_array( void* this__ )
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
#ifdef _DEBUG
  if ( this_->json_value_stack.top()->get_type() != JSONValue::TYPE_ARRAY )
    DebugBreak();
#endif
  this_->json_value_stack.pop();
  return 1;
}

int JSONParser::handle_yajl_end_map( void* this__ )
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
#ifdef _DEBUG
  if ( this_->json_value_stack.top()->get_type() != JSONValue::TYPE_OBJECT )
    DebugBreak();
#endif
  this_->json_value_stack.pop();
  return 1;
}

int JSONParser::handle_yajl_integer( void* this__, long value )
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
  this_->handleJSONValue( *new JSONNumber( value ) );
  return 1;
}

int JSONParser::handle_yajl_map_key
(
  void* this__,
  const unsigned char* map_key,
  unsigned int map_key_len
)
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
  this_->next_map_key = new JSONString( *this_->buffer, map_key, map_key_len );
  return 1;
}

int JSONParser::handle_yajl_null( void* this__ )
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
  this_->handleJSONValue( JSONnull.inc_ref() );
  return 1;
}

int JSONParser::handle_yajl_start_array( void* this__ )
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
  JSONArray* json_array = new JSONArray;
  this_->handleJSONValue( *json_array );
  this_->json_value_stack.push( json_array );
  return 1;
}

int JSONParser::handle_yajl_start_map( void* this__ )
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
  JSONObject* json_object = new JSONObject;
  this_->handleJSONValue( *json_object );
  this_->json_value_stack.push( json_object );
  return 1;
}

int JSONParser::handle_yajl_string
(
  void* this__,
  const unsigned char* buffer,
  unsigned int len
)
{
  JSONParser* this_ = static_cast<JSONParser*>( this__ );
  this_->handleJSONValue( *new JSONString( *this_->buffer, buffer, len ) );
  return 1;
}

JSONParser::JSONValue* JSONParser::parse( Buffer& buffer )
{
  if ( this->buffer == NULL )
    this->buffer = &buffer.inc_ref();
  else
    this->buffer->put( buffer );

  const unsigned char* json_text = *this->buffer;
  unsigned int json_text_len = this->buffer->size();

  next_map_key = NULL;
  yajl_status yajl_parse_status = yajl_parse( reader, json_text, json_text_len );

  if ( yajl_parse_status == yajl_status_ok )
  {
    Buffer::dec_ref( this->buffer );
    this->buffer = NULL;

    if ( !json_value_stack.empty() )
    {
#ifdef _DEBUG
      if ( json_value_stack.size() > 1 ) DebugBreak();
#endif
      JSONValue* json_value = json_value_stack.top();
      json_value_stack.pop();
      return json_value;
    }
    else
      return new JSONObject;
  }
  else if ( yajl_parse_status == yajl_status_insufficient_data )
  {
    if ( this->buffer == &buffer )
    {
      this->buffer = new StringBuffer;
      this->buffer->put( buffer );
    }

    return NULL;
  }
  else
  {
    //unsigned char* yajl_error_str
    //  = yajl_get_error( reader, 1, json_text, json_text_len );
    //ostringstream what;
    //what << __FILE__ << ":" << __LINE__ << ": JSON parsing error: "
    //  << reinterpret_cast<char*>( yajl_error_str ) << endl;
    //yajl_free_error( yajl_error_str );
    return NULL;
  }
}

JSONParser::JSONValue* JSONParser::JSONObject::operator[]( const char* name )
{
  return NULL;
}

JSONParser::JSONString::JSONString
(
  Buffer& underlying_buffer,
  const unsigned char* value,
  unsigned int value_len
)
  : underlying_buffer( underlying_buffer.inc_ref() ),
    value( value ),
    value_len( value_len )

{ }

JSONParser::JSONString::operator const char*() const
{
  return reinterpret_cast<const char*>( value );
}

bool JSONParser::JSONString::operator==( const char* other ) const
{
  return strcmp( *this, other ) == 0;
}


// json_unmarshaller.cpp
using yidl::runtime::SubBuffer;


JSONUnmarshaller::JSONUnmarshaller( const JSONParser::JSONValue& root_json_value )
  : root_json_value( root_json_value )
{
  next_json_value_i = 0;
}

bool JSONUnmarshaller::read_bool( const Key& key )
{
  const JSONParser::JSONValue* json_value = read( key );
  if ( json_value != NULL )
    return json_value == &JSONParser::JSONtrue;
  else
    return false;
}

void JSONUnmarshaller::read( const Key& key, double& value )
{
  const JSONParser::JSONValue* json_value = read( key );
  if ( json_value != NULL )
  {
    if ( json_value->get_type() == JSONParser::JSONValue::TYPE_NUMBER )
      value = *static_cast<const JSONParser::JSONNumber*>( json_value );
    else
      value = 0;
  }
  else
    value = 0;
}

void JSONUnmarshaller::read( const Key& key, int64_t& value )
{
  const JSONParser::JSONValue* json_value = read( key );
  if ( json_value != NULL )
  {
    if ( json_value->get_type() == JSONParser::JSONValue::TYPE_NUMBER )
    {
      value
        = static_cast<int64_t>
          (
            *static_cast<const JSONParser::JSONNumber*>( json_value )
          );
    }
    else
      value = 0;
  }
  else
    value = 0;
}

JSONUnmarshaller::Key* JSONUnmarshaller::read( Key::Type key_type )
{
  if
  (
    key_type == Key::TYPE_STRING
    &&
    root_json_value.get_type() == JSONParser::JSONValue::TYPE_OBJECT
    &&
    next_json_value_i
    <
    static_cast<const JSONParser::JSONObject&>( root_json_value ).size()
  )
  {
    const JSONParser::JSONObject& json_object
      = static_cast<const JSONParser::JSONObject&>( root_json_value );
    const JSONParser::JSONString* name = json_object.at( next_json_value_i ).first;
    return new StringKey( *name, name->size() );
  }
  else
    return NULL;
}

const JSONParser::JSONValue* JSONUnmarshaller::read( const Key& key )
{
  if ( root_json_value.get_type() == JSONParser::JSONValue::TYPE_ARRAY )
  {
    const JSONParser::JSONArray& json_array
      = static_cast<const JSONParser::JSONArray&>( root_json_value );

    if ( next_json_value_i < json_array.size() )
      return json_array[next_json_value_i++];
    else
      return NULL;
  }
  else if ( root_json_value.get_type() == JSONParser::JSONValue::TYPE_OBJECT )
  {
    const JSONParser::JSONObject& json_object
      = static_cast<const JSONParser::JSONObject&>( root_json_value );

    if ( next_json_value_i < json_object.size() )
      return json_object.at( next_json_value_i++ ).second;
    else
      return NULL;
  }
  else
    return &root_json_value;
}

void JSONUnmarshaller::read( const Key& key, Map& value )
{
  const JSONParser::JSONValue* json_value = read( key );
  if ( json_value != NULL )
  {
    if ( json_value->get_type() == JSONParser::JSONValue::TYPE_OBJECT )
      JSONUnmarshaller( *json_value ).read( value );
  }
  else if ( root_json_value.get_type() == JSONParser::JSONValue::TYPE_OBJECT )
    JSONUnmarshaller( root_json_value ).read( value );
}

void JSONUnmarshaller::read( Map& value )
{
  size_t size
    = static_cast<const JSONParser::JSONObject&>( root_json_value ).size();
  while ( next_json_value_i < size )
    value.unmarshal( *this );
}

void JSONUnmarshaller::read( const Key& key, MarshallableObject& value )
{
  const JSONParser::JSONValue* json_value = read( key );
  if ( json_value != NULL )
  {
    if ( json_value->get_type() == JSONParser::JSONValue::TYPE_OBJECT )
      JSONUnmarshaller( *json_value ).read( value );
  }
  else if ( root_json_value.get_type() == JSONParser::JSONValue::TYPE_OBJECT )
    JSONUnmarshaller( root_json_value ).read( value );
}

void JSONUnmarshaller::read( MarshallableObject& value )
{
  value.unmarshal( *this );
}

void JSONUnmarshaller::read( const Key& key, Sequence& value )
{
  const JSONParser::JSONValue* json_value = read( key );
  if ( json_value != NULL )
  {
    if ( json_value->get_type() == JSONParser::JSONValue::TYPE_ARRAY )
      JSONUnmarshaller( *json_value ).read( value );
  }
  else if ( root_json_value.get_type() == JSONParser::JSONValue::TYPE_ARRAY )
    JSONUnmarshaller( root_json_value ).read( value );
}

void JSONUnmarshaller::read( Sequence& value )
{
  size_t size
    = static_cast<const JSONParser::JSONArray&>( root_json_value ).size();
  while ( next_json_value_i < size )
    value.unmarshal( *this );
}

void JSONUnmarshaller::read( const Key& key, string& value )
{
  const JSONParser::JSONValue* json_value = read( key );
  if
  (
    json_value != NULL
    &&
    json_value->get_type() == JSONParser::JSONValue::TYPE_STRING
  )
  {
    value.assign
    (
      *static_cast<const JSONParser::JSONString*>( json_value ),
      static_cast<const JSONParser::JSONString*>( json_value )->size()
    );
  }
}


// oncrpc_client.cpp
using yidl::runtime::StackBuffer;


#ifdef _WIN32
#define ETIMEDOUT 10060 // WSAETIMEDOUT
#endif


class ONCRPCClient::Connection
  : public SocketClient::Connection,
    private ONCRPCResponseParser
{
public:
  Connection
  (
    ONCRPCClient& oncrpc_client,
    Socket& socket_
  )
  : SocketClient::Connection( socket_, oncrpc_client ),
    ONCRPCResponseParser( oncrpc_client.get_message_factory() ),
    operation_timeout( oncrpc_client.get_operation_timeout() )
  { }

  // RequestHandler
  void handleRequest( Request& request )
  {
    if ( request.get_type_id() == ONCRPCRequest::TYPE_ID )
    {
      ONCRPCRequest& oncrpc_request = static_cast<ONCRPCRequest&>( request );

      XDRMarshaller xdr_marshaller;
      oncrpc_request.marshal( xdr_marshaller );
      ONCRPCRecordFragment* record_fragment
        = new ONCRPCRecordFragment( xdr_marshaller.get_buffer() );

      if ( get_socket().is_connected() )
        get_socket().aio_send( *record_fragment, 0, *this, &oncrpc_request );
      else
      {
        get_socket().aio_connect
        (
          get_peername(),
          *this,
          &oncrpc_request,
          record_fragment
        );
      }
    }
    else
      DebugBreak();
  }

private:
  // Socket::AIOConnectCallback
  void onConnectCompletion( size_t bytes_written, void* context )
  {
    SocketClient::Connection::onConnectCompletion( bytes_written, context );

    onWriteCompletion( bytes_written, context );
  }

  void onConnectError( uint32_t error_code, void* context )
  {
    SocketClient::Connection::onConnectError( error_code, context );

    ONCRPCRequest::dec_ref( static_cast<ONCRPCRequest*>( context ) );
  }

  // Socket::AIOReadCallback
  void onReadCompletion( Buffer& buffer, void* context )
  {
    SocketClient::Connection::onReadCompletion( buffer, context );

    if ( buffer.size() == buffer.capacity() )
    {
      ONCRPCRequest* oncrpc_request = static_cast<ONCRPCRequest*>( context );

      if ( oncrpc_request == NULL )
      {
        uint32_t nbo_record_fragment_marker, nbo_xid;
        buffer.get( &nbo_record_fragment_marker, sizeof( uint32_t ) );
        buffer.get( &nbo_xid, sizeof( nbo_xid ) );
#ifdef __MACH__
        uint32_t hbo_xid = ntohl( nbo_xid );
#else
        uint32_t hbo_xid = XDRUnmarshaller::ntohl( nbo_xid );
#endif

        map<uint32_t, ONCRPCRequest*>::iterator oncrpc_request_i
          = outstanding_oncrpc_requests.find( hbo_xid );

        if ( oncrpc_request_i != outstanding_oncrpc_requests.end() )
        {
          oncrpc_request = oncrpc_request_i->second;
          outstanding_oncrpc_requests.erase( oncrpc_request_i );

          ONCRPCRecordFragment* record_fragment
            = new ONCRPCRecordFragment( nbo_record_fragment_marker );

          record_fragment->put( &nbo_xid, sizeof( nbo_xid ) );
          record_fragment->position
          (
            record_fragment->position() + sizeof( nbo_xid )
          );

          get_socket().aio_read( *record_fragment, *this, oncrpc_request );
        }
        else
          DebugBreak();
      }
      else
      {
        ONCRPCResponse* oncrpc_response
          = ONCRPCResponseParser::parse
            (
              static_cast<ONCRPCRecordFragment&>( buffer ),
              *oncrpc_request
            );

        if ( oncrpc_response != NULL )
        {
          oncrpc_request->respond( *oncrpc_response );
          ONCRPCRequest::dec_ref( *oncrpc_request );

          get_socket().aio_recv( *new StackBuffer<8>, 0, *this, NULL );
        }
        else
          DebugBreak();
      }
    }
    else // buffer.size() < buffer.capacity()
      get_socket().aio_recv( buffer, 0, *this, context );

  }

  void onReadError( uint32_t error_code, void* context )
  {
    SocketClient::Connection::onReadError( error_code, context );

    ONCRPCRequest::dec_ref( static_cast<ONCRPCRequest*>( context ) );
  }

  // Socket::AIOWriteCallback
  void onWriteCompletion( size_t bytes_written, void* context )
  {
    SocketClient::Connection::onWriteCompletion( bytes_written, context );

    ONCRPCRequest* oncrpc_request = static_cast<ONCRPCRequest*>( context );
    outstanding_oncrpc_requests[oncrpc_request->get_xid()] = oncrpc_request;

    get_socket().aio_recv( *new StackBuffer<8>, 0, *this, NULL );
  }

  void onWriteError( uint32_t error_code, void* context )
  {
    SocketClient::Connection::onWriteError( error_code, context );

    ONCRPCRequest::dec_ref( static_cast<ONCRPCRequest*>( context ) );
  }

private:
  Time operation_timeout;
  map<uint32_t, ONCRPCRequest*> outstanding_oncrpc_requests;
};


ONCRPCClient::ONCRPCClient
(
  uint16_t concurrency_level,
  MarshallableObject* cred,
  Log* error_log,
  IOQueue& io_queue,
  MessageFactory& message_factory,
  const Time& operation_timeout,
  SocketAddress& peername,
  uint32_t prog,
  uint16_t reconnect_tries_max,
  SocketFactory& socket_factory,
  Log* trace_log,
  uint32_t vers
)
  : SocketClient
    (
      error_log,
      io_queue,
      operation_timeout,
      peername,
      reconnect_tries_max,
      trace_log
    ),
    RPCClient( message_factory ),
    cred( Object::inc_ref( cred ) ),
    prog( prog ),
    vers( vers )
{
  uint16_t connection_i = 0;
  for ( ; connection_i < concurrency_level; connection_i++ )
  {
    Socket* socket_ = socket_factory.createSocket();
    if ( socket_ != NULL )
    {
      if ( socket_->associate( io_queue ) )
        connections.enqueue( new Connection( *this, *socket_ ) );
      else
      {
        Socket::dec_ref( *socket_ );
        break;
      }
    }
    else
      break;
  }

#ifdef _DEBUG
  if ( connection_i == 0 ) DebugBreak();
#endif
  SocketFactory::dec_ref( socket_factory );
}

ONCRPCClient::~ONCRPCClient()
{
  Connection* connection = connections.try_dequeue();
  while ( connection != NULL )
  {
    Connection::dec_ref( *connection );
    connection = connections.try_dequeue();
  }
}

ONCRPCClient&
ONCRPCClient::create
(
  const URI& absolute_uri,
  MessageFactory& message_factory,
  uint32_t prog,
  uint32_t vers,
  uint16_t concurrency_level,
  MarshallableObject* cred,
  Log* error_log,
  const Time& operation_timeout,
  uint16_t reconnect_tries_max,
  SSLContext* ssl_context,
  Log* trace_log
)
{
  SocketAddress& peername = createSocketAddress( absolute_uri );

  IOQueue* io_queue;
  SocketFactory* socket_factory;
#ifdef YIELD_IPC_HAVE_OPENSSL
  if ( absolute_uri.get_scheme() == "oncrpcs" )
  {
    io_queue = &yield::platform::NBIOQueue::create();
    if ( ssl_context == NULL )
      ssl_context = &SSLContext::create( SSLv23_client_method() );
    socket_factory = new SSLSocketFactory( *ssl_context );
    SSLContext::dec_ref( *ssl_context );
  }
  else
#endif
  {
    io_queue = &createIOQueue();
    if ( absolute_uri.get_scheme() == "oncrpcu" )
      socket_factory = new UDPSocketFactory;
    else if ( trace_log!= NULL )
      socket_factory = new TracingTCPSocketFactory( *trace_log );
    else
      socket_factory = new TCPSocketFactory;
  }

  return *new ONCRPCClient
              (
                concurrency_level,
                cred,
                error_log,
                *io_queue,
                message_factory,
                operation_timeout,
                peername,
                prog,
                reconnect_tries_max,
                *socket_factory,
                trace_log,
                vers
              );
}

void ONCRPCClient::handleRequest( Request& request )
{
  ONCRPCRequest* oncrpc_request;

  switch ( request.get_type_id() )
  {
    case ONCRPCRequest::TYPE_ID:
    {
      oncrpc_request = static_cast<ONCRPCRequest*>( &request );
    }
    break;

    default:
    {
      oncrpc_request
        = new ONCRPCRequest
              (
                request, // Steals this reference
                request.get_type_id(),
                prog,
                vers,
                static_cast<uint32_t>( Time().as_unix_time_s() ),
                get_cred()
              );

      ResponseTarget* response_target = new RPCResponseTarget( request );
      oncrpc_request->set_response_target( response_target );
      ResponseTarget::dec_ref( *response_target );
    }
    break;
  }

  Connection* connection = connections.dequeue();
  connection->send( *oncrpc_request );
}


// oncrpc_message.cpp
ONCRPCMessage::ONCRPCMessage( MarshallableObject* verf, uint32_t xid )
  : verf( verf ), xid( xid )
{ }

ONCRPCMessage::~ONCRPCMessage()
{
  MarshallableObject::dec_ref( verf );
}

void ONCRPCMessage::marshal_opaque_auth
(
  Marshaller& marshaller,
  MarshallableObject* opaque_auth
) const
{
  if ( opaque_auth != NULL  )
  {
    marshaller.write( "auth_flavor", opaque_auth->get_type_id() );
    XDRMarshaller temp_xdr_marshaller;
    opaque_auth->marshal( temp_xdr_marshaller );
    marshaller.write( "auth_body", temp_xdr_marshaller.get_buffer() );
  }
  else
  {
    marshaller.write( "auth_flavor", ONCRPCMessage::AUTH_NONE );
    marshaller.write( "auth_body_length", 0 );
  }
}


// oncrpc_message_parser.cpp
ONCRPCMessageParser::ONCRPCMessageParser( MessageFactory& message_factory )
  : message_factory( message_factory.inc_ref() )
{ }

ONCRPCMessageParser::~ONCRPCMessageParser()
{
  MessageFactory::dec_ref( message_factory );
}

MarshallableObject*
ONCRPCMessageParser::unmarshal_opaque_auth
(
  XDRUnmarshaller& xdr_unmarshaller
)
{
  uint32_t auth_flavor = xdr_unmarshaller.read_uint32();
  if ( auth_flavor != ONCRPCMessage::AUTH_NONE )
  {
    MarshallableObject* opaque_auth
      = message_factory.createMarshallableObject( auth_flavor );

    if ( opaque_auth != NULL )
    {
      xdr_unmarshaller.read_int32(); // length
      xdr_unmarshaller.read( *opaque_auth );
      return opaque_auth;
    }
  }

  string opaque_auth;
  xdr_unmarshaller.read( opaque_auth );

  return NULL;
}


// oncrpc_record_fragment.cpp
ONCRPCRecordFragment::ONCRPCRecordFragment( uint32_t nbo_marker )
  : yidl::runtime::HeapBuffer
    (
      sizeof( nbo_marker )
      +
#ifdef __MACH__
      ntohl( nbo_marker ) ^ ( 1 << 31UL )
#else
      XDRUnmarshaller::ntohl( nbo_marker ) ^ ( 1 << 31UL )
#endif
    )
{
  put( &nbo_marker, sizeof( nbo_marker ) );
  position( sizeof( nbo_marker ) );

#ifdef __MACH__
  hbo_marker = ntohl( nbo_marker );
#else
  hbo_marker = XDRUnmarshaller::ntohl( nbo_marker );
#endif
}

ONCRPCRecordFragment::ONCRPCRecordFragment( Buffer& buffer, bool is_last )
  : yidl::runtime::HeapBuffer
    (
      buffer.size() - buffer.position() + sizeof( hbo_marker )
    )
{
  hbo_marker = buffer.size() - buffer.position();
  if ( is_last ) hbo_marker |= ( 1 << 31UL );

  uint32_t nbo_marker;
#ifdef __MACH__
  nbo_marker = htonl( marker );
#else
  nbo_marker = XDRMarshaller::htonl( hbo_marker );
#endif
  put( &nbo_marker, sizeof( nbo_marker ) );

  Buffer::put( buffer );
}

bool ONCRPCRecordFragment::is_last() const
{
  return ( hbo_marker >> 31 ) != 0;
}


// oncrpc_request.cpp
ONCRPCRequest::ONCRPCRequest
(
  Request& body,
  uint32_t proc,
  uint32_t prog,
  uint32_t vers,
  uint32_t xid,
  MarshallableObject* cred,
  MarshallableObject* verf
)
  : ONCRPCMessage( verf, xid ),
    body( body ),
    cred( Object::inc_ref( cred ) ),
    proc( proc ),
    prog( prog ),
    vers( vers )
{ }

ONCRPCRequest::~ONCRPCRequest()
{
  Request::dec_ref( body );
  MarshallableObject::dec_ref( cred );
}

void ONCRPCRequest::marshal( Marshaller& marshaller ) const
{
  marshaller.write( "xid", get_xid() );
  marshaller.write( "msg_type", CALL );
  marshaller.write( "rpcvers", 2 );
  marshaller.write( "prog", get_prog() );
  marshaller.write( "vers", get_vers() );
  marshaller.write( "proc", get_proc() );
  marshal_opaque_auth( marshaller, get_cred() );
  marshal_opaque_auth( marshaller, get_verf() );
  marshaller.write( "body", get_body() );
}

void ONCRPCRequest::respond( ONCRPCResponse& response )
{
  Request::respond( response );
}

void ONCRPCRequest::respond( Response& response )
{
  Request::respond( *new ONCRPCResponse( response, get_xid(), get_verf() ) );
}

void ONCRPCRequest::respond( ExceptionResponse& response )
{
  Request::respond( *new ONCRPCResponse( response, get_xid(), get_verf() ) );
}


// oncrpc_request_parser.cpp
ONCRPCRequestParser::ONCRPCRequestParser( MessageFactory& message_factory )
  : ONCRPCMessageParser( message_factory )
{ }

Message*
ONCRPCRequestParser::parse
(
  ONCRPCRecordFragment& record_fragment
)
{
  XDRUnmarshaller xdr_unmarshaller( record_fragment );

  uint32_t xid = xdr_unmarshaller.read_uint32();

  uint32_t msg_type = xdr_unmarshaller.read_uint32();
  if ( msg_type == ONCRPCMessage::CALL )
  {
    uint32_t rpcvers = xdr_unmarshaller.read_uint32();
    if ( rpcvers == 2 )
    {
      uint32_t prog = xdr_unmarshaller.read_uint32();
      uint32_t vers = xdr_unmarshaller.read_uint32();
      uint32_t proc = xdr_unmarshaller.read_uint32();

      MarshallableObject* cred = unmarshal_opaque_auth( xdr_unmarshaller );
      MarshallableObject* verf = unmarshal_opaque_auth( xdr_unmarshaller );

      Request* body = get_message_factory().createRequest( proc );
      if ( body != NULL )
      {
        xdr_unmarshaller.read( *body );
        return new ONCRPCRequest( *body, proc, prog, vers, xid, cred, verf );
      }
      else
      {
        MarshallableObject::dec_ref( cred );
        return new ONCRPCResponse
                   (
                     *new ONCRPCProcedureUnavailableError,
                     xid,
                     verf
                   );
      }
    }
    else
    {
      ONCRPCResponse::mismatch_info mismatch_info_ = { 0 };
      return new ONCRPCResponse( mismatch_info_, xid );
    }
  }
  else
    return new ONCRPCResponse( *new ONCRPCGarbageArgumentsError, xid );
}


// oncrpc_response.cpp
ONCRPCResponse::ONCRPCResponse
(
  Response& body,
  uint32_t xid,
  MarshallableObject* verf
)
  : ONCRPCMessage( verf, xid ),
    accept_stat_( SUCCESS ),
    body( body ),
    auth_stat_( AUTH_OK ),
    reply_stat_( MSG_ACCEPTED ),
    reject_stat_( RPC_MISMATCH )
{
  memset( &mismatch_info_, 0, sizeof( mismatch_info_ ) );
}

ONCRPCResponse::ONCRPCResponse
(
  ExceptionResponse& body,
  uint32_t xid,
  MarshallableObject* verf
)
  : ONCRPCMessage( verf, xid ),
    accept_stat_( body.get_type_id() ),
    body( body ),
    auth_stat_( AUTH_OK ),
    reply_stat_( MSG_ACCEPTED ),
    reject_stat_( RPC_MISMATCH )
{
  memset( &mismatch_info_, 0, sizeof( mismatch_info_ ) );
}

ONCRPCResponse::ONCRPCResponse
(
  const struct mismatch_info& mismatch_info_,
  uint32_t xid
)
 : ONCRPCMessage( NULL, xid ),
   accept_stat_( SYSTEM_ERR ),
   body( *new ONCRPCRPCMismatchError ),
   auth_stat_( AUTH_OK ),
   mismatch_info_( mismatch_info_ ),
   reply_stat_( MSG_DENIED ),
   reject_stat_( RPC_MISMATCH )
{ }

ONCRPCResponse::ONCRPCResponse( auth_stat auth_stat_, uint32_t xid )
  : ONCRPCMessage( NULL, xid ),
    accept_stat_( SYSTEM_ERR ),
    body( *new ONCRPCAuthError( auth_stat_ ) ),
    auth_stat_( auth_stat_ ),
    reply_stat_( MSG_DENIED ),
    reject_stat_( AUTH_ERROR )
{
  memset( &mismatch_info_, 0, sizeof( mismatch_info_ ) );
}

ONCRPCResponse::~ONCRPCResponse()
{
  Response::dec_ref( body );
}

void ONCRPCResponse::marshal( Marshaller& marshaller ) const
{
  marshaller.write( "xid", get_xid() );
  marshaller.write( "msg_type", REPLY );
  marshaller.write( "reply_stat", reply_stat_ );
  if ( reply_stat_ == MSG_ACCEPTED )
  {
    marshal_opaque_auth( marshaller, get_verf() );
    marshaller.write( "accept_stat", accept_stat_ );
    marshaller.write( "body", get_body() );
  }
  else
  {
    marshaller.write( "reject_stat", reject_stat_ );
    if ( reject_stat_ == RPC_MISMATCH )
    {
      marshaller.write( "mismatch_info_low", mismatch_info_.low );
      marshaller.write( "mismatch_info_high", mismatch_info_.high );
    }
    else
      marshaller.write( "auth_stat", auth_stat_ );
  }
}


// oncrpc_response_parser.cpp
ONCRPCResponseParser::ONCRPCResponseParser( MessageFactory& message_factory )
  : ONCRPCMessageParser( message_factory )
{ }

ONCRPCResponse*
ONCRPCResponseParser::parse
(
  ONCRPCRecordFragment& record_fragment,
  ONCRPCRequest& oncrpc_request
)
{
  return parse
         (
           oncrpc_request.get_body().get_type_id(),
           record_fragment,
           oncrpc_request.get_xid()
         );
}

ONCRPCResponse*
ONCRPCResponseParser::parse
(
  uint32_t default_body_type_id,
  ONCRPCRecordFragment& record_fragment,
  uint32_t xid
)
{
  XDRUnmarshaller xdr_unmarshaller( record_fragment );

  uint32_t msg_type = xdr_unmarshaller.read_uint32();
  if ( msg_type == ONCRPCMessage::REPLY )
  {
    uint32_t reply_stat = xdr_unmarshaller.read_uint32();
    if ( reply_stat == ONCRPCResponse::MSG_ACCEPTED )
    {
      MarshallableObject* verf = unmarshal_opaque_auth( xdr_unmarshaller );

      uint32_t accept_stat = xdr_unmarshaller.read_uint32();
      switch ( accept_stat )
      {
        case ONCRPCResponse::SUCCESS:
        {
          Response* body
            = get_message_factory().createResponse( default_body_type_id );

          if ( body != NULL )
          {
            xdr_unmarshaller.read( *body );
            return new ONCRPCResponse( *body, xid, verf );
          }
          else
          {
            return new ONCRPCResponse
                       (
                         *new ONCRPCProcedureUnavailableError,
                         xid,
                         verf
                       );
          }
        }
        break;

        case ONCRPCResponse::PROG_UNAVAIL:
        {
          return new ONCRPCResponse
                     (
                       *new ONCRPCProgramUnavailableError,
                       xid,
                       verf
                     );
        }
        break;

        case ONCRPCResponse::PROG_MISMATCH:
        {
          return new ONCRPCResponse
                     (
                       *new ONCRPCProgramMismatchError,
                       xid,
                       verf
                     );
        }
        break;

        case ONCRPCResponse::PROC_UNAVAIL:
        {
          return new ONCRPCResponse
                     (
                       *new ONCRPCProcedureUnavailableError,
                       xid,
                       verf
                     );
        }
        break;

        case ONCRPCResponse::GARBAGE_ARGS:
        {
          return new ONCRPCResponse
                     (
                       *new ONCRPCGarbageArgumentsError,
                       xid,
                       verf
                     );

        }
        break;

        case ONCRPCResponse::SYSTEM_ERR:
        {
          return new ONCRPCResponse
                     (
                       *new ONCRPCSystemError,
                       xid,
                       verf
                     );
        }
        break;

        default:
        {
          ExceptionResponse* body
            = get_message_factory().createExceptionResponse( accept_stat );

          if ( body != NULL )
            xdr_unmarshaller.read( *body );
          else
            body = new ONCRPCSystemError;

          return new ONCRPCResponse( *body, xid, verf );
        }
        break;
      }
    }
    else if ( reply_stat == ONCRPCResponse::MSG_DENIED )
    {
      uint32_t reject_stat = xdr_unmarshaller.read_uint32();
      if ( reject_stat == ONCRPCResponse::RPC_MISMATCH )
      {
        struct ONCRPCResponse::mismatch_info mismatch_info_;
        mismatch_info_.low = xdr_unmarshaller.read_uint32();
        mismatch_info_.high =  xdr_unmarshaller.read_uint32();
        return new ONCRPCResponse( mismatch_info_, xid );
      }
      else if ( reject_stat == ONCRPCResponse::AUTH_ERROR )
      {
        uint32_t auth_stat = xdr_unmarshaller.read_uint32();
        if
        (
          auth_stat >= ONCRPCResponse::AUTH_OK
          &&
          auth_stat <= ONCRPCResponse::AUTH_FAILED
        )
        {
          return new ONCRPCResponse
                     (
                       static_cast<ONCRPCResponse::auth_stat>( auth_stat ),
                       xid
                     );
        }
        else
          return NULL;
      }
      else // Unknown reject_stat
        return NULL;
    }
    else // Unknown reply_stat value
      return NULL;
  }
  else // msg_type != REPLY
    return NULL;
}


// oncrpc_server.cpp
using yidl::runtime::StackBuffer;


namespace yield
{
  namespace ipc
  {
    class TCPONCRPCServer : public TCPSocketServer, private ONCRPCServer
    {
    public:
      static TCPONCRPCServer&
      create
      (
        const URI& absolute_uri,
        Log* error_log,
        MessageFactory& message_factory,
        EventTarget& request_target,
        bool send_oncrpc_requests,
        SSLContext* ssl_context,
        Log* trace_log
      )
      {
        IOQueue& io_queue = createIOQueue( ssl_context != NULL );

        return *new TCPONCRPCServer
                    (
                      error_log,
                      io_queue,
                      createListenTCPSocket
                      (
                        absolute_uri,
                        io_queue,
                        ssl_context,
                        trace_log
                      ),
                      message_factory,
                      request_target,
                      send_oncrpc_requests,
                      trace_log
                    );
      }

      // EventHandler
      const char* get_name() const { return "TCPONCRPCServer"; }

    private:
      TCPONCRPCServer
      (
        Log* error_log,
        IOQueue& io_queue,
        TCPSocket& listen_tcp_socket,
        MessageFactory& message_factory,
        EventTarget& request_target,
        bool send_oncrpc_requests,
        Log* trace_log
      )
      : TCPSocketServer
        (
          error_log,
          io_queue,
          listen_tcp_socket,
          request_target,
          trace_log
        ),
        ONCRPCServer
        (
          message_factory,
          request_target,
          send_oncrpc_requests
        )
      {
        get_listen_tcp_socket().aio_accept( *this, NULL, new StackBuffer<4> );
      }

      // TCPSocket::AIOAcceptCallback
      void
      onAcceptCompletion
      (
        TCPSocket& accepted_tcp_socket,
        void* context,
        Buffer* recv_buffer
      )
      {
        TCPSocketServer::onAcceptCompletion
        (
          accepted_tcp_socket,
          context,
          recv_buffer
        );

        Connection* connection = new Connection( accepted_tcp_socket, *this );
        connection->onReadCompletion( *recv_buffer, NULL );

        // get_listen_tcp_socket().aio_accept( *this, NULL, new StackBuffer<4> );
      }

    private:
      class Connection
        : public TCPSocketServer::Connection,
          private ONCRPCRequestParser
      {
      public:
        Connection( TCPSocket& tcp_socket, TCPONCRPCServer& tcp_oncrpc_server )
          : TCPSocketServer::Connection( tcp_socket, tcp_oncrpc_server ),
            ONCRPCRequestParser( tcp_oncrpc_server.get_message_factory() )
        { }

        // TCPSocket::AIOReadCallback
        void onReadCompletion( Buffer& buffer, void* context )
        {
          TCPSocketServer::Connection::onReadCompletion( buffer, context );

          if ( buffer.size() == buffer.capacity() )
          {
            if ( buffer.get_type_id() == StackBuffer<4>::TYPE_ID )
            {
              uint32_t nbo_record_fragment_marker;
              buffer.get( &nbo_record_fragment_marker, sizeof( uint32_t ) );

              ONCRPCRecordFragment* record_fragment
                = new ONCRPCRecordFragment( nbo_record_fragment_marker );

              get_socket().aio_recv( *record_fragment, 0, *this, NULL );
            }
            else if ( buffer.get_type_id() == ONCRPCRecordFragment::TYPE_ID )
            {
              Message* message
                = ONCRPCRequestParser::parse
                  (
                    static_cast<ONCRPCRecordFragment&>( buffer )
                  );

              if ( message != NULL )
              {
                if ( message->get_type_id() == ONCRPCRequest::TYPE_ID )
                {
                  ONCRPCRequest* oncrpc_request
                    = static_cast<ONCRPCRequest*>( message );
                  oncrpc_request->set_response_target( this );
                  get_tcp_oncrpc_server().sendRPCRequest( *oncrpc_request );
                }
                else if ( message->get_type_id() == ONCRPCResponse::TYPE_ID )
                  send( *static_cast<ONCRPCResponse*>( message ) );
                else
                  DebugBreak();
              }
              else
                DebugBreak();
            }
            else
              DebugBreak();
          }
          else // buffer.size() < buffer.capacity()
            get_socket().aio_recv( buffer, 0, *this, context );
        }

        // TCPSocket::AIOWriteCallback
        void onWriteCompletion( size_t bytes_written, void* context )
        {
          TCPSocketServer::Connection::onWriteCompletion
          (
            bytes_written,
            context
          );

          get_socket().aio_recv( *new StackBuffer<4>, 0, *this, NULL );
        }

        // ResponseTarget
        void send( Response& response )
        {
          if ( response.get_type_id() == ONCRPCResponse::TYPE_ID )
          {
            ONCRPCResponse& oncrpc_response
              = static_cast<ONCRPCResponse&>( response );

            XDRMarshaller xdr_marshaller;
            oncrpc_response.marshal( xdr_marshaller );
            ONCRPCResponse::dec_ref( oncrpc_response );

            ONCRPCRecordFragment* record_fragment =
              new ONCRPCRecordFragment( xdr_marshaller.get_buffer() );

            get_socket().aio_send( *record_fragment, 0, *this, NULL );
          }
          else
            DebugBreak();
        }

      private:
        TCPONCRPCServer& get_tcp_oncrpc_server() const
        {
          return static_cast<TCPONCRPCServer&>( get_socket_server() );
        }
      };
    };


    class UDPONCRPCServer
      : public UDPSocketServer,
        private ONCRPCServer,
        private ONCRPCRequestParser
    {
    public:
      static UDPONCRPCServer&
      create
      (
        const URI& absolute_uri,
        Log* error_log,
        MessageFactory& message_factory,
        EventTarget& request_target,
        bool send_oncrpc_requests,
        Log* trace_log
      )
      {
        IOQueue& io_queue = createIOQueue();
        return *new UDPONCRPCServer
                    (
                      error_log,
                      io_queue,
                      message_factory,
                      request_target,
                      send_oncrpc_requests,
                      trace_log,
                      createUDPSocket( absolute_uri, io_queue, trace_log )
                    );
      }

      // EventHandler
      const char* get_name() const { return "UDPONCRPCServer"; }

    private:
      UDPONCRPCServer
      (
        Log* error_log,
        IOQueue& io_queue,
        MessageFactory& message_factory,
        EventTarget& request_target,
        bool send_oncrpc_requests,
        Log* trace_log,
        UDPSocket& udp_socket
      )
      : UDPSocketServer
        (
          error_log,
          io_queue,
          request_target,
          trace_log,
          udp_socket
        ),
        ONCRPCServer
        (
          message_factory,
          request_target,
          send_oncrpc_requests
        ),
        ONCRPCRequestParser( message_factory )
      {
        // get_udp_socket().aio_recvfrom( *new StackBuffer<1024>, *this, NULL );
      }

      // UDPSocket::AIORecvFromCallback
      void
      onRecvFromCompletion
      (
        Buffer& buffer,
        SocketAddress& peername,
        void* context
      )
      {
        DebugBreak();
        //ONCRPCRequest* request = static_cast<ONCRPCRequest*>( context );
        //
        //Buffer* next_buffer = request->deserialize( &buffer );
        //if ( next_buffer == NULL )
        //{
        //  UDPConnection* udp_connection = new UDPConnection( peername, *udp_socket );
        //  request->set_response_target( udp_connection );
        //  UDPConnection::dec_ref( *udp_connection );
        //  sendRequest( *request );
        //}
        //else if ( next_buffer != reinterpret_cast<Buffer*>( -1 ) )
        //  Buffer::dec_ref( *next_buffer );

        //request = createRequest();
        //udp_socket->aio_recvfrom( *request->deserialize( NULL ), *this, request );
      }

      void onRecvFromError( uint32_t, void* context )
      {
        DebugBreak();
        //ONCRPCRequest* request = static_cast<ONCRPCRequest*>( context );
        //delete request;
        //request = createRequest();
        //udp_socket->aio_recvfrom( *request->deserialize( NULL ), *this, request );
      }

    private:
      class Connection : public UDPSocketServer::Connection
      {
      public:
        Connection
        (
          SocketAddress& peername,
          UDPSocket& udp_socket,
          UDPSocketServer& udp_socket_server
        )
          : UDPSocketServer::Connection
            (
              peername,
              udp_socket,
              udp_socket_server
            )
        { }

        // ResponseTarget
        void send( Response& response )
        {
          //if ( request_target.get_type_id() == ONCRPC
          //switch ( ev.get_type_id() )
          //{
          //  case ONCRPCResponse::TYPE_ID:
          //  {
          //    ONCRPCResponse& response = static_cast<ONCRPCResponse&>( ev );
          //    DebugBreak();
          //    //response.serialize( udp_socket, *this, NULL );
          //    //ONCRPCResponse::dec_ref( response );
          //  }
          //  break;

          //  default: Event::dec_ref( ev );
          //}
        }
      };
    };
  };
};


ONCRPCServer::ONCRPCServer
(
  MessageFactory& message_factory,
  EventTarget& request_target,
  bool send_oncrpc_requests
) : RPCServer
    (
      message_factory,
      request_target,
      send_oncrpc_requests
    )
{ }

SocketServer&
ONCRPCServer::create
(
  const URI& absolute_uri,
  MessageFactory& message_factory,
  EventTarget& request_target,
  Log* error_log,
  bool send_oncrpc_requests,
  SSLContext* ssl_context,
  Log* trace_log
)
{
  if ( absolute_uri.get_scheme() == "oncrpcu" )
  {
    return UDPONCRPCServer::create
           (
             absolute_uri,
             error_log,
             message_factory,
             request_target,
             send_oncrpc_requests,
             trace_log
           );
  }
  else
  {
    return TCPONCRPCServer::create
           (
             absolute_uri,
             error_log,
             message_factory,
             request_target,
             send_oncrpc_requests,
             ssl_context,
             trace_log
           );
  }
}


// socket_client.cpp
SocketClient::SocketClient
(
  Log* error_log,
  IOQueue& io_queue,
  const Time& operation_timeout,
  SocketAddress& peername,
  uint16_t reconnect_tries_max,
  Log* trace_log
)
  : SocketPeer( error_log, io_queue, trace_log ),
    operation_timeout( operation_timeout ),
    peername( peername ),
    reconnect_tries_max( reconnect_tries_max )
{ }

SocketClient::~SocketClient()
{
  SocketAddress::dec_ref( peername );
}

SocketClient::Connection::Connection
(
  Socket& socket_,
  SocketClient& socket_client
) : socket_( socket_ ),
    socket_client( socket_client )
{
  reconnect_tries = 0;
}

SocketClient::Connection::~Connection()
{
  Socket::dec_ref( socket_ );
}

//void SocketClient::Connection::connect()
//{
//  if ( socket_.is_connected() )
//    onConnectCompletion( NULL );
//  else
//  {
//    if ( socket_client.get_trace_log() != NULL )
//    {
//      socket_client.get_log()->get_stream( Log::LOG_INFO ) <<
//      "yield::ipc::SocketClient: connecting to <host>:" <<
//      socket_client.get_peername().get_port() <<
//      " with socket #" << reinterpret_cast<uint64_t>( &socket_ ) <<
//      " (try #" <<
//      reconnect_tries+1
//      << ").";
//    }
//
//    socket_.aio_connect( socket_client.get_peername(), *this );
//  }
//}

SocketAddress& SocketClient::Connection::get_peername() const
{
  return socket_client.get_peername();
}

void SocketClient::Connection::onConnectCompletion( size_t, void* )
{
  if ( socket_client.get_trace_log() != NULL )
  {
    socket_client.get_trace_log()->get_stream( Log::LOG_INFO ) <<
      "yield::ipc::SocketClient: successfully connected to <host>:" <<
      socket_client.get_peername().get_port() << " on socket #" <<
      reinterpret_cast<uint64_t>( &socket_ ) << ".";

    //log->get_stream( Log::LOG_INFO ) <<
    //"yield::ipc::SocketClient: writing " << request.get_type_name()
    //<< "/" << reinterpret_cast<uint64_t>( &request ) <<
    //" to <host>:" << client.get_peername().get_port() <<
    //" on socket #" << reinterpret_cast<uint64_t>( &socket_ ) << ".";
  }
}

void SocketClient::Connection::onConnectError( uint32_t error_code, void* )
{
  if ( socket_client.get_error_log() != NULL )
  {
    socket_client.get_error_log()->get_stream( Log::LOG_ERR ) <<
      "yield::ipc::SocketClient: connect() to <host>:" <<
      socket_client.get_peername().get_port() <<
      " failed: " << Exception( error_code ) << ".";
  }
}

void SocketClient::Connection::onReadCompletion( Buffer&, void* )
{
  if ( socket_client.get_trace_log() != NULL )
  {
  }
}

void SocketClient::Connection::onReadError( uint32_t error_code, void* )
{
  if ( socket_client.get_error_log() != NULL )
  {
    //log->get_stream( Log::LOG_ERR ) <<
    //"yield::ipc::SocketClient: error reading " << response->get_type_name() <<
    //  "/" << reinterpret_cast<uint64_t>( response ) <<
    //" from socket #" << reinterpret_cast<uint64_t>( &socket_ ) <<
    //", error='" << Exception( error_code ) <<
    //"', responding to " << request.get_type_name() <<
    //  "/" << reinterpret_cast<uint64_t>( &request ) <<
    //  " with ExceptionResponse.";
  }
}

void SocketClient::Connection::onWriteCompletion( size_t bytes_written, void* )
{ }

void SocketClient::Connection::onWriteError( uint32_t error_code, void* )
{
  if ( socket_client.get_error_log() != NULL )
  {
    //log->get_stream( Log::LOG_ERR ) <<
    //"yield::ipc::SocketClient: error writing " <<
    //request.get_type_name() <<
    //  "/" << reinterpret_cast<uint64_t>( &request ) <<
    //" to socket #" << reinterpret_cast<uint64_t>( &socket_ ) <<
    //", error='" << Exception( error_code ) <<
    //"', responding to " << request.get_type_name() <<
    //  "/" << reinterpret_cast<uint64_t>( &request ) <<
    //  " with ExceptionResponse.";
  }
}



// socket_peer.cpp
SocketPeer::SocketPeer
(
  Log* error_log,
  IOQueue& io_queue,
  Log* trace_log
)
  : error_log( Object::inc_ref( error_log ) ),
    io_queue( io_queue ),
    trace_log( Object::inc_ref( trace_log ) )
{ }

SocketPeer::~SocketPeer()
{
  Log::dec_ref( error_log );
  IOQueue::dec_ref( io_queue );
  Log::dec_ref( trace_log );
}

IOQueue& SocketPeer::createIOQueue()
{
#ifdef _WIN32
  return yield::platform::Win32AIOQueue::create();
#else
  return yield::platform::NBIOQueue::create();
#endif
}

SocketAddress& SocketPeer::createSocketAddress( const URI& absolute_uri )
{
  SocketAddress* sockaddr
    = SocketAddress::create
      (
        absolute_uri.get_host().c_str(),
        absolute_uri.get_port()
      );

  if ( sockaddr != NULL )
    return *sockaddr;
  else
    throw Exception();
}


// socket_server.cpp
SocketServer::SocketServer
(
  Log* error_log,
  IOQueue& io_queue,
  EventTarget& request_target,
  Log* trace_log
)
  : SocketPeer( error_log, io_queue, trace_log ),
    request_target( request_target )
{ }

SocketServer::~SocketServer()
{
  EventTarget::dec_ref( request_target );
}


SocketServer::Connection::Connection
(
  Socket& socket_,
  SocketServer& socket_server
)
: socket_( socket_ ),
  socket_server( socket_server )
{ }

SocketServer::Connection::~Connection()
{
  Socket::dec_ref( socket_ );
}



// ssl_context.cpp
#ifdef YIELD_IPC_HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#ifdef _WIN32
#pragma comment( lib, "libeay32.lib" )
#pragma comment( lib, "ssleay32.lib" )
#undef UUID
#endif
#endif

// Must be after the #undef UUID


#ifdef YIELD_IPC_HAVE_OPENSSL

namespace yield
{
  namespace ipc
  {
    class SSLException : public Exception
    {
    public:
      SSLException()
        : Exception( ERR_peek_error() )
      {
        SSL_load_error_strings();

        char error_message[256];
        ERR_error_string_n( ERR_peek_error(), error_message, 256 );
        set_error_message( error_message );
      }
    };
  };
};


static int pem_password_callback( char *buf, int size, int, void *userdata )
{
  const string* pem_password
    = static_cast<const string*>( userdata );
  if ( size > static_cast<int>( pem_password->size() ) )
    size = static_cast<int>( pem_password->size() );
  memcpy_s( buf, size, pem_password->c_str(), size );
  return size;
}


SSLContext::SSLContext( SSL_CTX* ctx )
  : ctx( ctx )
{ }

SSLContext&
SSLContext::create
(
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
  const
#endif
  SSL_METHOD* method
)
{
  return *new SSLContext( createSSL_CTX( method ) );
}

SSLContext&
SSLContext::create
(
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
  const
#endif
  SSL_METHOD* method,
#ifdef _WIN32
  const Path& _pem_certificate_file_path,
  const Path& _pem_private_key_file_path,
#else
  const Path& pem_certificate_file_path,
  const Path& pem_private_key_file_path,
#endif
  const string& pem_private_key_passphrase
)
{
  SSL_CTX* ctx = createSSL_CTX( method );

#ifdef _WIN32
  // Need to get a string on Windows, because SSL doesn't support wide paths
  string pem_certificate_file_path( _pem_certificate_file_path );
  string pem_private_key_file_path( _pem_private_key_file_path );
#endif

  if
  (
    SSL_CTX_use_certificate_file
    (
      ctx,
#ifdef _WIN32
      pem_certificate_file_path.c_str(),
#else
      pem_certificate_file_path,
#endif
      SSL_FILETYPE_PEM
    ) > 0
  )
  {
    if ( !pem_private_key_passphrase.empty() )
    {
      SSL_CTX_set_default_passwd_cb( ctx, pem_password_callback );
      SSL_CTX_set_default_passwd_cb_userdata
      (
        ctx,
        const_cast<string*>( &pem_private_key_passphrase )
      );
    }

    if
    (
      SSL_CTX_use_PrivateKey_file
      (
        ctx,
#ifdef _WIN32
        pem_private_key_file_path.c_str(),
#else
        pem_private_key_file_path,
#endif
        SSL_FILETYPE_PEM
      ) > 0
    )
      return *new SSLContext( ctx );
  }

  throw SSLException();
}

SSLContext&
SSLContext::create
(
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
  const
#endif
  SSL_METHOD* method,
  const string& pem_certificate_str,
  const string& pem_private_key_str,
  const string& pem_private_key_passphrase
)
{
  SSL_CTX* ctx = createSSL_CTX( method );

  BIO* pem_certificate_bio
    = BIO_new_mem_buf
      (
        reinterpret_cast<void*>
        (
          const_cast<char*>( pem_certificate_str.c_str() )
        ),
        static_cast<int>( pem_certificate_str.size() )
      );

  if ( pem_certificate_bio != NULL )
  {
    X509* cert
      = PEM_read_bio_X509
        (
          pem_certificate_bio,
          NULL,
          pem_password_callback,
          const_cast<string*>( &pem_private_key_passphrase )
        );

    if ( cert != NULL )
    {
      SSL_CTX_use_certificate( ctx, cert );

      BIO* pem_private_key_bio
        = BIO_new_mem_buf
          (
            reinterpret_cast<void*>
            (
              const_cast<char*>( pem_private_key_str.c_str() )
            ),
            static_cast<int>( pem_private_key_str.size() )
          );

      if ( pem_private_key_bio != NULL )
      {
        EVP_PKEY* pkey
          = PEM_read_bio_PrivateKey
            (
              pem_private_key_bio,
              NULL,
              pem_password_callback,
              const_cast<string*>( &pem_private_key_passphrase )
            );

        if ( pkey != NULL )
        {
          SSL_CTX_use_PrivateKey( ctx, pkey );

          BIO_free( pem_certificate_bio );
          BIO_free( pem_private_key_bio );

          return *new SSLContext( ctx );
        }

        BIO_free( pem_private_key_bio );
      }
    }

    BIO_free( pem_certificate_bio );
  }

  throw SSLException();
}

SSLContext&
SSLContext::create
(
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
  const
#endif
  SSL_METHOD* method,
#ifdef _WIN32
  const Path& _pkcs12_file_path,
#else
  const Path& pkcs12_file_path,
#endif
  const string& pkcs12_passphrase
)
{
  SSL_CTX* ctx = createSSL_CTX( method );

#ifdef _WIN32
  // See note in the PEM create above re: the rationale for this
  string pkcs12_file_path( _pkcs12_file_path );
#endif

#ifdef _WIN32
  BIO* bio = BIO_new_file( pkcs12_file_path.c_str(), "rb" );
#else
  BIO* bio = BIO_new_file( pkcs12_file_path, "rb" );
#endif
  if ( bio != NULL )
  {
    PKCS12* p12 = d2i_PKCS12_bio( bio, NULL );
    if ( p12 != NULL )
    {
      EVP_PKEY* pkey = NULL;
      X509* cert = NULL;
      STACK_OF( X509 )* ca = NULL;
      if ( PKCS12_parse( p12, pkcs12_passphrase.c_str(), &pkey, &cert, &ca ) )
      {
        if ( pkey != NULL && cert != NULL && ca != NULL )
        {
          SSL_CTX_use_certificate( ctx, cert );
          SSL_CTX_use_PrivateKey( ctx, pkey );

          X509_STORE* store = SSL_CTX_get_cert_store( ctx );
          for ( int i = 0; i < sk_X509_num( ca ); i++ )
          {
            X509* store_cert = sk_X509_value( ca, i );
            X509_STORE_add_cert( store, store_cert );
          }

          BIO_free( bio );

          return *new SSLContext( ctx );
        }
      }
    }

    BIO_free( bio );
  }

  throw SSLException();
}

#endif

SSLContext::~SSLContext()
{
#ifdef YIELD_IPC_HAVE_OPENSSL
  SSL_CTX_free( ctx );
#endif
}

#ifdef YIELD_IPC_HAVE_OPENSSL

SSL_CTX*
SSLContext::createSSL_CTX
(
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
  const
#endif
  SSL_METHOD* method
)
{
  SSL_library_init();
  OpenSSL_add_all_algorithms();

  SSL_CTX* ctx = SSL_CTX_new( method );
  if ( ctx != NULL )
  {
#ifdef SSL_OP_NO_TICKET
    SSL_CTX_set_options( ctx, SSL_OP_ALL|SSL_OP_NO_TICKET );
#else
    SSL_CTX_set_options( ctx, SSL_OP_ALL );
#endif
    SSL_CTX_set_verify( ctx, SSL_VERIFY_NONE, NULL );
    return ctx;
  }
  else
    throw SSLException();
}

#endif


// ssl_socket.cpp
#ifdef YIELD_IPC_HAVE_OPENSSL



SSLSocket::SSLSocket
(
  int domain,
  socket_t socket_,
  SSL* ssl,
  SSLContext& ssl_context
)
  : TCPSocket( domain, socket_ ),
    ssl( ssl ),
    ssl_context( ssl_context.inc_ref() )
{ }

SSLSocket::~SSLSocket()
{
  SSL_free( ssl );
  SSLContext::dec_ref( ssl_context );
}

TCPSocket* SSLSocket::accept()
{
  SSL_set_fd( ssl, *this );
  socket_t peer_socket = TCPSocket::_accept();
  if ( peer_socket != -1 )
  {
    SSL* peer_ssl = SSL_new( ssl_context );
    SSL_set_fd( peer_ssl, peer_socket );
    SSL_set_accept_state( peer_ssl );
    return new SSLSocket( get_domain(), peer_socket, peer_ssl, ssl_context );
  }
  else
    return NULL;
}

bool SSLSocket::associate( IOQueue& io_queue )
{
  if ( get_io_queue() == NULL )
  {
    switch ( io_queue.get_type_id() )
    {
      case yield::platform::BIOQueue::TYPE_ID:
      case yield::platform::NBIOQueue::TYPE_ID:
      {
        set_io_queue( io_queue );
        return true;
      }
      break;

      default: return false;
    }
  }
  else
    return false;
}

bool SSLSocket::connect( const SocketAddress& peername )
{
  if ( TCPSocket::connect( peername ) )
  {
    SSL_set_fd( ssl, *this );
    SSL_set_connect_state( ssl );
    return true;
  }
  else
    return false;
}

SSLSocket* SSLSocket::create( SSLContext& ssl_context )
{
  return create( DOMAIN_DEFAULT, ssl_context );
}

SSLSocket* SSLSocket::create( int domain, SSLContext& ssl_context )
{
  SSL* ssl = SSL_new( ssl_context );
  if ( ssl != NULL )
  {
    socket_t socket_ = TCPSocket::create( &domain );
    if ( socket_ != -1 )
      return new SSLSocket( domain, socket_, ssl, ssl_context );
    else
    {
      SSL_free( ssl );
      return NULL;
    }
  }
  else
    return NULL;
}

/*
void SSLSocket::info_callback( const SSL* ssl, int where, int ret )
{
  ostringstream info;

  int w = where & ~SSL_ST_MASK;
  if ( ( w & SSL_ST_CONNECT ) == SSL_ST_CONNECT ) info << "SSL_connect:";
  else if ( ( w & SSL_ST_ACCEPT ) == SSL_ST_ACCEPT ) info << "SSL_accept:";
  else info << "undefined:";

  if ( ( where & SSL_CB_LOOP ) == SSL_CB_LOOP )
    info << SSL_state_string_long( ssl );
  else if ( ( where & SSL_CB_ALERT ) == SSL_CB_ALERT )
  {
    if ( ( where & SSL_CB_READ ) == SSL_CB_READ )
      info << "read:";
    else
      info << "write:";
    info << "SSL3 alert" << SSL_alert_type_string_long( ret ) << ":" <<
            SSL_alert_desc_string_long( ret );
  }
  else if ( ( where & SSL_CB_EXIT ) == SSL_CB_EXIT )
  {
    if ( ret == 0 )
      info << "failed in " << SSL_state_string_long( ssl );
    else
      info << "error in " << SSL_state_string_long( ssl );
  }
  else
    return;

  reinterpret_cast<SSLSocket*>( SSL_get_app_data( const_cast<SSL*>( ssl ) ) )
    ->log->get_stream( Log::LOG_NOTICE ) << "SSLSocket: " << info.str();
}
*/

ssize_t SSLSocket::recv( void* buf, size_t buflen, int )
{
  return SSL_read( ssl, buf, static_cast<int>( buflen ) );
}

ssize_t SSLSocket::send( const void* buf, size_t buflen, int )
{
  return SSL_write( ssl, buf, static_cast<int>( buflen ) );
}

ssize_t SSLSocket::sendmsg( const struct iovec* iov, uint32_t iovlen, int )
{
  // Concatenate the buffers
  return OStream::writev( iov, iovlen );
}

bool SSLSocket::shutdown()
{
  if ( SSL_shutdown( ssl ) != -1 )
    return TCPSocket::shutdown();
  else
    return false;
}

bool SSLSocket::want_read() const
{
  return SSL_want_read( ssl ) == 1;
}

bool SSLSocket::want_write() const
{
  return SSL_want_write( ssl ) == 1;
}

#endif


// tcp_socket_server.cpp
TCPSocketServer::TCPSocketServer
(
  Log* error_log,
  IOQueue& io_queue,
  TCPSocket& listen_tcp_socket,
  EventTarget& request_target,
  Log* trace_log
) : SocketServer( error_log, io_queue, request_target, trace_log ),
    listen_tcp_socket( listen_tcp_socket )
{ }

TCPSocketServer::~TCPSocketServer()
{
  TCPSocket::dec_ref( listen_tcp_socket );
}

IOQueue& TCPSocketServer::createIOQueue( bool for_ssl )
{
  if ( for_ssl )
    return yield::platform::NBIOQueue::create();
  else
    return SocketServer::createIOQueue();
}

TCPSocket&
TCPSocketServer::createListenTCPSocket
(
  const URI& absolute_uri,
  IOQueue& io_queue,
  SSLContext* ssl_context,
  Log* trace_log
)
{
  SocketAddress& sockname = createSocketAddress( absolute_uri );

  TCPSocket* listen_tcp_socket = NULL;
#ifdef YIELD_IPC_HAVE_OPENSSL
  if
  (
    absolute_uri.get_scheme()[absolute_uri.get_scheme().size()-1] == 's'
    &&
    ssl_context != NULL
  )
  {
    listen_tcp_socket = SSLSocket::create( *ssl_context );
    SSLContext::dec_ref( *ssl_context );
  }
  else
#endif
  if ( trace_log != NULL )
    listen_tcp_socket = TracingTCPSocket::create( *trace_log );
  else
    listen_tcp_socket = TCPSocket::create();

  if
  (
    listen_tcp_socket != NULL
    &&
    listen_tcp_socket->associate( io_queue )
    &&
    listen_tcp_socket->bind( sockname )
    &&
    listen_tcp_socket->listen()
    &&
    listen_tcp_socket->setsockopt( Socket::OPTION_SO_KEEPALIVE, true )
    &&
    listen_tcp_socket->setsockopt( Socket::OPTION_SO_LINGER, true )
    &&
    listen_tcp_socket->setsockopt( TCPSocket::OPTION_TCP_NODELAY, true )
  )
  {
    SocketAddress::dec_ref( sockname );
    return *listen_tcp_socket;
  }
  else
  {
    TCPSocket::dec_ref( listen_tcp_socket );
    SocketAddress::dec_ref( sockname );
    throw Exception();
  }
}

void
TCPSocketServer::onAcceptCompletion
(
  TCPSocket& accepted_tcp_socket,
  void* context,
  Buffer* recv_buffer
)
{
  if ( !accepted_tcp_socket.associate( get_io_queue() ) )
    DebugBreak();
}

void TCPSocketServer::onAcceptError( uint32_t error_code, void* )
{
  DebugBreak();
  // listen_tcp_socket->aio_accept( *this );
}


TCPSocketServer::Connection::Connection
(
  TCPSocket& accepted_tcp_socket,
  TCPSocketServer& tcp_socket_server
)
  : SocketServer::Connection( accepted_tcp_socket, tcp_socket_server )
{ }

void TCPSocketServer::Connection::onReadCompletion( Buffer& buffer, void* )
{ }

void TCPSocketServer::Connection::onReadError( uint32_t error_code, void* )
{
  get_socket().shutdown();
  get_socket().close();
  dec_ref( *this );
}

void
TCPSocketServer::Connection::onWriteCompletion
(
  size_t bytes_written,
  void*
)
{ }

void TCPSocketServer::Connection::onWriteError( uint32_t error_code, void* )
{
  get_socket().shutdown();
  get_socket().close();
  dec_ref( *this );
}


// tracing_tcp_socket.cpp
#ifdef _WIN32
#pragma warning( 4 : 4365 )
#endif


TracingTCPSocket::TracingTCPSocket( int domain, Log& log, socket_t socket_ )
  : TCPSocket( domain, socket_ ),
    log( log.inc_ref() )
{ }

TracingTCPSocket::~TracingTCPSocket()
{
  Log::dec_ref( log );
}

TCPSocket* TracingTCPSocket::accept()
{
  socket_t socket_ = TCPSocket::_accept();
  if ( socket_ != INVALID_SOCKET )
    return new TracingTCPSocket( get_domain(), log, socket_ );
  else
    return NULL;
}

TracingTCPSocket* TracingTCPSocket::create( Log& log )
{
  return create( DOMAIN_DEFAULT, log );
}

TracingTCPSocket* TracingTCPSocket::create( int domain, Log& log )
{
  socket_t socket_ = TCPSocket::create( &domain );
  if ( socket_ != INVALID_SOCKET )
    return new TracingTCPSocket( domain, log, socket_ );
  else
    return NULL;
}

bool TracingTCPSocket::connect( const SocketAddress& peername )
{
  string to_hostname;
  if ( peername.getnameinfo( to_hostname ) )
  {
    log.get_stream( Log::LOG_INFO ) <<
      "yield::ipc::TracingTCPSocket: connecting socket #" <<
      static_cast<uint64_t>( *this ) <<
      " to " << to_hostname << ".";
  }

  return TCPSocket::connect( peername );
}

ssize_t TracingTCPSocket::recv( void* buf, size_t buflen, int flags )
{
  log.get_stream( Log::LOG_INFO ) <<
    "yield::ipc::TracingTCPSocket: trying to read " << buflen <<
    " bytes from socket #" << static_cast<uint64_t>( *this ) << ".";

  ssize_t recv_ret = TCPSocket::recv( buf, buflen, flags );

  if ( recv_ret > 0 )
  {
    log.get_stream( Log::LOG_INFO ) <<
      "yield::ipc::TracingTCPSocket: read " << recv_ret <<
      " bytes from socket #" << static_cast<uint64_t>( *this ) << ".";
    log.write( buf, static_cast<size_t>( recv_ret ), Log::LOG_DEBUG );
    log.write( "\n", Log::LOG_DEBUG );
  }
  else if
  (
    recv_ret == 0
    ||
    ( !TCPSocket::want_read() && !TCPSocket::want_write() )
  )
    log.get_stream( Log::LOG_INFO ) <<
      "yield::ipc::TracingTCPSocket: lost connection while trying to read " <<
      "socket #" << static_cast<uint64_t>( *this ) << ".";

  return recv_ret;
}

ssize_t TracingTCPSocket::send( const void* buf, size_t buflen, int flags )
{
  ssize_t send_ret = TCPSocket::send( buf, buflen, flags );
  if ( send_ret >= 0 )
  {
    log.get_stream( Log::LOG_INFO ) <<
      "yield::ipc::TracingTCPSocket: wrote " << send_ret <<
      " bytes to socket #" << static_cast<uint64_t>( *this ) << ".";
    log.write( buf, buflen, Log::LOG_DEBUG );
  }
  else if ( !TCPSocket::want_read() && !TCPSocket::want_write() )
  {
    log.get_stream( Log::LOG_DEBUG ) <<
      "yield::ipc::TracingTCPSocket: lost connection while trying to write to " <<
      "socket #" << static_cast<uint64_t>( *this ) << ".";
  }

  return send_ret;
}

ssize_t TracingTCPSocket::sendmsg
(
  const struct iovec* iov,
  uint32_t iovlen,
  int flags
)
{
  size_t buffers_len = 0;
  for ( uint32_t iov_i = 0; iov_i < iovlen; iov_i++ )
    buffers_len += iov[iov_i].iov_len;

  log.get_stream( Log::LOG_INFO ) <<
    "yield::ipc::TracingTCPSocket: trying to write " << buffers_len <<
    " bytes to socket #" << static_cast<uint64_t>( *this ) << ".";

  ssize_t sendmsg_ret = TCPSocket::sendmsg( iov, iovlen, flags );

  if ( sendmsg_ret >= 0 )
  {
    size_t temp_sendmsg_ret = static_cast<size_t>( sendmsg_ret );
    log.get_stream( Log::LOG_INFO ) <<
      "yield::ipc::TracingTCPSocket: wrote " << sendmsg_ret <<
      " bytes to socket #" << static_cast<uint64_t>( *this ) << ".";

    for ( uint32_t iov_i = 0; iov_i < iovlen; iov_i++ )
    {
      if ( iov[iov_i].iov_len <= temp_sendmsg_ret )
      {
        log.write( iov[iov_i].iov_base, iov[iov_i].iov_len, Log::LOG_DEBUG );
        temp_sendmsg_ret -= iov[iov_i].iov_len;
      }
      else
      {
        log.write( iov[iov_i].iov_base, temp_sendmsg_ret, Log::LOG_DEBUG );
        break;
      }
    }

    log.write( "\n", Log::LOG_DEBUG );
  }
  else if ( !TCPSocket::want_read() && !TCPSocket::want_write() )
  {
    log.get_stream( Log::LOG_DEBUG ) <<
      "yield::ipc::TracingTCPSocket: lost connection while trying to write to " <<
      "socket #" << static_cast<uint64_t>( *this ) << ".";
  }

  return sendmsg_ret;
}

bool TracingTCPSocket::want_connect() const
{
  bool want_connect_ret = TCPSocket::want_connect();

  if ( want_connect_ret )
  {
    log.get_stream( Log::LOG_INFO ) <<
      "yield::ipc::TracingTCPSocket: would block on connect on socket #" <<
      static_cast<uint64_t>( *this ) << ".";
  }

  return want_connect_ret;
}

bool TracingTCPSocket::want_read() const
{
  bool want_recv_ret = TCPSocket::want_read();

  if ( want_recv_ret )
  {
    log.get_stream( Log::LOG_INFO ) <<
      "yield::ipc::TracingTCPSocket: would block on read on socket #" <<
      static_cast<uint64_t>( *this ) << ".";
  }

  return want_recv_ret;
}

bool TracingTCPSocket::want_write() const
{
  bool want_write_ret = TCPSocket::want_write();

  if ( want_write_ret )
  {
    log.get_stream( Log::LOG_INFO ) <<
      "yield::ipc::TracingTCPSocket: would block on write on socket #" <<
      static_cast<uint64_t>( *this ) << ".";
  }

  return want_write_ret;
}


// udp_socket_server.cpp
UDPSocketServer::UDPSocketServer
(
  Log* error_log,
  IOQueue& io_queue,
  EventTarget& request_target,
  Log* trace_log,
  UDPSocket& udp_socket
) : SocketServer( error_log, io_queue, request_target, trace_log ),
    udp_socket( udp_socket )
{ }

UDPSocketServer::~UDPSocketServer()
{
  UDPSocket::dec_ref( udp_socket );
}

UDPSocket&
UDPSocketServer::createUDPSocket
(
  const URI& absolute_uri,
  IOQueue& io_queue,
  Log* trace_log
)
{
  SocketAddress& sockname = createSocketAddress( absolute_uri );

  UDPSocket* udp_socket = UDPSocket::create();
  if
  (
    udp_socket != NULL
    &&
    udp_socket->associate( io_queue )
    &&
    udp_socket->bind( sockname )
  )
  {
    SocketAddress::dec_ref( sockname );
    return *udp_socket;
  }
  else
  {
    SocketAddress::dec_ref( sockname );
    throw Exception();
  }
}


UDPSocketServer::Connection::Connection
(
  SocketAddress& peername,
  UDPSocket& udp_socket,
  UDPSocketServer& udp_socket_server
)
: SocketServer::Connection( udp_socket, udp_socket_server ),
  peername( peername )
{ }

UDPSocketServer::Connection::~Connection()
{
  SocketAddress::dec_ref( peername );
}

UDPSocket& UDPSocketServer::Connection::get_udp_socket() const
{
  return static_cast<UDPSocket&>( get_socket() );
}


// uri.cpp
extern "C"
{
};


URI::URI()
{ }

URI::URI( const char* uri )
{
  init( uri, strnlen( uri, UINT16_MAX ) );
}

URI::URI( const string& uri )
{
  init( uri.c_str(), uri.size() );
}

URI::URI( const char* uri, size_t uri_len )
{
  init( uri, uri_len );
}

URI::URI( const char* scheme, const char* host, uint16_t port )
  : scheme( scheme ), host( host ), port( port ), resource( "/" )
{ }

URI::URI
(
  const char* scheme,
  const char* host,
  uint16_t port,
  const char* resource
)
  : scheme( scheme ), host( host ), port( port ), resource( resource )
{ }

URI::URI( const URI& other )
: scheme( other.scheme ), user( other.user ), password( other.password ),
  host( other.host ), port( other.port ), resource( other.resource )
{ }

URI::URI( UriUriStructA& parsed_uri )
{
  init( parsed_uri );
}

void URI::init( const char* uri, size_t uri_len )
{
  UriParserStateA parser_state;
  UriUriA parsed_uri;
  parser_state.uri = &parsed_uri;
  if ( uriParseUriExA( &parser_state, uri, uri + uri_len ) == URI_SUCCESS )
  {
    init( parsed_uri );
    uriFreeUriMembersA( &parsed_uri );
  }
  else
  {
    uriFreeUriMembersA( &parsed_uri );
    throw Exception( "invalid URI" );
  }
}

void URI::init( UriUriA& parsed_uri )
{
  scheme.assign
  (
    parsed_uri.scheme.first,
    static_cast<size_t>
    (
      parsed_uri.scheme.afterLast - parsed_uri.scheme.first
    )
  );

  host.assign
  (
    parsed_uri.hostText.first,
    static_cast<size_t>
    (
      parsed_uri.hostText.afterLast - parsed_uri.hostText.first
    )
  );

  if ( parsed_uri.portText.first != NULL )
  {
    port
      = static_cast<uint16_t>
        (
          strtol
          (
            parsed_uri.portText.first,
            NULL,
            0
          )
        );
  }
  else
    port = 0;

  if ( parsed_uri.userInfo.first != NULL )
  {
    const char* userInfo_p = parsed_uri.userInfo.first;
    while ( userInfo_p < parsed_uri.userInfo.afterLast )
    {
      if ( *userInfo_p == ':' )
      {
        user.assign
        (
          parsed_uri.userInfo.first,
          static_cast<size_t>
          (
            userInfo_p - parsed_uri.userInfo.first
          )
        );

        password.assign
        (
          userInfo_p + 1,
          static_cast<size_t>
          (
            parsed_uri.userInfo.afterLast - userInfo_p - 1
          )
        );

        break;
      }
      userInfo_p++;
    }

    if ( user.empty() ) // No colon found => no password, just the user
    {
      user.assign
      (
        parsed_uri.userInfo.first,
        static_cast<size_t>
        (
          parsed_uri.userInfo.afterLast - parsed_uri.userInfo.first
        )
      );
    }
  }

  if ( parsed_uri.pathHead != NULL )
  {
    UriPathSegmentA* path_segment = parsed_uri.pathHead;
    do
    {
      resource.append( "/" );
      resource.append
      (
        path_segment->text.first,
        static_cast<size_t>
        (
          path_segment->text.afterLast - path_segment->text.first
        )
      );

      path_segment = path_segment->next;
    }
    while ( path_segment != NULL );

    if ( parsed_uri.query.first != NULL )
    {
      UriQueryListA* query_list;
      uriDissectQueryMallocA
      (
        &query_list,
        NULL,
        parsed_uri.query.first,
        parsed_uri.query.afterLast
      );

      UriQueryListA* query_list_p = query_list;
      while ( query_list_p != NULL )
      {
        query.insert( make_pair( query_list_p->key, query_list_p->value ) );
        query_list_p = query_list_p->next;
      }

      uriFreeQueryListA( query_list );
    }
  }
  else
    resource = "/";
}

string
URI::get_query_value
(
  const string& key,
  const char* default_query_value
) const
{
  multimap<string, string>::const_iterator query_value_i
    = query.find( key );

  if ( query_value_i != query.end() )
    return query_value_i->second;
  else
    return default_query_value;
}

multimap<string, string>::const_iterator
URI::get_query_values
(
  const string& key
) const
{
  return query.find( key );
}

URI& URI::operator=( const URI& other )
{
  scheme = other.scheme;
  user = other.user;
  password = other.password;
  host = other.host;
  port = other.port;
  resource = other.resource;
  return *this;
}

URI::operator std::string() const
{
  ostringstream uri_oss;
  uri_oss << scheme << "://";
  if ( !user.empty() )
    uri_oss << user;
  if ( !password.empty() )
    uri_oss << ":" << password;
  uri_oss << host;
  if ( port != 0 )
    uri_oss << ":" << port;
  uri_oss << resource;
  if ( !query.empty() )
    DebugBreak();
  return uri_oss.str();
}

URI* URI::parse( const char* uri )
{
  return parse( uri, strnlen( uri, UINT16_MAX ) );
}

URI* URI::parse( const string& uri )
{
  return parse( uri.c_str(), uri.size() );
}

URI* URI::parse( const char* uri, size_t uri_len )
{
  UriParserStateA parser_state;
  UriUriA parsed_uri;
  parser_state.uri = &parsed_uri;
  if ( uriParseUriExA( &parser_state, uri, uri + uri_len ) == URI_SUCCESS )
  {
    URI* uri = new URI( parsed_uri );
    uriFreeUriMembersA( &parsed_uri );
    return uri;
  }
  else
  {
    uriFreeUriMembersA( &parsed_uri );
    return NULL;
  }
}

void URI::set_password( const string& password )
{
  this->password = password;
}

void URI::set_resource( const string& resource )
{
  this->resource = resource;
}


// uuid.cpp
#if defined(_WIN32)
namespace win32_Rpc_h
{
  #define RPC_NO_WINDOWS_H
  #include <Rpc.h>
};
#pragma comment( lib, "Rpcrt4.lib" )
#elif defined(YIELD_IPC_HAVE_LINUX_LIBUUID)
#include <uuid/uuid.h>
#elif defined(__sun)
#include <uuid/uuid.h>
#elif defined(YIELD_IPC_HAVE_OPENSSL)
#include <openssl/sha.h>
#endif

#ifndef _WIN32
#include <cstring>
#endif


UUID::UUID()
{
#if defined(_WIN32)
  win32_uuid = new win32_Rpc_h::UUID;
  win32_Rpc_h::UuidCreate( static_cast<win32_Rpc_h::UUID*>( win32_uuid ) );
#elif defined(YIELD_IPC_HAVE_LINUX_LIBUUID)
  linux_libuuid_uuid = new uuid_t;
  uuid_generate( *static_cast<uuid_t*>( linux_libuuid_uuid ) );
#elif defined(__sun)
  sun_uuid = new uuid_t;
  uuid_generate( *static_cast<uuid_t*>( sun_uuid ) );
#else
  strncpy( generic_uuid, Socket::getfqdn().c_str(), 256 );
#ifdef YIELD_IPC_HAVE_OPENSSL
  SHA_CTX ctx; SHA1_Init( &ctx );
  SHA1_Update( &ctx, generic_uuid, strlen( generic_uuid ) );
  memset( generic_uuid, 0, sizeof( generic_uuid ) );
  unsigned char sha1sum[SHA_DIGEST_LENGTH]; SHA1_Final( sha1sum, &ctx );
  static char hex_array[]
    =
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
    };
  unsigned int sha1sum_i = 0, generic_uuid_i = 0;
  for ( ; sha1sum_i < SHA_DIGEST_LENGTH; sha1sum_i++, generic_uuid_i += 2 )
  {
   generic_uuid[generic_uuid_i]
     = hex_array[( sha1sum[sha1sum_i] & 0xf0 ) >> 4];

   generic_uuid[generic_uuid_i+1]
     = hex_array[( sha1sum[sha1sum_i] & 0x0f )];
  }
  generic_uuid[generic_uuid_i] = 0;
#endif
#endif
}

UUID::UUID( const string& from_string )
{
#if defined(_WIN32)
  win32_uuid = new win32_Rpc_h::UUID;
  win32_Rpc_h::UuidFromStringA
  (
    reinterpret_cast<win32_Rpc_h::RPC_CSTR>
    (
      const_cast<char*>( from_string.c_str() )
    ),
    static_cast<win32_Rpc_h::UUID*>( win32_uuid )
  );
#elif defined(YIELD_IPC_HAVE_LINUX_LIBUUID)
  uuid_parse
  (
    from_string.c_str(),
    *static_cast<uuid_t*>( linux_libuuid_uuid )
  );
#elif defined(__sun)
  uuid_parse
  (
    const_cast<char*>( from_string.c_str() ),
    *static_cast<uuid_t*>( sun_uuid )
  );
#else
  strncpy( generic_uuid, from_string.c_str(), 256 );
#endif
}

UUID::~UUID()
{
#if defined(_WIN32)
  delete static_cast<win32_Rpc_h::UUID*>( win32_uuid );
#elif defined(YIELD_IPC_HAVE_LINUX_LIBUUID)
  delete static_cast<uuid_t*>( linux_libuuid_uuid );
#elif defined(__sun)
  delete static_cast<uuid_t*>( sun_uuid );
#endif
}

bool UUID::operator==( const UUID& other ) const
{
#ifdef _WIN32
  return memcmp
         (
           win32_uuid,
           other.win32_uuid,
           sizeof( win32_Rpc_h::UUID )
         ) == 0;
#elif defined(YIELD_IPC_HAVE_LINUX_LIBUUID)
  return uuid_compare
         (
           *static_cast<uuid_t*>( linux_libuuid_uuid ),
           *static_cast<uuid_t*>( other.linux_libuuid_uuid )
         ) == 0;
#elif defined(__sun)
  return uuid_compare
         (
           *static_cast<uuid_t*>( sun_uuid ),
           *static_cast<uuid_t*>( other.sun_uuid )
         ) == 0;
#else
  return strncmp( generic_uuid, other.generic_uuid, 256 );
#endif
}

UUID::operator string() const
{
#if defined(_WIN32)
  win32_Rpc_h::RPC_CSTR temp_to_string;
  win32_Rpc_h::UuidToStringA
  (
    static_cast<win32_Rpc_h::UUID*>( win32_uuid ),
    &temp_to_string
  );
  string to_string( reinterpret_cast<char*>( temp_to_string ) );
  win32_Rpc_h::RpcStringFreeA( &temp_to_string );
  return to_string;
#elif defined(YIELD_IPC_HAVE_LINUX_LIBUUID)
  char out[37];
  uuid_unparse( *static_cast<uuid_t*>( linux_libuuid_uuid ), out );
  return out;
#elif defined(__sun)
#else
  return generic_uuid;
#endif
}

