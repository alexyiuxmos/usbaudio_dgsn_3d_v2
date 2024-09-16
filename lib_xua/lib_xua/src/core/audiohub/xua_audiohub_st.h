// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#if (USE_EX3D == 1) || (DSP_TASK == 1)
#if (USE_EX3D == 1)
#include "c_dsp.h"
#endif
#if (DSP_TASK == 1)
#include "dsp_op.h"
#endif

extern "C" {
    // Double buffers to reduce time spent on moving data via channel
    extern AUDIO_T usb_to_dsp_buf[2][NUM_USB_CHAN_OUT][FRAME_SIZE];
    extern AUDIO_T dsp_to_usb_buf[2][NUM_USB_CHAN_IN][FRAME_SIZE];
}

int bank=0;
int samp_idx=0;
#endif

#pragma unsafe arrays
static inline unsigned DoSampleTransfer(chanend ?c_out, const int readBuffNo, const unsigned underflowWord
#if (USE_EX3D == 1) || (DSP_TASK == 1)
        , chanend c_dsp
#endif
    )
{
    if(XUA_USB_EN)
    {
        outuint(c_out, underflowWord);

        /* Check for sample freq change (or other command) or new samples from mixer*/
        if(testct(c_out))
        {
            unsigned command = inct(c_out);
#ifndef CODEC_MASTER
            if(dsdMode == DSD_MODE_OFF)
            {
#if (I2S_CHANS_ADC != 0 || I2S_CHANS_DAC != 0)
                /* Set clocks low */
                p_lrclk <: 0;
                p_bclk <: 0;
#endif
            }
            else
            {
#if(DSD_CHANS_DAC != 0)
                /* DSD Clock might not be shared with lrclk or bclk... */
                p_dsd_clk <: 0;
#endif
            }
#endif
#if (DSD_CHANS_DAC > 0)
            if(dsdMode == DSD_MODE_DOP)
                dsdMode = DSD_MODE_OFF;
#endif
                return command;
        }
        else
        {
#if NUM_USB_CHAN_OUT > 0
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
            {
                int tmp = inuint(c_out);
                samplesOut[i] = tmp;
            }
#else
            inuint(c_out);
#endif
            UserBufferManagement(samplesOut, samplesIn[readBuffNo]);

#if (USE_EX3D == 1) || (DSP_TASK == 1)
#pragma loop unroll
#if defined(AUDIO_T_16)
            for(int ch = 0; ch < 2; ch++) {
                usb_to_dsp_buf[bank][ch][samp_idx] = (AUDIO_T)(samplesOut[ch] / 65536);
                samplesIn[readBuffNo][ch] = ((int32_t)dsp_to_usb_buf[bank][ch][samp_idx]) * 65536;
#if (USE_LOOPBACK_TEST == 1)
                samplesIn[readBuffNo][ch] = ((int32_t)dsp_to_usb_buf[bank][ch][samp_idx]) * 65536;
#endif
            }
            for(int ch = 2; ch < NUM_USB_CHAN_OUT; ch++) {
                usb_to_dsp_buf[bank][ch][samp_idx] = (AUDIO_T)(samplesOut[ch] / 65536);
                //samplesOut[ch] = 0;
            }
#else
            for(int ch = 0; ch < 2; ch++) {
                usb_to_dsp_buf[bank][ch][samp_idx] = samplesOut[ch];
                samplesOut[ch] = dsp_to_usb_buf[bank][ch][samp_idx];
#if (USE_LOOPBACK_TEST == 1)
                samplesIn[readBuffNo][ch] = dsp_to_usb_buf[bank][ch][samp_idx];
#endif
            }
            for(int ch = 2; ch < NUM_USB_CHAN_OUT; ch++) {
                usb_to_dsp_buf[bank][ch][samp_idx] = samplesOut[ch];
                samplesOut[ch] = 0;
            }
#endif

            //process in frame
            samp_idx++;
            if (samp_idx == FRAME_SIZE) {
                samp_idx = 0;   //reset index
                c_dsp <: bank;  //signal dsp task
                bank ^= 1;      //toggle bank
            }
#endif

#if NUM_USB_CHAN_IN > 0
#pragma loop unroll
            for(int i = 0; i < NUM_USB_CHAN_IN; i++)
            {
                outuint(c_out, samplesIn[readBuffNo][i]);
            }
#endif
        }
    }
    else
        UserBufferManagement(samplesOut, samplesIn[readBuffNo]);

    return 0;
}

