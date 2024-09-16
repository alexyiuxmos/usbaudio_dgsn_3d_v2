// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
//
#ifndef __DSP_FUNC_H__
#define __DSP_FUNC_H__

#include "xmath/xmath.h"

extern void do_filter_biquad_s32(filter_biquad_s32_t *pfilter, int in_buf[FRAME_SIZE], int out_buf[FRAME_SIZE]);
extern filter_biquad_s32_t filter_notch_3khz;
extern filter_biquad_s32_t filter_notch_8khz;

extern void do_amplify(int buf[FRAME_SIZE], int multipler, int q_factor);

#endif

