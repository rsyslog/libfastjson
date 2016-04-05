
#ifndef _fj_json_inttypes_h_
#define _fj_json_inttypes_h_

#include "json_config.h"

#ifdef FJSON_HAVE_INTTYPES_H
/* inttypes.h includes stdint.h */
#include <inttypes.h>

#else
#include <stdint.h>

#define PRId64 "I64d"
#define SCNd64 "I64d"

#endif

#endif
