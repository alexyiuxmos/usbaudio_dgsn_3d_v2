// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __DSP_OP_H__
#define __DSP_OP_H__

#include <stdint.h>

typedef int32_t AUDIO_T;

#define FRAME_SIZE     128                           //128 samples / 48kHz = 2.7ms
#define PL_BUF_SIZE    (FRAME_SIZE*NUM_USB_CHAN_IN)  //TODO: assume pipeline buffer size is fixed to this number
#define PL_STAGE_MAX   1

#endif
