#include <platform.h>

#if !defined(USE_EX3D)
on tile[0]: out port p_leds1 = XS1_PORT_4F;
#endif

int g_audio_stream_started = 0xF;
void UserAudioStreamStart(void)
{
    if (g_audio_stream_started) g_audio_stream_started--;    
    /* Turn all LEDs on */
#if !defined(USE_EX3D)
    p_leds1 <: 0xF;
#endif
}

void UserAudioStreamStop(void)
{
    /* Turn all LEDs off */
#if !defined(USE_EX3D)
    p_leds1 <: 0x0;
#endif
}

