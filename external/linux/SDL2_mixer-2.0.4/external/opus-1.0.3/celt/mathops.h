/* Copyright (c) 2002-2008 Jean-Marc Valin
   Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2009 Xiph.Org Foundation
   Written by Jean-Marc Valin */
/**
   @file mathops.h
   @brief Various math functions
*/
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MATHOPS_H
#define MATHOPS_H

#include "arch.h"
#include "entcode.h"
#include "os_support.h"

/* Multiplies two 16-bit fractional values. Bit-exactness of this macro is important */
#define FRAC_MUL16(a,b) ((16384+((opus_int32)(opus_int16)(a)*(opus_int16)(b)))>>15)

unsigned isqrt32(opus_uint32 _val);

#ifndef FIXED_POINT

#define PI 3.141592653f
#define celt_sqrt(x) ((float)sqrt(x))
#define celt_rsqrt(x) (1.f/celt_sqrt(x))
#define celt_rsqrt_norm(x) (celt_rsqrt(x))
#define celt_cos_norm(x) ((float)cos((.5f*PI)*(x)))
#define celt_rcp(x) (1.f/(x))
#define celt_div(a,b) ((a)/(b))
#define frac_div32(a,b) ((float)(a)/(b))

#ifdef FLOAT_APPROX

/* Note: This assumes radix-2 floating point with the exponent at bits 23..30 and an offset of 127
         denorm, +/- inf and NaN are *not* handled */

/** Base-2 log approximation (log2(x)). */
static inline float celt_log2(float x)
{
   int integer;
   float frac;
   union {
      float f;
      opus_uint32 i;
   } in;
   in.f = x;
   integer = (in.i>>23)-127;
   in.i -= integer<<23;
   frac = in.f - 1.5f;
   frac = -0.41445418f + frac*(0.95909232f
          + frac*(-0.33951290f + frac*0.16541097f));
   return 1+integer+frac;
}

/** Base-2 exponential approximation (2^x). */
static inline float celt_exp2(float x)
{
   int integer;
   float frac;
   union {
      float f;
      opus_uint32 i;
   } res;
   integer = floor(x);
   if (integer < -50)
      return 0;
   frac = x-integer;
   /* K0 = 1, K1 = log(2), K2 = 3-4*log(2), K3 = 3*log(2) - 2 */
   res.f = 0.99992522f + frac * (0.69583354f
           + frac * (0.22606716f + 0.078024523f*frac));
   res.i = (res.i + (integer<<23)) & 0x7fffffff;
   return res.f;
}

#else
#define celt_log2(x) ((float)(1.442695040888963387*log(x)))
#define celt_exp2(x) ((float)exp(0.6931471805599453094*(x)))
#endif

#endif

#ifdef FIXED_POINT

#include "os_support.h"

#ifndef OVERRIDE_CELT_ILOG2
/** Integer log in base2. Undefined for zero and negative numbers */
static inline opus_int16 celt_ilog2(opus_int32 x)
{
   celt_assert2(x>0, "celt_ilog2() only defined for strictly positive numbers");
   return EC_ILOG(x)-1;
}
#endif

#ifndef OVERRIDE_CELT_MAXABS16
static inline opus_val32 celt_maxabs16(const opus_val16 *x, int len)
{
   int i;
   opus_val16 maxval = 0;
   opus_val16 minval = 0;
   for (i=0;i<len;i++)
   {
      maxval = MAX16(maxval, x[i]);
      minval = MIN16(minval, x[i]);
   }
   return MAX32(EXTEND32(maxval),-EXTEND32(minval));
}
#endif

#ifndef OVERRIDE_CELT_MAXABS32
static inline opus_val32 celt_maxabs32(opus_val32 *x, int len)
{
   int i;
   opus_val32 maxval = 0;
   for (i=0;i<len;i++)
      maxval = MAX32(maxval, ABS32(x[i]));
   return maxval;
}
#endif

/** Integer log in base2. Defined for zero, but not for negative numbers */
static inline opus_int16 celt_zlog2(opus_val32 x)
{
   return x <= 0 ? 0 : celt_ilog2(x);
}

opus_val16 celt_rsqrt_norm(opus_val32 x);

opus_val32 celt_sqrt(opus_val32 x);

opus_val16 celt_cos_norm(opus_val32 x);

static inline opus_val16 celt_log2(opus_val32 x)
{
   int i;
   opus_val16 n, frac;
   /* -0.41509302963303146, 0.9609890551383969, -0.31836011537636605,
       0.15530808010959576, -0.08556153059057618 */
   static const opus_val16 C[5] = {-6801+(1<<(13-DB_SHIFT)), 15746, -5217, 2545, -1401};
   if (x==0)
      return -32767;
   i = celt_ilog2(x);
   n = VSHR32(x,i-15)-32768-16384;
   frac = ADD16(C[0], MULT16_16_Q15(n, ADD16(C[1], MULT16_16_Q15(n, ADD16(C[2], MULT16_16_Q15(n, ADD16(C[3], MULT16_16_Q15(n, C[4]))))))));
   return SHL16(i-13,DB_SHIFT)+SHR16(frac,14-DB_SHIFT);
}

/*
 K0 = 1
 K1 = log(2)
 K2 = 3-4*log(2)
 K3 = 3*log(2) - 2
*/
#define D0 16383
#define D1 22804
#define D2 14819
#define D3 10204
/** Base-2 exponential approximation (2^x). (Q10 input, Q16 output) */
static inline opus_val32 celt_exp2(opus_val16 x)
{
   int integer;
   opus_val16 frac;
   integer = SHR16(x,10);
   if (integer>14)
      return 0x7f000000;
   else if (integer < -15)
      return 0;
   frac = SHL16(x-SHL16(integer,10),4);
   frac = ADD16(D0, MULT16_16_Q15(frac, ADD16(D1, MULT16_16_Q15(frac, ADD16(D2 , MULT16_16_Q15(D3,frac))))));
   return VSHR32(EXTEND32(frac), -integer-2);
}

opus_val32 celt_rcp(opus_val32 x);

#define celt_div(a,b) MULT32_32_Q31((opus_val32)(a),celt_rcp(b))

opus_val32 frac_div32(opus_val32 a, opus_val32 b);

#define M1 32767
#define M2 -21
#define M3 -11943
#define M4 4936

/* Atan approximation using a 4th order polynomial. Input is in Q15 format
   and normalized by pi/4. Output is in Q15 format */
static inline opus_val16 celt_atan01(opus_val16 x)
{
   return MULT16_16_P15(x, ADD32(M1, MULT16_16_P15(x, ADD32(M2, MULT16_16_P15(x, ADD32(M3, MULT16_16_P15(M4, x)))))));
}

#undef M1
#undef M2
#undef M3
#undef M4

/* atan2() approximation valid for positive input values */
static inline opus_val16 celt_atan2p(opus_val16 y, opus_val16 x)
{
   if (y < x)
   {
      opus_val32 arg;
      arg = celt_div(SHL32(EXTEND32(y),15),x);
      if (arg >= 32767)
         arg = 32767;
      return SHR16(celt_atan01(EXTRACT16(arg)),1);
   } else {
      opus_val32 arg;
      arg = celt_div(SHL32(EXTEND32(x),15),y);
      if (arg >= 32767)
         arg = 32767;
      return 25736-SHR16(celt_atan01(EXTRACT16(arg)),1);
   }
}

#endif /* FIXED_POINT */
#endif /* MATHOPS_H */
