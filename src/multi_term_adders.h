#ifndef __MULTI_TERM_ADDERS_H__
#define __MULTI_TERM_ADDERS_H__

#include <ac_fixed.h>
#include <ac_std_float.h>

#include "max_lzc.h"

#define CLOG2(x) ac::log2_ceil<x>::val
#define BITS(x)  ac::nbits<x>::val

typedef ac_int<1, false> bit_t;

// MULTI-TERM ADDER CLASS
template<int M, int E, int OFP_SAT=1>
class ffloat {
public:
  typedef ac_int<M,false> man_t;
  typedef ac_int<E,false> exp_t;
  typedef ac_int<1,false> sgn_t;
  
  man_t mantissa;
  exp_t exponent;
  sgn_t sign;
   
  static const int width = M + E + 1;
  static const int man_width = M;
  static const int exp_width = E;
  static const int e_bias = (1 << (E-1)) - 1;
  static const int frac_bits = M + 1;

  enum RND_ENUM {EVEN, ODD, INF};

public:
  /** DIFFERENT RADIX OPERATORS
    There are 6 types of operators:
      - radix-N (for arbitrary number of N-inputs)
      - radix-N-intermediate

      Opeartors withought the `-intermediate` suffix are for the
      last level of the binary operator tree. Other operators are
      used in the rest of the levels down to level 0 (root) where 
      the final sum and max exponent is computed.
  **/

  template <int N, int outbits>
  void radix_reduce(
    ffloat<M, E>       A[N],
    ac_int<outbits, true> &acc_o,
    ac_int<E+2, true>     &max_o
  ){
    ac_int<E+2, true > mul_exp_result [N];
    ac_int<M+1, false> mul_prod       [N];
    ac_int<1  , false> mulSign        [N];
 
    init_start_nodes<N, ffloat<M,E>>(
      A,
      mul_exp_result,
      mul_prod,
      mulSign
    );
    
    ac_int<E+2,true> maxExp = max<N>(mul_exp_result); // find max exponent

    ac_int<M+1,false> shifted_prod[N];
    ac_int<E+2,true>      diffE[N];
    #pragma hls_unroll
    ALLIGN: for (int i=0; i<N; i++) {
      diffE[i] = maxExp-mul_exp_result[i];
      shifted_prod[i] = mul_prod[i] >> diffE[i];
    }

    ac_int<M+1+1  ,true> add_op[N];
    ac_int<outbits,true> acc_s=0;

    #pragma hls_unroll
    TWOs_COMPL_ADD: for (int i=0; i<N; i++) {
      if (mulSign[i]) {
        add_op[i] = -shifted_prod[i];
      } else {
        add_op[i] = shifted_prod[i];
      }   
      acc_s += add_op[i];
    }

    max_o = maxExp;
    acc_o = acc_s;
  }

  template<int N, int in_bits, int out_bits>
  void radix_reduce_intermediate(
    ac_int<in_bits, true>   acc_i[N],
    ac_int<E+2    , true>   max_i[N],
    ac_int<out_bits, true> &acc_o   ,
    ac_int<E+2, true>      &max_o
  ){
    ac_int<E+2,true> maxExp = max<N>(max_i); // find max exponent

    ac_int<in_bits,true> shifted_prod[N];
    ac_int<E+2,true>      diffE[N];

    #pragma hls_unroll
    ALLIGN: for (int i=0; i<N; i++) {
      diffE[i]        = maxExp - max_i[i];
      shifted_prod[i] = acc_i[i] >> diffE[i];
    }

    ac_int<out_bits,true> acc_s=0;

    #pragma hls_unroll
    TWOs_COMPL_ADD: for (int i=0; i<N; i++) {
      acc_s += shifted_prod[i];
    }

    max_o = maxExp;
    acc_o = acc_s;
  }

  /** MULTI-TERM ONLINE ADDERS
    There are 3 types of multi-term adders for 3 different 
    input_sizes of 16-, 32- and 64-terms.
    
    Each adder has the prefix `fpaReduce_online`, then 
    follows the concatenated radix operators e.g. 2-2-2 is 222, 
    and 32-2 or 2-16-2 is 322, 2162 respectively.
  **/

  // IN 64
  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_322(ffloat<M,E> A[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;
    
    ac_int<M+1+1+5, true> acc_ar[2];
    ac_int<E+2    , true> max_ar[2];
    
    ffloat<M,E> A_0[32];
    ffloat<M,E> A_1[32];

    SET_A0:
    #pragma hls_unroll
    for(int i = 0; i < 32; i++) {
      A_0[i] = A[i];
    }
    SET_A1:
    #pragma hls_unroll
    for(int i = 0; i < 32; i++) {
      A_1[i] = A[32 + i];
    }

    radix_reduce<32>(A_0, acc_ar[0], max_ar[0]);
    radix_reduce<32>(A_1, acc_ar[1], max_ar[1]);

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_232(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[32];
    ac_int<E+2    , true> max_ar[32];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      radix_reduce<2>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<32>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_2162(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[2][16];
    ac_int<E+2    , true> max_ar[2][16];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 16; j++) {
        radix_reduce<2>(A[i*16+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+5, true> acc_ar_1[2];
    ac_int<E+2    , true> max_ar_1[2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<16>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_88(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+3, true> acc_ar[8];
    ac_int<E+2    , true> max_ar[8];
    
    ffloat<M,E> A[8][8];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        A[i][j] = A_i[i*8+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      radix_reduce<8>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<8>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_164(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+4, true> acc_ar[4];
    ac_int<E+2    , true> max_ar[4];
    
    ffloat<M,E> A[4][16];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 16; j++) {
        A[i][j] = A_i[i*16+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce<16>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_416(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[16];
    ac_int<E+2    , true> max_ar[16];
    
    ffloat<M,E> A[16][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      radix_reduce<4>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<16>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_284(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[4][8];
    ac_int<E+2    , true> max_ar[4][8];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        radix_reduce<2>(A[i*8+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_1[4];
    ac_int<E+2    , true> max_ar_1[4];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce_intermediate<8>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

    template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_248(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[8][4];
    ac_int<E+2    , true> max_ar[8][4];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce<2>(A[i*4+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[8];
    ac_int<E+2    , true> max_ar_1[8];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      radix_reduce_intermediate<4>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<8>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_482(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[2][8];
    ac_int<E+2    , true> max_ar[2][8];
    
    ffloat<M,E> A[16][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        radix_reduce<4>(A[i*8+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+5, true> acc_ar_1[2];
    ac_int<E+2    , true> max_ar_1[2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<8>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_428(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[8][2];
    ac_int<E+2    , true> max_ar[8][2];
    
    ffloat<M,E> A[16][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<4>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[8];
    ac_int<E+2    , true> max_ar_1[8];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      radix_reduce_intermediate<2>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<8>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_842(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+3, true> acc_ar[2][4];
    ac_int<E+2    , true> max_ar[2][4];
    
    ffloat<M,E> A[8][8];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        A[i][j] = A_i[i*8+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce<8>(A[i*4+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+5, true> acc_ar_1[2];
    ac_int<E+2    , true> max_ar_1[2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<4>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_824(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+3, true> acc_ar[4][2];
    ac_int<E+2    , true> max_ar[4][2];
    
    ffloat<M,E> A[8][8];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        A[i][j] = A_i[i*8+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<8>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_1[4];
    ac_int<E+2    , true> max_ar_1[4];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce_intermediate<2>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_444(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[4][4];
    ac_int<E+2    , true> max_ar[4][4];
    
    ffloat<M,E> A[16][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce<4>(A[i*4+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_1[4];
    ac_int<E+2    , true> max_ar_1[4];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce_intermediate<4>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_2228(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[16][2];
    ac_int<E+2    , true> max_ar[16][2];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[8][2];
    ac_int<E+2    , true> max_ar_1[8][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_2[8];
    ac_int<E+2    , true> max_ar_2[8];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      radix_reduce_intermediate<2>(acc_ar_1[i], max_ar_1[i], acc_ar_2[i], max_ar_2[i]);
    }
  
    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<8>(acc_ar_2, max_ar_2, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_2822(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[4][8];
    ac_int<E+2    , true> max_ar[4][8];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        radix_reduce<2>(A[i*8+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_1[2][2];
    ac_int<E+2    , true> max_ar_1[2][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<8>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+5, true> acc_ar_2[2];
    ac_int<E+2    , true> max_ar_2[2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar_1[i], max_ar_1[i], acc_ar_2[i], max_ar_2[i]);
    }
  
    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_2, max_ar_2, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_22224(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[16][2];
    ac_int<E+2    , true> max_ar[16][2];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[8][2];
    ac_int<E+2    , true> max_ar_1[8][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_2[4][2];
    ac_int<E+2    , true> max_ar_2[4][2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar_1[i*2+j], max_ar_1[i*2+j], acc_ar_2[i][j], max_ar_2[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_3[4];
    ac_int<E+2    , true> max_ar_3[4];
  
    ROW_4:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce_intermediate<2>(acc_ar_2[i], max_ar_2[i], acc_ar_3[i], max_ar_3[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar_3, max_ar_3, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_24222(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[8][4];
    ac_int<E+2    , true> max_ar[8][4];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce<2>(A[i*4+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[4][2];
    ac_int<E+2    , true> max_ar_1[4][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<4>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_2[2][2];
    ac_int<E+2    , true> max_ar_2[2][2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar_1[i*2+j], max_ar_1[i*2+j], acc_ar_2[i][j], max_ar_2[i][j]);
      }
    }

    ac_int<M+1+1+5, true> acc_ar_3[2];
    ac_int<E+2    , true> max_ar_3[2];
  
    ROW_4:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar_2[i], max_ar_2[i], acc_ar_3[i], max_ar_3[i]);
    }

    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_3, max_ar_3, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_222222(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[16][2];
    ac_int<E+2    , true> max_ar[16][2];
    
    ffloat<M,E> A[32][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 32; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[8][2];
    ac_int<E+2    , true> max_ar_1[8][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_2[4][2];
    ac_int<E+2    , true> max_ar_2[4][2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar_1[i*2+j], max_ar_1[i*2+j], acc_ar_2[i][j], max_ar_2[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_3[2][2];
    ac_int<E+2    , true> max_ar_3[2][2];

    ROW_4:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar_2[i*2+j], max_ar_2[i*2+j], acc_ar_3[i][j], max_ar_3[i][j]);
      }
    }

    ac_int<M+1+1+5, true> acc_ar_4[2];
    ac_int<E+2    , true> max_ar_4[2];

    ROW_5:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar_3[i], max_ar_3[i], acc_ar_4[i], max_ar_4[i]);
    }
    
    ac_int<M+1+1+6, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_4, max_ar_4, acc, maxExp);
    
    #include "norm_and_round.h"
  }


  // IN 32
  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_162(ffloat<M,E> A[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;
    
    ac_int<M+1+1+4, true> acc_ar[2];
    ac_int<E+2    , true> max_ar[2];
    
    ffloat<M,E> A_0[16];
    ffloat<M,E> A_1[16];

    SET_A0:
    #pragma hls_unroll
    for(int i = 0; i < 16; i++) {
      A_0[i] = A[i];
    }
    SET_A1:
    #pragma hls_unroll
    for(int i = 0; i < 16; i++) {
      A_1[i] = A[16 + i];
    }

    radix_reduce<16>(A_0, acc_ar[0], max_ar[0]);
    radix_reduce<16>(A_1, acc_ar[1], max_ar[1]);

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_216(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[16];
    ac_int<E+2    , true> max_ar[16];
    
    ffloat<M,E> A[16][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      radix_reduce<2>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<16>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_282(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[2][8];
    ac_int<E+2    , true> max_ar[2][8];
    
    ffloat<M,E> A[16][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        radix_reduce<2>(A[i*8+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_1[2];
    ac_int<E+2    , true> max_ar_1[2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<8>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_244(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[4][4];
    ac_int<E+2    , true> max_ar[4][4];
    
    ffloat<M,E> A[16][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce<2>(A[i*4+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[4];
    ac_int<E+2    , true> max_ar_1[4];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce_intermediate<4>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_442(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[2][4];
    ac_int<E+2    , true> max_ar[2][4];
    
    ffloat<M,E> A[8][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce<4>(A[i*4+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_1[2];
    ac_int<E+2    , true> max_ar_1[2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<4>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_822(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+3, true> acc_ar[2][2];
    ac_int<E+2    , true> max_ar[2][2];
    
    ffloat<M,E> A[4][8];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        A[i][j] = A_i[i*8+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<8>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_1[2];
    ac_int<E+2    , true> max_ar_1[2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_424(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[4][2];
    ac_int<E+2    , true> max_ar[4][2];
    
    ffloat<M,E> A[8][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<4>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[4];
    ac_int<E+2    , true> max_ar_1[4];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce_intermediate<2>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_84(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+3, true> acc_ar[4];
    ac_int<E+2    , true> max_ar[4];
    
    ffloat<M,E> A[4][8];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 8; j++) {
        A[i][j] = A_i[i*8+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce<8>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_48(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[8];
    ac_int<E+2    , true> max_ar[8];
    
    ffloat<M,E> A[8][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      radix_reduce<4>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<8>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_228(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[8][2];
    ac_int<E+2    , true> max_ar[8][2];
    
    ffloat<M,E> A[16][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[8];
    ac_int<E+2    , true> max_ar_1[8];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      radix_reduce_intermediate<2>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<8>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_2224(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[8][2];
    ac_int<E+2    , true> max_ar[8][2];
    
    ffloat<M,E> A[16][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[4][2];
    ac_int<E+2    , true> max_ar_1[4][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_2[4];
    ac_int<E+2    , true> max_ar_2[4];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce_intermediate<2>(acc_ar_1[i], max_ar_1[i], acc_ar_2[i], max_ar_2[i]);
    }
  
    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar_2, max_ar_2, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_2242(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[8][2];
    ac_int<E+2    , true> max_ar[8][2];
    
    ffloat<M,E> A[16][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[2][4];
    ac_int<E+2    , true> max_ar_1[2][4];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce_intermediate<2>(acc_ar[i*4+j], max_ar[i*4+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_2[2];
    ac_int<E+2    , true> max_ar_2[2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<4>(acc_ar_1[i], max_ar_1[i], acc_ar_2[i], max_ar_2[i]);
    }
  
    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_2, max_ar_2, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_2422(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[4][4];
    ac_int<E+2    , true> max_ar[4][4];
    
    ffloat<M,E> A[16][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce<2>(A[i*4+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[2][2];
    ac_int<E+2    , true> max_ar_1[2][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<4>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_2[2];
    ac_int<E+2    , true> max_ar_2[2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar_1[i], max_ar_1[i], acc_ar_2[i], max_ar_2[i]);
    }
  
    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_2, max_ar_2, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_4222(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[4][2];
    ac_int<E+2    , true> max_ar[4][2];
    
    ffloat<M,E> A[8][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<4>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[2][2];
    ac_int<E+2    , true> max_ar_1[2][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_2[2];
    ac_int<E+2    , true> max_ar_2[2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar_1[i], max_ar_1[i], acc_ar_2[i], max_ar_2[i]);
    }
  
    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_2, max_ar_2, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_22222(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[8][2];
    ac_int<E+2    , true> max_ar[8][2];
    
    ffloat<M,E> A[16][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 16; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[4][2];
    ac_int<E+2    , true> max_ar_1[4][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_2[2][2];
    ac_int<E+2    , true> max_ar_2[2][2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar_1[i*2+j], max_ar_1[i*2+j], acc_ar_2[i][j], max_ar_2[i][j]);
      }
    }

    ac_int<M+1+1+4, true> acc_ar_3[2];
    ac_int<E+2    , true> max_ar_3[2];

    ROW_4:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar_2[i], max_ar_2[i], acc_ar_3[i], max_ar_3[i]);
    }

    ac_int<M+1+1+5, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_3, max_ar_3, acc, maxExp);
    
    #include "norm_and_round.h"
  }

  // IN 16
  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_82(ffloat<M,E> A[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;
    
    typedef ac_int<E+2   , true> k_t;
    typedef ac_int<sum_bw, true> o_t;

    ac_int<M+1+1+3, true> acc_ar[2];
    ac_int<E+2    , true> max_ar[2];
    
    ffloat<M,E> A_0[8];
    ffloat<M,E> A_1[8];

    SET_A0:
    #pragma hls_unroll
    for(int i = 0; i < 8; i++) {
      A_0[i] = A[i];
    }
    SET_A1:
    #pragma hls_unroll
    for(int i = 0; i < 8; i++) {
      A_1[i] = A[8 + i];
    }

    radix_reduce<8>(A_0, acc_ar[0], max_ar[0]);
    radix_reduce<8>(A_1, acc_ar[1], max_ar[1]);

    ac_int<M+1+1+4, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_28(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[8];
    ac_int<E+2    , true> max_ar[8];
    
    ffloat<M,E> A[8][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      radix_reduce<2>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+4, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<8>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_242(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[2][4];
    ac_int<E+2    , true> max_ar[2][4];
    
    ffloat<M,E> A[8][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        radix_reduce<2>(A[i*4+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[2];
    ac_int<E+2    , true> max_ar_1[2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<4>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+4, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_224(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[4][2];
    ac_int<E+2    , true> max_ar[4][2];
    
    ffloat<M,E> A[8][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[4];
    ac_int<E+2    , true> max_ar_1[4];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce_intermediate<2>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+4, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_422(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[2][2];
    ac_int<E+2    , true> max_ar[2][2];
    
    ffloat<M,E> A[4][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<4>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_1[2];
    ac_int<E+2    , true> max_ar_1[2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar[i], max_ar[i], acc_ar_1[i], max_ar_1[i]);
    }

    ac_int<M+1+1+4, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_1, max_ar_1, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_44(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+2, true> acc_ar[4];
    ac_int<E+2    , true> max_ar[4];
    
    ffloat<M,E> A[4][4];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 4; j++) {
        A[i][j] = A_i[i*4+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      radix_reduce<4>(A[i], acc_ar[i], max_ar[i]);
    }

    ac_int<M+1+1+4, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<4>(acc_ar, max_ar, acc, maxExp);

    #include "norm_and_round.h"
  }

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce_online_2222(ffloat<M,E> A_i[N]) {
    const int sumbits = CLOG2(N);

    /// @note This is the width of the accumulator
    /// (ovf_bits)(1)(mantissa)(extra4prec)
    const int sum_bw = (sumbits + 1) + 1 + man_width + 0;

    ac_int<M+1+1+1, true> acc_ar[4][2];
    ac_int<E+2    , true> max_ar[4][2];
    
    ffloat<M,E> A[8][2];

    SET_A_i:
    #pragma hls_unroll
    for (int i = 0; i < 8; i++) {
      SET_A_i_j:
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        A[i][j] = A_i[i*2+j];
      }
    }

    ROW_1:
    #pragma hls_unroll
    for (int i = 0; i < 4; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce<2>(A[i*2+j], acc_ar[i][j], max_ar[i][j]);
      }
    }

    ac_int<M+1+1+2, true> acc_ar_1[2][2];
    ac_int<E+2    , true> max_ar_1[2][2];

    ROW_2:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      #pragma hls_unroll
      for (int j = 0; j < 2; j++) {
        radix_reduce_intermediate<2>(acc_ar[i*2+j], max_ar[i*2+j], acc_ar_1[i][j], max_ar_1[i][j]);
      }
    }

    ac_int<M+1+1+3, true> acc_ar_2[2];
    ac_int<E+2    , true> max_ar_2[2];

    ROW_3:
    #pragma hls_unroll
    for (int i = 0; i < 2; i++) {
      radix_reduce_intermediate<2>(acc_ar_1[i], max_ar_1[i], acc_ar_2[i], max_ar_2[i]);
    }

    ac_int<M+1+1+4, true> acc;
    ac_int<E+2    , true> maxExp;
    radix_reduce_intermediate<2>(acc_ar_2, max_ar_2, acc, maxExp);

    #include "norm_and_round.h"
  }

  /** THE BASELINE ADDER
    The baseline adder that uses a single radix operator
    of size equal to the input's size.
  **/

  template<int N, RND_ENUM RND_MODE=EVEN, bool DENORMALS=false>
  void fpaReduce(ffloat<M,E> A[N]){
    static const int mul_W = M+1;

    ac_int<E+2  , true > mul_exp_result [N];
    ac_int<mul_W, false> mul_prod       [N];
    ac_int<1    , false> mulSign        [N];
 
    init_start_nodes<N, ffloat<M,E>>(
      A,
      mul_exp_result,
      mul_prod,
      mulSign
    );
    
    ac_int<E+2,true> maxExp = max<N>(mul_exp_result);
    
    static const int ext_bs = 0;
    
    // ac_int<E+2,true> shifted_exp_res[N];
    ac_int<mul_W+ext_bs,false> shifted_prod[N];
    ac_int<E+1,true> diffE[N];
    #pragma hls_unroll
    ALLIGN: for (int i=0; i<N; i++) {
      diffE[i] = maxExp-mul_exp_result[i];
      shifted_prod[i] = mul_prod[i] >> diffE[i];
    }

    static const int acc_w  = mul_W+1+CLOG2(N)+ext_bs;
        
    ac_int<mul_W+1+ext_bs,true> add_op[N];
    ac_int<acc_w,true> acc=0;

    #pragma hls_unroll
    TWOs_COMPL_ADD: for (int i=0; i<N; i++) {
      if (mulSign[i]) {
        add_op[i] = -shifted_prod[i];
      } else {
        add_op[i] = shifted_prod[i];
      }   
      acc += add_op[i];
      
    }

    // A simple renaming of a variable
    static const int sum_bw = acc_w;

    #include "norm_and_round.h"
  }

public:
  // Constructors and Destructor
  ffloat() {};
  ffloat(const ac_int<M+E+1,false> &in) {
    this->operator=(in);
  }
  ffloat(const ffloat<M,E> &in) {
    this->operator=(in);
  }
  ffloat(const float fval) {
    this->operator=(fval);
  }
  ~ffloat() {};

  /*
  * .to_float() is a non-synthesizable function that
  * is used for converting the ffloat into a c++ float
  */
  float to_float() const{
  #ifndef __SYNTHESIS__

    ac_int<32,false> data;

    for (int i = 0; i < M ; i++) data[22-i] = mantissa[M-1-i];
    for (int i = M; i < 23; i++) data[22-i] = 0;
    
    ac_int<8,false> tmpExp = (exponent==0) ? ( ac_int<8,false>)0 : ( ac_int<8,false>)(exponent - e_bias + 127);
    
    switch(E) {
      /// FP32, bfloat16, FP8_e6m1
      case 6:
      case 8: {
        for (int i=0; i<8; i++) data[23+i] = (exponent == (1<<E)+-1) ? 1 : tmpExp[i];
        break;
      }
      
      /// OFP w/ e4m3 (FP8_e4m3)
      case 4: {
        bit_t is_nan = mantissa.and_reduce() && exponent.and_reduce();
        ac_int<8, false> ofp_exp = (exponent==0) ? ac_int<8, false>(0) : is_nan ? ac_int<8, false>(-1) : ac_int<8, false>(exponent - e_bias + 127);
        data.set_slc(23, ofp_exp);
        data.set_slc(23-M, is_nan ? man_t(-1) : this->mantissa);
        break;
      }
      
      /// OFP w/ e5m2 (FP8_e5m2)
      case 5: {
        bit_t is_nan = mantissa.or_reduce() && exponent.and_reduce();
        bit_t is_inf = !mantissa.or_reduce() && exponent.and_reduce();
        ac_int<8, false> ofp_exp = (exponent==0) ? ac_int<8, false>(0) : (is_nan | is_inf) ? ac_int<8, false>(-1) : ac_int<8, false>(exponent - e_bias + 127);
        data.set_slc(23, ofp_exp);
        data.set_slc(23-M, is_inf ? man_t(0) : this->mantissa);
        break;
      }
    }
    
    data[31] = sign;

    ac_std_float<32,8> ieee_data;
    ieee_data.set_data(data);
    return ieee_data.to_float();

  #endif
  }

  ac_int<M+1, false>
  sig() const {
    ac_int<M+1, false> significand = mantissa;
    significand[M] = 1;

    return significand;
  }

  bit_t
  round_float(ac_int<32, false> float32) {
    static const int ind = (M < 23) ? (32-1-8-M) : 1;

    ac_int<2, false> rsel;

    rsel[0] = float32.template slc<ind-1>(0).or_reduce();
    rsel[1] = float32[ind-1];

    switch(rsel) {
      case 0:
      case 1:
        return 0;
      case 2:
        return float32[ind];
      case 3:
        return 1;
    }

    return 0;
  }

  // A non-synthesizable function to convert
  // a C++ double to a ffloat
  void operator = (const double &inFP) {
    #ifndef __SYNTHESIS__
      double y = inFP;

      ac_int<64, false> y_bits = *(unsigned long long *)(&y);
      
      static const int ye_width = 11;
      static const int ym_width = 52;
      static const int y_bias   = 1023;

      ac_int<1       , false> y_sign     = y_bits[y_bits.length()-1];
      ac_int<ye_width, false> y_exponent = y_bits.template slc<ye_width>(y_bits.length()-1-ye_width);
      ac_int<ym_width, false> y_mantissa = y_bits.template slc<ym_width>(0);
      
      bit_t is_denorm = (y_exponent == 0);
      bit_t nan_in    = y_exponent.and_reduce() && y_mantissa.or_reduce();

      sign = y_sign;

      /// ffpe4m3
      if (E == 4) {
        static const double min_norm = 1 / (double)(1<<6);

        if (!nan_in) {
          if ( (inFP > 448) || (inFP < -448) ) {
            exponent = -1;
            mantissa = OFP_SAT ? 6 : -1;
          } else if ( (inFP < min_norm) && (inFP > -min_norm)) {
            exponent = mantissa = 0;
          } else {
            exponent = exp_t(y_exponent - y_bias + e_bias);

            ac_int<M+1, false> rmant = y_mantissa.template slc<M>(y_mantissa.length()-M) + round_float(y_bits);
            ac_int<E+1, false> rexp  = exponent + rmant[M];

            if (rexp > 15) {
              mantissa = OFP_SAT ? 6 : -1;
              exponent = -1;
            } else {
              mantissa = rmant.template slc<M>(0);
              exponent = rmant[M] ? exp_t(rexp.template slc<E>(0)) : exponent;
            }
          }
        } else {
          exponent = -1;
          mantissa = -1;
        }

      /// ffpe5m2
      } else if (E == 5) {
        static const double min_norm = 1 / (double)(1<<14);

        bit_t nan_in  = y_exponent.and_reduce() && y_mantissa.or_reduce();

        if (!nan_in) {
          if ( (inFP > 57344) || (inFP < -57344) ) {
            exponent = OFP_SAT ? -2 : -1;
            mantissa = -1;
          } else if ( (inFP < min_norm) && (inFP > -min_norm) ) {
            exponent = mantissa = 0;
          } else {
            exponent = exp_t(y_exponent - y_bias + e_bias);

            ac_int<M+1, false> rmant = y_mantissa.template slc<M>(y_mantissa.length()-M) + round_float(y_bits);
            ac_int<E+1, false> rexp  = exponent + rmant[M];

            if (rexp > 31) {
              exponent = OFP_SAT ? -2 : -1;
              mantissa = -1;
            } else {
              mantissa = rmant.template slc<M>(0);
              exponent = rmant[M] ? exp_t(rexp.template slc<E>(0)) : exponent;
            }
          }
        } else {
          exponent = -1;
          mantissa = -1;
        }
      /// ffp16b
      } else if (E == 8 && M == 7){
        if (!nan_in) {

          if (is_denorm) {
            exponent = 0;
          } else {
            exponent = y_exponent - y_bias + e_bias;
          }

          ac_int<M+1, false> rmant = y_mantissa.template slc<M>(y_mantissa.length()-M) + round_float(y_bits);
          ac_int<E+1, false> rexp  = exponent + 1;

          if (rexp > 255) {
            mantissa = -1;
            exponent = -2;
          } else {
            mantissa = rmant.template slc<M>(0);
            exponent = rmant[M] ? exp_t(rexp)              : exponent;
          }
        } else {
          exponent = -1;
          mantissa = -1;
        }
        
      /// ffpe6mX
      } else if (E == 6) {
        if (!nan_in) {
          exponent = y_exponent - y_bias + e_bias;

          ac_int<M+1, false> rmant = y_mantissa[ym_width-1] + round_float(y_bits);
          ac_int<E+1, false> rexp  = exponent + 1;

          if (rexp > 65) {
            mantissa = -1;
            exponent = -2;
          } else {
            mantissa = rmant.template slc<M>(0);
            exponent = rmant[M] ? exp_t(rexp) : exponent;
          }
        } else {
          exponent = -1;
          mantissa = -1;
        }
      /// ffp32
      } else {
        if (!nan_in) {
          exponent = y_exponent;

          ac_int<M+1, false> rmant = y_mantissa.template slc<M>(y_mantissa.length()-M) + round_float(y_bits);
          ac_int<E+1, false> rexp  = exponent + 1;

          if (rexp > 255) {
            mantissa = -1;
            exponent = -2;
          } else {
            mantissa = rmant.template slc<M>(0);
            exponent = rmant[M] ? exp_t(rexp)              : exponent;
          }
        } else {
          exponent = -1;
          mantissa = -1;
        }
      }
    #endif
  }

private:
  template<int N, class data_t>
  void
  init_start_nodes(
    data_t                             ffp_array      [N],

    ac_int<data_t::exp_width+2, true > mul_exp_result [N],
    ac_int<data_t::man_width+1, false> mul_prod       [N],
    ac_int<1                  , false> mulSign        [N]
  ){
    #pragma hls_unroll
    for (int i = 0; i < N; i++) {
      mul_exp_result[i]              = ffp_array[i].exponent;
      mul_prod[i]                    = ffp_array[i].mantissa;
      mul_prod[i][data_t::man_width] = ffp_array[i].exponent.or_reduce();
      mulSign[i]                     = ffp_array[i].sign;
    }
  }
};

// single-precission IEEE754 
typedef ffloat<23,8> fp32;
// bfloat16
typedef ffloat<7, 8> bf16; 
// OFP 8-bit floats
typedef ffloat<3, 4> fp8_e4m3; 
typedef ffloat<2, 5> fp8_e5m2;

// 8-bit cornercase
typedef ffloat<1, 6> fp8_e6m1;
#endif
