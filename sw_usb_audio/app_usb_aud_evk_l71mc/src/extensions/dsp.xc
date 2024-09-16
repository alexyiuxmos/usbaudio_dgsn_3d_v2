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

extern "C" {
    extern void dsp_task_in_c(int bank);
    extern int button_task_in_c(int button);
    extern int OnProcessing();
	extern void ConvolutionTaskInit();
    extern void ConvolutionTask(int taskIdx, chanend c_main_tile_to_sub_tile1);

    extern int audio_ex3d_init(uint32_t dwChannels, uint32_t dwSampleSize, uint32_t dwSRHz, uint32_t dwAudioDataSize);
    extern int audio_ex3d_conv_init(uint32_t dwTileNum, uint32_t dwChannels);
}

on tile[0]: in port p_buttons = XS1_PORT_8D;
#define BUTTON_PIN 0b00100000
on tile[0]: out port p_leds = XS1_PORT_4F;
#define LED_R 0b00000100

static unsigned char lastHidData;

#if 1
// Tile 0
void button_task(chanend c_button)
{
    int current_val = 0, last_val = 0;
    int is_stable = 0;
    int button_pressed = 0;
    timer tmr;
    const unsigned debounce_delay_ms = 200;
    unsigned debounce_timeout;
    int current_time;
    
    tmr :> current_time;
    debounce_timeout = current_time + (debounce_delay_ms * 10000/*XS1_TIMER_HZ*/);
    //p_buttons :> current_val;

    while (1) {
        select {
#if 0
            // if the button is stable, react when the i/o pin changes value
            case is_stable => p_buttons when pinsneq(current_val) :> current_val:
                if ((current_val & BUTTON_PIN) == BUTTON_PIN) {
                    //printf("Button released\n");
                    //printhex(current_val);
                    if (button_pressed == 1) {
                        button_pressed = 0; //button is released
                        c_button <: 1; //((current_val >> 5) & 0x01);
                    }                    
                } else {
                    //printf("Button pressed\n");
                    button_pressed = 1;
                    //button pressed
                    //button_response();
                    //c_button <: g_Ex3dSfIdx;
                }
                is_stable = 0;
                int current_time;
                tmr :> current_time;
                // calculate time to event after debounce period
                debounce_timeout = current_time + (debounce_delay_ms * 10000/*XS1_TIMER_HZ*/);
                break;
            
            case !is_stable => tmr when timerafter(debounce_timeout) :> void:
                is_stable = 1;
                break;
#endif            
            case tmr when timerafter(debounce_timeout) :> void:
                p_buttons :> current_val;
                current_val = current_val & BUTTON_PIN;
                if (current_val != last_val) { // pin changed
                    if ((current_val & BUTTON_PIN) == BUTTON_PIN) {
                        //printf("Button released\n");
                        //printhex(current_val);
                        if (button_pressed == 1) {
                            button_pressed = 0; //button is released
                            c_button <: 1; //((current_val >> 5) & 0x01);
                        }
                    } else {
                        //printf("Button pressed\n");
                        button_pressed = 1;
                        //button pressed
                        //button_response();
                        //c_button <: g_Ex3dSfIdx;
                    }
                }
                //int current_time;
                tmr :> current_time;
                debounce_timeout = current_time + (debounce_delay_ms * 10000/*XS1_TIMER_HZ*/);
                last_val = current_val;
                break;
        }
    }
}
#else
void button_task(chanend c_button)
{
    int button = 0;
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
            c_button <: ((button >> 5) & 0x01);

            tmr :> start_time;
        }
    }
}
#endif

void led_task(chanend c_led)
{
    int led = 0;
    static int led_status = 0; //LED_R;

    audio_ex3d_conv_init(1, NUM_USB_CHAN_OUT);  // convolution_task_sub_tile1 �� ���� tile���� ����

    while(1) {
        c_led :> led;
        //printf("led: ( %x )\n", led);
        //led = (led << 2) ^ LED_R; //~((led << 2) & 0x0c);
        led_status = led_status ^ LED_R;
        p_leds <: (led_status);
    }
}

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
void ex3d_task(chanend c_ex3d_started)
{
    set_core_high_priority_on();
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

//task to call C
void dsp_task(chanend c_dsp, chanend c_button, chanend c_led, chanend c_ex3d_started)
{
    int bank, button, led_status;
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
            dsp_task_in_c(bank);   //do signal processing in C

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
            led_status = button_task_in_c(button);
            c_led <: (led_status & 0x03);
            break;
        }
    }
}

#endif      //USE_EX3D
