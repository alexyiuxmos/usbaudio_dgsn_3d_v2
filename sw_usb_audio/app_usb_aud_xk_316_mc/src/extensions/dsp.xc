// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#if (USE_EX3D == 1)

#include <stdio.h>
#include <xs1.h>
#include <xua_conf.h>
#include "user_hid.h"
#include "xua_hid_report.h"
#include "c_dsp.h"
#include "print.h"

#include <quadflashlib.h>

#define MEASURE_ELAPSED_TIME

/* Write HID Report Data into hidData array
 *
 * Bits are as follows:
 * 0: Play/Pause
 * 1: Scan Next Track
 * 2: Scan Prev Track
 * 3: Volume Up
 * 4: Volume Down
 * 5: Mute
 */

#define HID_CONTROL_PLAYPAUSE   0x01
#define HID_CONTROL_NEXT        0x02
#define HID_CONTROL_PREV        0x04
#define HID_CONTROL_VOLUP       0x08
#define HID_CONTROL_VOLDN       0x10
#define HID_CONTROL_MUTE        0x20

//offset position for different angles
#define OFFSET_V090H000 0
#define OFFSET_V090H045 OFFSET_V090H000+SF_SIZE_PER_ANGLE
#define OFFSET_V090H090 OFFSET_V090H045+SF_SIZE_PER_ANGLE
#define OFFSET_V090H135 OFFSET_V090H090+SF_SIZE_PER_ANGLE
#define OFFSET_V090H180 OFFSET_V090H135+SF_SIZE_PER_ANGLE
#define OFFSET_V090H225 OFFSET_V090H180+SF_SIZE_PER_ANGLE
#define OFFSET_V090H270 OFFSET_V090H225+SF_SIZE_PER_ANGLE
#define OFFSET_V090H315 OFFSET_V090H270+SF_SIZE_PER_ANGLE
#define OFFSET_LFE OFFSET_V090H315+SF_SIZE_PER_ANGLE

extern "C" {
    extern int g_audio_stream_started;

    extern void dsp_task_in_c(int bank, int sf_changed);
    extern int button_task_in_c(int button, chanend c_x_tile);
    extern int OnProcessing();
	extern void ConvolutionTaskInit();
    extern void ConvolutionTask(int taskIdx, chanend c_main_tile_to_sub_tile1);

    extern int audio_ex3d_init(uint32_t dwChannels, uint32_t dwSampleSize, uint32_t dwSRHz, uint32_t dwAudioDataSize);
    extern int audio_ex3d_conv_init(uint32_t dwTileNum, uint32_t dwChannels);
    extern int audio_ex3d_conv_setSF(uint32_t dwTileNum, uint32_t dwChannels, uint32_t sf_idx);
    void audio_ex3d_task(void);
    extern void send_soundField_to_tile1(chanend c_copy2tile0);
    extern void get_soundField_from_tile0(chanend c_copy_from_tile1);
    extern void get_soundField_000(chanend c_copy_from_tile1);
    extern void get_soundField_045(chanend c_copy_from_tile1);
    extern void get_soundField_090(chanend c_copy_from_tile1);
    extern void get_soundField_135(chanend c_copy_from_tile1);
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
}

on tile[0]: in port p_buttons = XS1_PORT_4E;
#define BUTTON_PIN 0b00000001
//on tile[0]: out port p_leds = XS1_PORT_4F;
//#define LED_R 0 //2
//#define LED_G 3
//#define LED_MASK 0b00000011

on tile[0]: fl_QSPIPorts p_qspi =
{
    PORT_SQI_CS,
    PORT_SQI_SCLK,
    PORT_SQI_SIO,
    XS1_CLKBLK_1
};
static const fl_QuadDeviceSpec deviceSpec =
{
    0,                      /* unknown flash id */
    256,                    /* page size */
    16384, //32768,                  /* num pages */
    3,                      /* address size */
    2,                      /* log2 clock divider */
    0x9F,                   /* QSPI_RDID */
    0,                      /* id dummy bytes */
    3,                      /* id size in bytes */
    0xEF4016, //0xEF6017, //0xBA6016,               /* device id */
    0x20,                   /* QSPI_SE */
    4096,                   /* Sector erase is always 4KB */
    0x06,                   /* QSPI_WREN */
    0x04,                   /* QSPI_WRDI */
    PROT_TYPE_SR,           /* no protection */
    {{0x7C,0x00},{0,0}},    /* QSPI_SP, QSPI_SU */
    0x02,                   /* QSPI_PP */
    0xEB,                   /* QSPI_READ_FAST */
    1,                      /* 1 read dummy byte */
    SECTOR_LAYOUT_REGULAR,  /* mad sectors */
    {4096,{0,{0}}},         /* regular sector sizes */
    0x05,                   /* QSPI_RDSR */
    0x01,                   /* QSPI_WRSR */
    0x01,                   /* QSPI_WIP_BIT_MASK */    
};
static unsigned char lastHidData;

#if 1
// Tile 0

typedef enum {
    idle = 0,
    reconnect = 1,
    read_h000 = 2,
    read_h045 = 3,
    read_h090 = 4,
    read_h135 = 5,
    read_h180 = 6,
    read_h225 = 7,
    read_h270 = 8,
    read_h315 = 9,
    read_lfe = 10,
    read_disconnect = 11,
    load_SF_on_t0 = 12,
    load_SF_on_t1 = 13,
    total_states
} e_flash_read_state;

e_flash_read_state g_fread_state;

// 10ms polling interval
#define FLASH_READ_POLLING_PERIOD (10 * 100000/*XS1_TIMER_HZ*/)
void flash_read_task(chanend c_x_tile, chanend c_flash_rd_req, chanend c_chg_sf)
{
#ifdef FLASH_READ
    timer tmr;
    const unsigned time_poll = 5;
    unsigned timeout;
    int current_time;
    uint32_t offset = 0;
    uint32_t sf_idx = 0;
    uint32_t start_time, stop_time;

    // connect to flash device
    printstrln("Connecting to flash device...");
    if (fl_connectToDevice(p_qspi, &deviceSpec, 1) != 0) {
        printstrln("fl_connectToDevice failed");
        return;
    }
    // debug;; tile 0
    fl_readData(OFFSET_V090H180+offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_v090h180);
    fl_readData(OFFSET_V090H225+offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_v090h225);
    fl_readData(OFFSET_V090H270+offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_v090h270);
    fl_readData(OFFSET_V090H315+offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_v090h315);
    audio_ex3d_conv_init(1, NUM_USB_CHAN_OUT);  // convolution_task_sub_tile1 �� ���� tile���� ����

    fl_readData(OFFSET_V090H000+offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_lfe);
    send_soundField_to_tile1(c_x_tile);
    fl_readData(OFFSET_V090H045+offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_lfe);
    send_soundField_to_tile1(c_x_tile);
    fl_readData(OFFSET_V090H090+offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_lfe);
    send_soundField_to_tile1(c_x_tile);
    fl_readData(OFFSET_V090H135+offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_lfe);
    send_soundField_to_tile1(c_x_tile);
    
    //g_fread_state = read_h180;
    g_fread_state = idle;

    tmr :> current_time;
    timeout = current_time + (FLASH_READ_POLLING_PERIOD);

    while (1) {
        select {
            case c_flash_rd_req :> int tmp:
                sf_idx = tmp;
//                if (g_fread_state == idle) {
                    offset = tmp * SF_SIZE_TOTAL;   // offset position for each sound field                        
//                    g_fread_state = load_SF_on_t0;
//                }

                        fl_readData(OFFSET_V090H000 + offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_lfe);
                        send_soundField_to_tile1(c_chg_sf);
                        
                        fl_readData(OFFSET_V090H045 + offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_lfe);
                        send_soundField_to_tile1(c_chg_sf);                        
                        
                        fl_readData(OFFSET_V090H090+ offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_lfe);
                        send_soundField_to_tile1(c_chg_sf);
                                                
                        fl_readData(OFFSET_V090H135 + offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_lfe);
                        send_soundField_to_tile1(c_chg_sf);
                        
                        fl_readData(OFFSET_V090H180 + offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_v090h180);
                        fl_readData(OFFSET_V090H225 + offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_v090h225);
                        fl_readData(OFFSET_V090H270 + offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_v090h270);
                        fl_readData(OFFSET_V090H315 + offset, SF_SIZE_PER_ANGLE, exir2k_xmos_game_wm_posData_v090h315);
                        //tmr :> start_time;
                        audio_ex3d_conv_setSF(1, NUM_USB_CHAN_OUT, sf_idx);
                break;  // case c_flash_read_req
        }
//        tmr :> current_time;
//        timeout = current_time + (FLASH_READ_POLLING_PERIOD);
    }
#else
    return;
#endif
}

typedef enum {
    sf_game = 0,
    sf_music = 1,
    sf_movie = 2,
    sf_total_states
} e_sf_status;

void button_task(chanend c_button)
{
    int current_val = 0, last_val = 0;
    int button_pressed = 0;
    e_sf_status status = sf_game;
    timer tmr;
    const unsigned debounce_delay_ms = 20;
    unsigned debounce_timeout;
    int current_time;
    uint32_t tmp = 0;
    
    //audio_ex3d_conv_init(1, NUM_USB_CHAN_OUT);  // convolution_task_sub_tile1 �� ���� tile���� ����
    
    //sf_game mode on
//    p_leds <: ( ((status << LED_R)) & LED_MASK );
    tmr :> current_time;
    debounce_timeout = current_time + (debounce_delay_ms * 10000/*XS1_TIMER_HZ*/);
    //p_buttons :> current_val;

    while (1) {
        select {        
            case tmr when timerafter(debounce_timeout) :> void:
//                if (g_audio_stream_started) {
                p_buttons :> current_val;
                current_val = current_val & BUTTON_PIN;
                if (current_val != last_val) { // pin changed
                    if ((current_val & BUTTON_PIN) == BUTTON_PIN) {
                        if (button_pressed == 1) {
                            button_pressed = 0; //button is released
                            
                            status++;
                            if (status >= sf_total_states) status = sf_game;
                            
                            c_button <: status; //((current_val >> 5) & 0x01);

                            //read sound field
                            //c_flash_rd_req <: status;
//                            p_leds <: ( ((status << LED_R)) & LED_MASK );
                        }
                    } else {
                        button_pressed = 1;
                    }
                }
                tmr :> current_time;
                debounce_timeout = current_time + (debounce_delay_ms * 100000/*XS1_TIMER_HZ*/);
                last_val = current_val;
//                }
                break;
        }
    }
}
#else
void button_task(chanend c_button)
{
    int button = 0;
    unsigned char hidData = 0;
    unsigned start_time, end_time, elapsed_time;
    timer tmr;

    tmr :> start_time;
    p_buttons :> button;
    while(1) {
        tmr :> end_time;
        if(end_time > start_time) elapsed_time = end_time - start_time;
        else elapsed_time = 0xffffffff - start_time + end_time + 1;

        if(elapsed_time > 2000000) {
            p_buttons when pinsneq(button) :> button;
            c_button <: (button & 0x07);
            // printf("button:0x%02x", button);

            tmr :> start_time;

            if(button == 0x03) {
                if (hidIsChangePending(0))
                    continue;

                hidData = HID_CONTROL_PLAYPAUSE;

                unsafe {
                    volatile unsigned char * unsafe lastHidDataUnsafe = &lastHidData;
                    *lastHidDataUnsafe = hidData;
                    hidSetChangePending(0);
                }
            }
        }
    }
}

void led_task(chanend c_led)
{
    int led = 0;
    int last_led = 0;

    audio_ex3d_conv_init(1, NUM_USB_CHAN_OUT);  // convolution_task_sub_tile1 �� ���� tile���� ����

    while(1) {
        delay_milliseconds(20);
        c_led :> led;
        led = (led & 0x0f);
        if(last_led != led) {
            last_led = led;
            p_leds <: led;
        }
    }
}
#endif
size_t UserHIDGetData( const unsigned id, unsigned char hidData[ HID_MAX_DATA_BYTES ])
{
    // There is only one report, so the id parameter is ignored

    hidData[0] = lastHidData;

    // One byte of data is always returned
    return 1;
}

void UserHIDInit( void )
{
}

#if defined(USE_OS)
void ex3d_task(chanend c_ex3d_started, chanend c_x_tile)
{
    set_core_high_priority_on();
    
    get_soundField_from_tile0(c_x_tile);

    if(audio_ex3d_init(NUM_USB_CHAN_OUT, NUM_USB_CHAN_OUT * FRAME_SIZE * sizeof(AUDIO_T), 48000, sizeof(AUDIO_T)) == 0) {
        printf("audio_ex3d_init() success\n\r");
    }

    printf("ex3d_task\n");
    c_ex3d_started <: 1;
    while (1) {
        printf("OnProcessing\n");
        OnProcessing();
    }
}

void convolution_task_sub_tile1(chanend c_main_tile_to_sub_tile1)
{
    set_core_high_priority_on();
	ConvolutionTaskInit();

    while (1) {
        printf("sub1 ConvolutionTask(0)\n");
        ConvolutionTask(0, c_main_tile_to_sub_tile1);
    }
}

void convolution_task_main_tile(chanend c_main_tile_to_sub_tile1)
{
    set_core_high_priority_on();
	ConvolutionTaskInit();

    while (1) {
        printf("main ConvolutionTask(0)\n");
        ConvolutionTask(0, c_main_tile_to_sub_tile1);
    }
}
#endif

void load_SF_task(chanend c_load_SF, chanend c_chg_sf)
{
    uint32_t sf_idx = 0;
    set_core_high_priority_on();
    while (1) {
        select {
            case c_load_SF :> sf_idx:
                get_soundField_000(c_chg_sf);
                get_soundField_045(c_chg_sf);
                get_soundField_090(c_chg_sf);
                get_soundField_135(c_chg_sf);
                button_task_in_c(sf_idx, c_chg_sf);
                audio_ex3d_task();
                break;
            
            default:
                break;
        }
    }
}
//task to call C
void dsp_task(chanend c_dsp, chanend c_button, chanend c_x_tile, chanend c_ex3d_started, chanend c_load_SF/*c_chg_sf*/, chanend c_flash_rd_req)
{
    int bank, button, led_status, sf_changed = 0, load_SF_status = 0, sf_idx = 0;
    int sync_t0 = 0;
//    timer tmr;
//    uint32_t start_time, end_time, elapsed_time;
#if defined(MEASURE_ELAPSED_TIME) && !defined(USE_OS)
    // int time[10];

#define MEAN_CNT    1000

    static uint32_t last_time = 0;
    static uint32_t cnt = 0;
    static uint32_t sum_elapsed_time = 0;
    static uint32_t max_elapsed_time = 0;
    static uint32_t min_elapsed_time = 0xffffffff;
    static uint32_t sum_event_time = 0;
    static uint32_t skip = 0;

    timer tmr;
    uint32_t start_time, end_time, overhead_time, elapsed_time, event_time;
    tmr :> start_time;
    tmr :> end_time;
    overhead_time = end_time - start_time;
#endif

    set_core_high_priority_on();

    uint32_t tmp;
    c_ex3d_started :> tmp;
    printf("dsp_task started\n");
    while (1) {
        select {
        case c_dsp :> bank:        //when a frame is received

#if defined(MEASURE_ELAPSED_TIME) && !defined(USE_OS)
            tmr :> start_time;
#endif
            dsp_task_in_c(bank, sf_changed);   //do signal processing in C
            if (sf_changed) {
                sync_t0++;
                if (sync_t0 > 100) {
                    sync_t0 = 0;
                    sf_changed = 0;
                }
            }
#if defined(MEASURE_ELAPSED_TIME) && !defined(USE_OS)
            tmr :> end_time;

            if(skip == 0) {
                elapsed_time = end_time-start_time-overhead_time;
                if(max_elapsed_time < elapsed_time) max_elapsed_time = elapsed_time;
                if(min_elapsed_time > elapsed_time) min_elapsed_time = elapsed_time;
                sum_elapsed_time += elapsed_time;

                event_time = start_time - last_time;
                sum_event_time += event_time;

                // if(cnt < 8) time[cnt] = elapsed_time;
                cnt++;
            } else {
                skip = 0;
            }

            if(cnt == MEAN_CNT) {
                // for(int i=0; i<8; i++) printf("%d-%d ", i, time[i]);
                printf("event: %d, elapsed-avr: %d, min: %d, max: %d\n",
                    sum_event_time / MEAN_CNT,
                    sum_elapsed_time / MEAN_CNT, min_elapsed_time, max_elapsed_time );
                sum_elapsed_time = 0;
                max_elapsed_time = 0;
                min_elapsed_time = 0xffffffff;
                sum_event_time = 0;
                skip = 1;
                cnt = 0;
            }
            last_time = start_time;
#endif
            break;

        case c_button :> button:
            //led_status = button_task_in_c(button);
            //c_led <: (led_status & 0x0f);
            if (sf_changed == 0) {
                sf_idx = button;
                c_flash_rd_req <: sf_idx;
                sf_changed = 1;
                c_load_SF <: button; //sf_idx;
            }
            break;
        }
    }
}

#endif      //USE_EX3D
