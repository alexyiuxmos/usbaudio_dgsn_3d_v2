#if (USE_EX3D == 1)

#include <stdio.h>
#include <xua_conf.h>
#include "debug_print.h"
#include "c_dsp.h"
#include "audio_ex3d_control.h"
#include "math.h"
#include "dsp.h"

// Double buffers to reduce time spent on moving data via channel
AUDIO_T usb_to_dsp_buf[2][NUM_USB_CHAN_OUT][FRAME_SIZE];
AUDIO_T dsp_to_usb_buf[2][NUM_USB_CHAN_IN][FRAME_SIZE];

extern uint8_t bEX3D_On;                // EX3D On 유무 저장
uint8_t g_Ex3dSfIdx = 0;

void UserBufferManagementInit(unsigned sampFreq)
{
    debug_printf("audio_ex3d_change_parameter()++, ch:%d, size:%d, samfreq:%d\n\r", NUM_USB_CHAN_OUT, NUM_USB_CHAN_OUT * 2 * FRAME_SIZE, sampFreq);
    if(audio_ex3d_change_parameter(NUM_USB_CHAN_OUT, NUM_USB_CHAN_OUT * FRAME_SIZE * sizeof(AUDIO_T), sampFreq, sizeof(AUDIO_T)) == NO_ERR) {
        debug_printf("audio_ex3d_change_parameter() success, \n\r");
    }
}

int button_task_in_c(int button)
{
    int led = 0;

    if(button != 7) debug_printf("button:%d\n\r", button);
    switch(button) {
    case 3:
        led = 0x08;
        break;
    case 5:
        if(g_Ex3dSfIdx >= 2) g_Ex3dSfIdx = 0;
        else g_Ex3dSfIdx++;
        audio_ex3d_set_sf(g_Ex3dSfIdx);
        break;
    case 6:
        audio_ex3d_toggle();
        break;
    default:
        break;
    }

    led |= (bEX3D_On) | (((g_Ex3dSfIdx + 1) << 1) & 0x06);

    return led;
}

void dsp_task_in_c(int bank)
{
#if 1
    //Use audio data frame here
    EX3DAudio_ProcessAudioData((PBYTE)&usb_to_dsp_buf[bank][0][0], (PBYTE)&dsp_to_usb_buf[bank][0][0], NUM_USB_CHAN_OUT * FRAME_SIZE * sizeof(AUDIO_T), 0);

    audio_ex3d_task();
#else
    for (int i = 0; i < FRAME_SIZE; i++) {
        dsp_to_usb_buf[bank][0][i] = usb_to_dsp_buf[bank][0][i];
        dsp_to_usb_buf[bank][1][i] = usb_to_dsp_buf[bank][1][i];
        for(int ch=2; ch < NUM_USB_CHAN_OUT; ch+=2){
            dsp_to_usb_buf[bank][0][i] += usb_to_dsp_buf[bank][ch][i];
            dsp_to_usb_buf[bank][1][i] += usb_to_dsp_buf[bank][ch+1][i];
        }
    }
#endif
}

#endif
