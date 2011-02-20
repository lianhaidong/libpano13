/*
 * The so called TIFF types conflict with definitions from inttypes.h 
 * included from sys/types.h on AIX (at least using VisualAge compiler). 
 * We try to work around this by detecting this case.  Defining 
 * _TIFF_DATA_TYPEDEFS_ short circuits the later definitions in tiff.h, and
 * we will in the holes not provided for by inttypes.h. 
 *
 * See http://bugzilla.remotesensing.org/show_bug.cgi?id=39
 */
#if defined(_H_INTTYPES) && defined(_ALL_SOURCE) && defined(USING_VISUALAGE)

#define _PANO_DATA_TYPEDEFS_
typedef unsigned char pt_uint8;
typedef unsigned short pt_uint16;
typedef unsigned int pt_uint32;

#endif

/*
 * Intrinsic data types required by the file format:
 *
 * 8-bit quantities	int8/uint8
 * 16-bit quantities	int16/uint16
 * 32-bit quantities	int32/uint32
 * strings		unsigned char*
 */
#ifndef _PANO_DATA_TYPEDEFS_
#define _PANO_DATA_TYPEDEFS_

#ifdef __STDC__
typedef	signed char pt_int8;	/* NB: non-ANSI compilers may not grok */
#else
typedef	char pt_int8;
#endif
typedef	unsigned char pt_uint8;
typedef	short pt_int16;
typedef	unsigned short pt_uint16;	/* sizeof (uint16) must == 2 */
#if defined(__s390x__) || defined(__ia64__) || defined(__alpha) || defined(__x86_64__) || (defined(_MIPS_SZLONG) && _MIPS_SZLONG == 64) || defined(__LP64__) || defined(_LP64)
typedef	int pt_int32;
typedef	unsigned int pt_uint32;	/* sizeof (uint32) must == 4 */
#define FMT_INT32 "%d"
#else
typedef	long pt_int32;
typedef	unsigned long pt_uint32;	/* sizeof (uint32) must == 4 */
#define FMT_INT32 "%ld"
#endif
#endif /* _PANO_DATA_TYPEDEFS_ */

/* The macro PT_UNUSED indicates that a function, function argument or
 * variable may potentially be unused.
 * Examples:
 *   1) static int PT_UNUSED unused_function (char arg);
 *   2) int foo (char unused_argument PT_UNUSED);
 *   3) int unused_variable PT_UNUSED;
 */

#ifdef __GNUC__
  #define PT_UNUSED __attribute__ ((__unused__))
#else
  #define PT_UNUSED
#endif
