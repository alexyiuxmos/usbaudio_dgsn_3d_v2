#if (USE_EX3D == 1)

#include <stdint.h>

#define AUDIO_T_16

#if defined(AUDIO_T_16)
typedef int16_t AUDIO_T;
#else // defined(AUDIO_T_32)
typedef int32_t AUDIO_T;
#endif

#define FRAME_SIZE          512    //128 samples / 48kHz = 2.6ms

#define SF_SIZE_PER_ANGLE   8236 //2048 sound field data size per angle
#define SF_SIZE_TOTAL       (SF_SIZE_PER_ANGLE * 9) // Total size for 1 sound field data
#endif
