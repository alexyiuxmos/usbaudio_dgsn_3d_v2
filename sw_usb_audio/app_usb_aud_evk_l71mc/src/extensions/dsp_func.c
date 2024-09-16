// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#if (DSP_TASK == 1)
#include <stdio.h>
#include <stdlib.h>

#include "xmath/xmath.h"
#include "dsp_op.h"

#define SECTION_COUNT   1
#define FILTER_TICKS    FRAME_SIZE

filter_biquad_s32_t filter_notch_3khz = {
  // Number of biquad sections in this filter block
  .biquad_count = SECTION_COUNT,
  
  // Filter state, initialized to 0
  .state = {{0}},

  // Filter coefficients
  .coef = {
    //b0, b1, b2, -a1, -a2
    {Q28(0.7870056430245259) },
    {Q28(-1.4541968111224874)},
    {Q28(0.7870056430245259) },
    {Q28(1.4541968111224874) },
    {Q28(-0.574011286049052) }
  }
};

filter_biquad_s32_t filter_notch_8khz = {
  // Number of biquad sections in this filter block
  .biquad_count = SECTION_COUNT,
  
  // Filter state, initialized to 0
  .state = {{0}},

  // Filter coefficients
  .coef = {
    { Q28(0.6201685286720973)   },
    { Q28(-0.6201685286720974)  },
    { Q28(0.6201685286720973)   },
    { Q28(0.6201685286720974)   },
    { Q28(-0.24033705734419472) }
  }
};

void do_filter_biquad_s32(filter_biquad_s32_t *pfilter, int in_buf[FRAME_SIZE], int out_buf[FRAME_SIZE]) {
    for(int i = 0; i < FILTER_TICKS; i++){
        out_buf[i] = filter_biquad_s32(pfilter, in_buf[i]);
    }
}

void do_amplify(int buf[FRAME_SIZE], int multipler, int q_factor) {
    long long result;
    for (int i = 0; i < FRAME_SIZE; i++) {
        result = (long long)buf[i] * multipler;
        buf[i] = (int)(result>>q_factor);
    }
}

#endif  //DSP_tASK
