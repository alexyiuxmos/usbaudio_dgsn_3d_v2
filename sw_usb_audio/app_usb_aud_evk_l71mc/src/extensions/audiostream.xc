#include <platform.h>

#if !defined(USE_EX3D)
on tile[0]: out port p_leds = XS1_PORT_4F;
#endif

void UserAudioStreamStart(void)
{
    /* Turn all LEDs on */
#if !defined(USE_EX3D)
    p_leds <: 0xF;
#endif
}

void UserAudioStreamStop(void)
{
    /* Turn all LEDs off */
#if !defined(USE_EX3D)
    p_leds <: 0x0;
#endif
}

