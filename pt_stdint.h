/* pt_stdint.h */
#ifndef PT_STDINT__H
#define PT_STDINT__H
# ifdef _MSC_VER  /* Microsoft Visual C++ */
typedef signed char             int8_t;
typedef short int               int16_t;
typedef int                     int32_t;
typedef __int64                 int64_t;
 
typedef unsigned char             uint8_t;
typedef unsigned short int        uint16_t;
typedef unsigned int              uint32_t;
/* no uint64_t */

#define INT32_MAX _I32_MAX

#  define vsnprintf _vsnprintf
#  define snprintf _snprintf
# else
#  include <stdint.h>
# endif

#endif

