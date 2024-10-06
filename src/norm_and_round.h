/**
  NORMALIZATION AND ROUND LOGIC. THIS IS 
  THE SAME FOR EVERY ADDER. THIS FILE IS 
  INCLUDED AT THE END OF EACH ADDER IN 
  `multi_term_adders.h` FILE.
*/

bit_t res_sign = acc[acc.length()-1];

ac_int<sum_bw, false> res1 = acc;
ac_int<sum_bw, false> res2 = acc.bit_complement() + res_sign;

ac_int<sum_bw-1, false> res;
res = (res_sign) ? res2.template slc<sum_bw-1>(0) : res1.template slc<sum_bw-1>(0);

bit_t r_norm_needed = res[res.length()-2];

/// @note LZC is fit only for power of two input sizes
/// @param ww is the input size padded to the nearest power of two
const int ww = (sum_bw-1 <= 8)  ? 8  :
                (sum_bw-1 <= 16) ? 16 :
                (sum_bw-1 <= 32) ? 32 : 64;

/// @note Number of extra LSB bits from ww bits
const int uu = ww - sum_bw + 1;

ac_int<ww, false> tcz = 0;    
#pragma hls_unroll
for (int ii=0; ii<sum_bw-1; ii++)
  tcz[ii+uu]=res[ii];

ac_int<BITS(sum_bw-1), false> lead_zer = lzcount<ww>(tcz);

/// @note When an ace is present after the significand
/// this means that an normalization to the right is needed.
/// Instead of that I just add 1 to the exponent (r_norm_needed).
res    <<= lead_zer;
maxExp  -= (lead_zer - CLOG2(N));

/// Keep M+1 MSBs of res
static const int  ind         = sum_bw-1 - M-1;
ac_int<M+2,false> rounded_res = res.template slc<M+1>(ind);

ac_int<2, false> rsel;
rsel[0] = res.template slc<ind-1>(0).or_reduce();
rsel[1] = res[ind-1];

ac_int<1,false> mul_round;
switch (RND_MODE) {
  case EVEN:
    // Round to nearest tie to even.
    switch(rsel) {
      case 0:
      case 1:
        mul_round = 0;
        break;
      case 2:
        mul_round = res[ind];
        break;
      case 3:
        mul_round = 1;
        break;
    }

    break;
}
rounded_res += (mul_round);

exp_t tmp_exp;
exp_t over_exp;
bit_t exp_ovf, sat_sel, man_is_max, exp_is_max;
man_t temp_mant;
switch(E) {
  /// FP32, bfloat16, FP8_e6m1
  case 6:
  case 8:
    tmp_exp  = maxExp.template slc<E>(0);
    over_exp = (tmp_exp == (1<<E)-1) ? tmp_exp : (exp_t)(tmp_exp+1);

    mantissa = rounded_res.template slc<M>(0); 
    exponent = (rounded_res[M+1]) ?  over_exp : rounded_res.or_reduce() ? tmp_exp : exp_t(0);
    sign     = res_sign;

    break;
  
  /// ffp_ofp34
  case 4:
    maxExp += rounded_res[M+1];
    
    tmp_exp    = maxExp.template slc<E>(0);
    temp_mant  = rounded_res.template slc<M>(0);
    
    exp_ovf    = maxExp.template slc<2>(E).xor_reduce();
    exp_is_max = exp_ovf | tmp_exp.and_reduce();
    man_is_max = (exp_is_max & temp_mant.and_reduce()) | exp_ovf;

    mantissa = man_is_max ? man_t(6 ) : temp_mant;
    exponent = exp_is_max ? exp_t(-1) : rounded_res.or_reduce() ? tmp_exp : exp_t(0);
    sign     = res_sign;

    break;
  
  /// ffp_ofp25
  case 5:
    tmp_exp  = maxExp.template slc<E>(0);
    over_exp = (tmp_exp == (1<<E)-1) ? tmp_exp : (exp_t)(tmp_exp+1);

    mantissa = rounded_res.template slc<M>(0); 
    exponent = (rounded_res[M+1]) ?  over_exp : rounded_res.or_reduce() ? tmp_exp : exp_t(0);
    sign     = res_sign;

    break;
}
