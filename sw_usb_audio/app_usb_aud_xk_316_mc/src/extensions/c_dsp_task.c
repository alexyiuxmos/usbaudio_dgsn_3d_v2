#if (USE_EX3D == 1)

#include <stdio.h>
#include <xua_conf.h>
#include "debug_print.h"
#include "c_dsp.h"
#include "audio_ex3d_control.h"
#include "math.h"
#include "dsp.h"

#include "xcore/channel.h"
#include "xcore/chanend.h"

// Double buffers to reduce time spent on moving data via channel
AUDIO_T usb_to_dsp_buf[2][NUM_USB_CHAN_OUT][FRAME_SIZE];
AUDIO_T dsp_to_usb_buf[2][NUM_USB_CHAN_IN][FRAME_SIZE];

extern uint8_t bEX3D_On;                // EX3D On 유무 저장
extern uint8_t EX3D_SF_Idx;
uint8_t g_Ex3dSfIdx = 0;
uint8_t g_ButtonCount = 1;

#ifdef FLASH_READ
extern unsigned char exir2k_xmos_game_wm_posData_v090h000[SF_SIZE_PER_ANGLE];
extern unsigned char exir2k_xmos_game_wm_posData_v090h045[SF_SIZE_PER_ANGLE];
extern unsigned char exir2k_xmos_game_wm_posData_v090h090[SF_SIZE_PER_ANGLE];
extern unsigned char exir2k_xmos_game_wm_posData_v090h135[SF_SIZE_PER_ANGLE];
extern unsigned char exir2k_xmos_game_wm_posData_v090h180[SF_SIZE_PER_ANGLE];
extern unsigned char exir2k_xmos_game_wm_posData_v090h225[SF_SIZE_PER_ANGLE];
extern unsigned char exir2k_xmos_game_wm_posData_v090h270[SF_SIZE_PER_ANGLE];
extern unsigned char exir2k_xmos_game_wm_posData_v090h315[SF_SIZE_PER_ANGLE];
extern unsigned char exir2k_xmos_game_wm_posData_lfe[SF_SIZE_PER_ANGLE];
#else
extern const unsigned char exir2k_xmos_game_wm_posData_v090h000[SF_SIZE_PER_ANGLE];
extern const unsigned char exir2k_xmos_game_wm_posData_v090h045[SF_SIZE_PER_ANGLE];
extern const unsigned char exir2k_xmos_game_wm_posData_v090h090[SF_SIZE_PER_ANGLE];
extern const unsigned char exir2k_xmos_game_wm_posData_v090h135[SF_SIZE_PER_ANGLE];
extern const unsigned char exir2k_xmos_game_wm_posData_v090h180[SF_SIZE_PER_ANGLE];
extern const unsigned char exir2k_xmos_game_wm_posData_v090h225[SF_SIZE_PER_ANGLE];
extern const unsigned char exir2k_xmos_game_wm_posData_v090h270[SF_SIZE_PER_ANGLE];
extern const unsigned char exir2k_xmos_game_wm_posData_v090h315[SF_SIZE_PER_ANGLE];
extern const unsigned char exir2k_xmos_game_wm_posData_lfe[SF_SIZE_PER_ANGLE];
#endif

void send_soundField_to_tile1(chanend_t c_copy2tile0)
{
#ifdef FLASH_READ
    chan_out_word(c_copy2tile0, 1); // sync
    for (int i = 0; i< SF_SIZE_PER_ANGLE; i++) {       
        chanend_out_word(c_copy2tile0, exir2k_xmos_game_wm_posData_lfe[i]);
    }
    return;
#else
    return;
#endif
}
void get_soundField_000(chanend_t c_copy_from_tile1)
{
#ifdef FLASH_READ
    uint32_t tmp;
    int i;
    tmp = chan_in_word(c_copy_from_tile1);
    for (i = 0; i< SF_SIZE_PER_ANGLE; i++) {
        exir2k_xmos_game_wm_posData_v090h000[i] = chanend_in_word(c_copy_from_tile1);
    }
    //printhex(exir2k_xmos_game_wm_posData_v090h000[0x30]);
#endif
}
void get_soundField_045(chanend_t c_copy_from_tile1)
{
#ifdef FLASH_READ
    uint32_t tmp;
    int i;    
    tmp = chan_in_word(c_copy_from_tile1);
    for (i = 0; i< SF_SIZE_PER_ANGLE; i++) {
        exir2k_xmos_game_wm_posData_v090h045[i] = chanend_in_word(c_copy_from_tile1);
    }
    //printhex(exir2k_xmos_game_wm_posData_v090h045[0x60]);
#endif
}
void get_soundField_090(chanend_t c_copy_from_tile1)
{
#ifdef FLASH_READ
    uint32_t tmp;
    int i;
    tmp = chan_in_word(c_copy_from_tile1);
    for (i = 0; i< SF_SIZE_PER_ANGLE; i++) {
        exir2k_xmos_game_wm_posData_v090h090[i] = chanend_in_word(c_copy_from_tile1);
    }
    //printhex(exir2k_xmos_game_wm_posData_v090h090[0x98]);
#endif
}
void get_soundField_135(chanend_t c_copy_from_tile1)
{
#ifdef FLASH_READ
    uint32_t tmp;
    int i;
    tmp = chan_in_word(c_copy_from_tile1);
    for (i = 0; i< SF_SIZE_PER_ANGLE; i++) {
        exir2k_xmos_game_wm_posData_v090h135[i] = chanend_in_word(c_copy_from_tile1);
    }
    //printhex(exir2k_xmos_game_wm_posData_v090h135[0xA0]);
#endif
}

void get_soundField_from_tile0(chanend_t c_copy_from_tile1)
{
    uint32_t tmp;
    int i;
#ifdef FLASH_READ
#if 1
    tmp = chan_in_word(c_copy_from_tile1);
    for (i = 0; i< SF_SIZE_PER_ANGLE; i++) {
        exir2k_xmos_game_wm_posData_v090h000[i] = chanend_in_word(c_copy_from_tile1);
    }
    
    tmp = chan_in_word(c_copy_from_tile1);
    for (i = 0; i< SF_SIZE_PER_ANGLE; i++) {
        exir2k_xmos_game_wm_posData_v090h045[i] = chanend_in_word(c_copy_from_tile1);
    }
    
    tmp = chan_in_word(c_copy_from_tile1);
    for (i = 0; i< SF_SIZE_PER_ANGLE; i++) {
        exir2k_xmos_game_wm_posData_v090h090[i] = chanend_in_word(c_copy_from_tile1);
    }
    
    tmp = chan_in_word(c_copy_from_tile1);
    for (i = 0; i< SF_SIZE_PER_ANGLE; i++) {
        exir2k_xmos_game_wm_posData_v090h135[i] = chanend_in_word(c_copy_from_tile1);
    }
#endif

#else
    return;
#endif
}
void UserBufferManagementInit(unsigned sampFreq)
{
    debug_printf("audio_ex3d_change_parameter()++, ch:%d, size:%d, samfreq:%d\n\r", NUM_USB_CHAN_OUT, NUM_USB_CHAN_OUT * 2 * FRAME_SIZE, sampFreq);
    if(audio_ex3d_change_parameter(NUM_USB_CHAN_OUT, NUM_USB_CHAN_OUT * FRAME_SIZE * sizeof(AUDIO_T), sampFreq, sizeof(AUDIO_T)) == NO_ERR) {
        debug_printf("audio_ex3d_change_parameter() success, \n\r");
    }
}

int button_task_in_c(int button, chanend_t c_chg_sf)
{

    audio_ex3d_set_sf(button);       //set sound field
#if 0
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
#endif
}

void dsp_task_in_c(int bank, int sf_changed)
{
#if 1
    //Use audio data frame here
    if (sf_changed == 0) {
        EX3DAudio_ProcessAudioData((PBYTE)&usb_to_dsp_buf[bank][0][0], (PBYTE)&dsp_to_usb_buf[bank][0][0], NUM_USB_CHAN_OUT * FRAME_SIZE * sizeof(AUDIO_T), 0);        
    } else {
        for (int i = 0; i < FRAME_SIZE; i++) {
            dsp_to_usb_buf[bank][0][i] = 0;
            dsp_to_usb_buf[bank][1][i] = 0;
        }
    }
        
    //audio_ex3d_task();
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
