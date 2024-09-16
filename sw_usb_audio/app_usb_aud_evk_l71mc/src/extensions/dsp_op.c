// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#if (DSP_TASK == 1)
#include <stdio.h>
#include "swlock.h"
#include <xcore/channel.h>
#include "math.h"
#include "dsp.h"

#include "dsp_op.h"
// #include "dsp_func.h"

#define QVAL        30

typedef struct {
    AUDIO_T bufin[NUM_USB_CHAN_IN][FRAME_SIZE];    //TODO:assume whole pipeline running on NUM_USB_CHAN_OUT channels
    int buf_new;
} pipeline_stage_t;

pipeline_stage_t pipeline[PL_STAGE_MAX];
swlock_t lockin[PL_STAGE_MAX] = {SWLOCK_INITIAL_VALUE};

//Double buffers to reduce time spent on moving data via channel
AUDIO_T usb_to_dsp_buf[2][NUM_USB_CHAN_OUT][FRAME_SIZE];   //8 ch
AUDIO_T dsp_to_usb_buf[2][NUM_USB_CHAN_IN][FRAME_SIZE];    //2 ch

int32_t qGainM3dB;

void init(void)
{
    qGainM3dB = Q(QVAL)(pow(10, -3/20.0));

    for (int i = 0; i < PL_STAGE_MAX; i++) {
        pipeline[i].buf_new  = 0;  //input buffer is empty
    }
}

void write_frame(int stage_id, AUDIO_T buf[NUM_USB_CHAN_IN][FRAME_SIZE])
{
    int next_idx = stage_id+1;
    if (next_idx == PL_STAGE_MAX) {
        next_idx = 0; //last stage, write back to audiohub
    }
    while (1) {
        swlock_acquire(&lockin[next_idx]);
        if (pipeline[next_idx].buf_new) {
            //next stage didn't read last frame, wait
            swlock_release(&lockin[next_idx]);
            continue;
        } else {
            //next stage in buf is empty, write to it
            //printf("T%d writing to\n", next_idx);
            for (int ch = 0; ch < NUM_USB_CHAN_IN; ch++) {
                for (int i = 0; i < FRAME_SIZE; i++) {  // TODO: Should use xs3_memcpy() for better efficiency
                    pipeline[next_idx].bufin[ch][i] = buf[ch][i];
                }
            }
            pipeline[next_idx].buf_new = 1;     //new frame data is available
            swlock_release(&lockin[next_idx]);
            return;
        }
    }
}

void read_frame(int stage_id, AUDIO_T buf[NUM_USB_CHAN_IN][FRAME_SIZE])
{
    while(1) {
        swlock_acquire(&lockin[stage_id]);
        if (pipeline[stage_id].buf_new) {
            //upstream stage has provided a new frame
            for (int ch = 0; ch < NUM_USB_CHAN_IN; ch++) {   // TODO: This is more readible but should use memcpy_xs3()
                for (int i = 0; i < FRAME_SIZE; i++) {
                    buf[ch][i] = pipeline[stage_id].bufin[ch][i];
                }
            }
            //printf("T%d reading\n", stage_id);
            pipeline[stage_id].buf_new = 0;   //mark it empty
            swlock_release(&lockin[stage_id]);
            return;
        } else {
            //upstream stage hasn't provided new frame
            swlock_release(&lockin[stage_id]);
            continue;
        }
    }
}

// This task exchanges audio data from AudioHub 
// Write first, read last
// buf former is buf[ch][FRAME_SIZE]
void dsp_stage0_task(chanend_t c_dsp)
{
    int stage_id=0;    //this is stage 0
    AUDIO_T from_usb_buf[NUM_USB_CHAN_OUT][FRAME_SIZE];  //NOTE: Adjust these base on DSP buffer arrangement
    AUDIO_T to_usb_buf[NUM_USB_CHAN_IN][FRAME_SIZE];
    AUDIO_T lo_buf[NUM_USB_CHAN_IN][FRAME_SIZE];         //NOTE: Adjust these base on DSP buffer arrangement
    int lo_bank;

    init();
    printf("stage0_task started\n");
    while (1)
    {
        lo_bank = chan_in_word(c_dsp);      //a frame is ready on bank lobank
        //frame from USB
        for (int ch = 0; ch < NUM_USB_CHAN_OUT; ch++) {
            for (int i = 0; i < FRAME_SIZE; i++) {
                from_usb_buf[ch][i] = usb_to_dsp_buf[lo_bank][ch][i];  //save a copy
            }
        }

        // NOTE: For demo, down mix 8 channels to 2 channels
        AUDIO_T *pFL = (AUDIO_T *)(from_usb_buf[0]);
        AUDIO_T *pFR = (AUDIO_T *)(from_usb_buf[1]);
        AUDIO_T *pC = (AUDIO_T *)(from_usb_buf[2]);
        AUDIO_T *pLfe = (AUDIO_T *)(from_usb_buf[3]);
        AUDIO_T *pBL = (AUDIO_T *)(from_usb_buf[4]);
        AUDIO_T *pBR = (AUDIO_T *)(from_usb_buf[5]);
        AUDIO_T *pSL = (AUDIO_T *)(from_usb_buf[6]);
        AUDIO_T *pSR = (AUDIO_T *)(from_usb_buf[7]);

        dsp_vector_muls(pC, qGainM3dB, pC, FRAME_SIZE, QVAL);
        dsp_vector_muls(pLfe, qGainM3dB, pLfe, FRAME_SIZE, QVAL);
        dsp_vector_muls(pBL, qGainM3dB, pBL, FRAME_SIZE, QVAL);
        dsp_vector_muls(pBR, qGainM3dB, pBR, FRAME_SIZE, QVAL);
        dsp_vector_muls(pSL, qGainM3dB, pSL, FRAME_SIZE, QVAL);
        dsp_vector_muls(pSR, qGainM3dB, pSR, FRAME_SIZE, QVAL);

        for (int i = 0; i < FRAME_SIZE; i++) {
            //mixing
            // lo_buf[0][i] = from_usb_buf[0][i]/2 + from_usb_buf[1][i]/2 + from_usb_buf[2][i]/2 + from_usb_buf[3][i]/2;  // TODO: should use long long to preserve dynamic range
            // lo_buf[1][i] = from_usb_buf[4][i]/2 + from_usb_buf[5][i]/2 + from_usb_buf[6][i]/2 + from_usb_buf[7][i]/2;
            lo_buf[0][i] = pFL[i] + pC[i] + pLfe[i] + pBL[i] + pSL[i];
            lo_buf[1][i] = pFR[i] + pC[i] + pLfe[i] + pBR[i] + pSR[i];
        }

        write_frame(stage_id, lo_buf);    //write to next stage's input buffer'

        read_frame(stage_id, to_usb_buf);       //read last frame

        //frame to USB
        for (int ch = 0; ch < NUM_USB_CHAN_IN; ch++) {
            for (int i = 0; i < FRAME_SIZE; i++) {
                dsp_to_usb_buf[lo_bank][ch][i] = to_usb_buf[ch][i];
            }
        }
    }
}

#if 0
void dsp_stage1_task(void) {
    int stage_id=1;
    int lo_buf[NUM_USB_CHAN_IN][FRAME_SIZE];

    printf("dsp_stage1_task started\n");
    while (1) {
        write_frame(stage_id, lo_buf);

        read_frame(stage_id, lo_buf);

        // TODO: add your process code here.
        // TODO: For demo, apply 6dB gain on both channels
        for (int ch = 0; ch < NUM_USB_CHAN_IN; ch++) {
            for (int i = 0; i < FRAME_SIZE; i++) {
                lo_buf[ch][i] *= 2;
            }
        }
    }
}

void dsp_stage2_task(void) {
    int stage_id=2;
    int input_buf[NUM_USB_CHAN_IN][FRAME_SIZE];
    int output_buf[NUM_USB_CHAN_IN][FRAME_SIZE];

    printf("dsp_stage2_task started\n");
    while (1) {
        write_frame(stage_id, output_buf);

        read_frame(stage_id, input_buf);

        // TODO: add your DSP code here.
        // NOTE: for demo, a 3kHz notch filter on ch0 and 8kHz notch on ch1
        do_filter_biquad_s32(&filter_notch_3khz, input_buf[0], output_buf[0]);
        do_filter_biquad_s32(&filter_notch_8khz, input_buf[1], output_buf[1]);
    }
}

void dsp_stage3_task(void) {
    int stage_id=3;
    int lo_buf[NUM_USB_CHAN_IN][FRAME_SIZE];

    printf("dsp_stage3_task started\n");
    while (1) {
        write_frame(stage_id, lo_buf);

        read_frame(stage_id, lo_buf);

        // TODO: add your DSP code here.
        // NOTE: for demo, amplify ch0 by 15dB and ch1 by 9dB
        do_amplify(lo_buf[0], Q28(5.6234), 28);
        do_amplify(lo_buf[1], Q28(2.818), 28);
    }
}

//define working variables for FFT, iFFT by stage4
int32_t DWORD_ALIGNED l_data[FRAME_SIZE];

void dsp_stage4_task(void) {
    int stage_id=4;
    int lo_buf[NUM_USB_CHAN_IN][FRAME_SIZE];

    printf("dsp_stage4_task started\n");
    while (1) {
        write_frame(stage_id, lo_buf);

        read_frame(stage_id, lo_buf);
        // TODO: add your DSP code here.
        // NOTEL for demo, this stage convert left channel to frequency domain and than back to time domain
        for (int i = 0; i < FRAME_SIZE; i++) {
            l_data[i] = lo_buf[0][i];
        }

        bfp_s32_t left;

        bfp_s32_init(&left,  l_data, -31, FRAME_SIZE, 1);

        bfp_complex_s32_t* L = bfp_fft_forward_mono(&left);     //fft

        bfp_fft_unpack_mono(L);    //unpack spectrum

        // convert it back to time domain
        bfp_fft_pack_mono(L);
        bfp_fft_inverse_mono(L);

        //copy processed data back to lo_buf for transfer to next stage
        for (int i = 0; i < FRAME_SIZE; i++) {
              lo_buf[0][i] = Q31(ldexp(left.data[i],  left.exp));
        }
    }
}

void dsp_stage5_task(void) {
    int stage_id=5;
    int lo_buf[NUM_USB_CHAN_IN][FRAME_SIZE];

    printf("dsp_stage5_task started\n");
    while (1) {
        write_frame(stage_id, lo_buf);

        read_frame(stage_id, lo_buf);

        // TODO: add your DSP code here.
    }
}
#endif

#endif  //DSP_tASK
