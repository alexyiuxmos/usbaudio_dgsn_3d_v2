#ifndef AUDIO_EX3D_CONTROL_H
#define AUDIO_EX3D_CONTROL_H

#include "EX3DAudio.h"

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_AUDIO_EX3D_CONTROL_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

#define EX3D_SET_CH_NUM             8

EXTERN int audio_ex3d_init(uint32_t dwChannels, uint32_t dwSampleSize, uint32_t dwSRHz, uint32_t dwAudioDataSize);
EXTERN int audio_ex3d_conv_init(uint32_t dwTileNum, uint32_t dwChannels);
EXTERN int audio_ex3d_deinit(void);
EXTERN int audio_ex3d_change_parameter(uint32_t dwChannels, uint32_t dwSampleSize, uint32_t dwSRHz, uint32_t dwAudioDataSize);

EXTERN void audio_ex3d_set_onoff(DS_BOOL onoff);
EXTERN void audio_ex3d_set_sf(uint32_t SF_Idx);
EXTERN void audio_ex3d_set_angle(uint32_t dwCh1Angle, uint32_t dwCh2Angle,
                          uint32_t dwCh3Angle, uint32_t dwCh4Angle,
                          uint32_t dwCh5Angle, uint32_t dwCh6Angle,
                          uint32_t dwCh7Angle, uint32_t dwCh8Angle);
EXTERN void audio_ex3d_set_upmix(DS_BOOL onoff);

EXTERN void audio_ex3d_upmix_toggle(void);
EXTERN void audio_ex3d_toggle(void);
EXTERN void audio_ex3d_test_cancel(void);
EXTERN void audio_ex3d_step_angle(void);
EXTERN void audio_ex3d_rotate_mono_angle_wait(uint32_t msWaitTime);
EXTERN void audio_ex3d_rotate_mono_angle(void);
EXTERN void audio_ex3d_rotate_all_angle_wait(uint32_t msWaitTime);
EXTERN void audio_ex3d_rotate_all_angle(void);
EXTERN void audio_ex3d_rotate_sf(void);
EXTERN void audio_ex3d_enable_rotate_angle(DS_BOOL bEnable);

EXTERN void audio_ex3d_task(void);

EXTERN uint32_t EX3D_HAngle[EX3D_SET_CH_NUM]; // EX3D Channel �닔�룊媛�
EXTERN uint32_t EX3D_VAngle[EX3D_SET_CH_NUM]; // EX3D Channel �닔吏곴컖
EXTERN DS_BOOL EX3D_Mute[EX3D_SET_CH_NUM]; // EX3D Channel Mute

#endif/*AUDIO_EX3D_CONTROL_H*/
