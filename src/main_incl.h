#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#define LOG 1
#define STR(x) #x
#define MACRO2STR(y) STR(y)

#include <string>

#include "file_io.h"
#include "multi_term_adders.h"

#ifdef FP32
  typedef fp32 T;
#endif
#ifdef BF16
  typedef bf16 T;
#endif
#ifdef FP8_E4M3
  typedef fp8_e4m3 T;
#endif
#ifdef FP8_E5M2
  typedef fp8_e5m2 T;
#endif

#ifdef FP8_E6M1
  typedef fp8_e6m1 T;
#endif


#endif
