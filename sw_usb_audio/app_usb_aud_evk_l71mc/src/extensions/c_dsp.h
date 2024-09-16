#if (USE_EX3D == 1)

#include <stdint.h>

#define AUDIO_T_16

#if defined(AUDIO_T_16)
typedef int16_t AUDIO_T;
#else // defined(AUDIO_T_32)
typedef int32_t AUDIO_T;
#endif

#define FRAME_SIZE          512//256    //128 samples / 48kHz = 2.6ms

#endif
