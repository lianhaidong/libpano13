/*
 * The so called TIFF types conflict with definitions from inttypes.h 
 * included from sys/types.h on AIX (at least using VisualAge compiler). 
 * We try to work around this by detecting this case.  Defining 
 * _TIFF_DATA_TYPEDEFS_ short circuits the later definitions in tiff.h, and
 * we will in the holes not provided for by inttypes.h. 
 *
 * See http://bugzilla.remotesensing.org/show_bug.cgi?id=39
 */

#ifndef PANOTYPES_H
#define PANOTYPES_H

// First make sure that we have the int8_t, int16_t (32, and 64) and uint8_t equivalents
#include "pt_stdint.h"


/*
 * Intrinsic data types required by the file format:
 *
 * 8-bit quantities	int8/uint8
 * 16-bit quantities	int16/uint16
 * 32-bit quantities	int32/uint32
 * strings		unsigned char*
 */
typedef uint8_t  pt_uint8;
typedef uint16_t pt_uint16;
typedef uint32_t pt_uint32;

typedef int8_t  pt_int8;
typedef int16_t pt_int16;
typedef int32_t pt_int32;

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

/* Simple define to reduce warnings in printfs */
#if  __WORDSIZE == 64 /* 64 bit system */
   #define FMT_INT32 "%ld"
#else
   #define FMT_INT32 "%d"
#endif

#endif

