
#ifndef _fj_json_inttypes_h_
#define _fj_json_inttypes_h_

#include "config.h"

#ifdef FJSON_HAVE_INTTYPES_H
/* inttypes.h includes stdint.h */
#include <inttypes.h>

#else
#include <stdint.h>

#ifndef PRId64
#  define PRId64 "I64d"
#endif
#ifndef SCNd64
#  define SCNd64 "I64d"
#endif

#endif

#endif
