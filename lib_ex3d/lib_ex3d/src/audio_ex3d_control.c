#define _AUDIO_EX3D_CONTROL_C_

#include "audio_ex3d_control.h"
#define DEBUG_UNIT EX3D_DEBUG
#ifndef DEBUG_PRINT_ENABLE_EX3D_DEBUG
    #define DEBUG_PRINT_ENABLE_EX3D_DEBUG 1
#endif
#include "debug_print.h"

#if defined(USE_EX3D)

// #define log_debug(X) debug_printf X
#define log_debug(X)
#define log_info(X) debug_printf X
// #define log_info(X)
#define log_error(X) debug_printf X

#define DEFAULT_LFE_DB              -6

typedef enum{
    EX3D_MODE_NONE = 0,             // Normal 상태
    EX3D_MODE_SET_EX3D,             // EX3D On/Off 설정
    EX3D_MODE_SET_SF,               // EX3D 음장 설정
    EX3D_MODE_SET_ANGLE,            // EX3D 수평각 설정
    EX3D_MODE_SET_UPMIX,            // EX3D Upmix 설정
    EX3D_MODE_NUM,                  // EX3D 모드수

    EX3D_MODE_STEP_ANGLE,           // Test 용 - Ch1 수평각 45도씩 변경, Ch2 Mute
    EX3D_MODE_ROTATE_MONO_SET_WAIT,
    EX3D_MODE_ROTATE_MONO_SET,      // Test 용 - EX3D_MODE_ROTATE_MONO 모드 실행
    EX3D_MODE_ROTATE_MONO,          // Test 용 - Ch1 수평각 TEST_ROTATE_INTERVAL마다 TEST_ROTATE_INCREMENT_ANGLE도씩 변경, Ch2 Mute
    EX3D_MODE_ROTATE_ALL_SET_WAIT,
    EX3D_MODE_ROTATE_ALL_SET,       // Test 용 - EX3D_MODE_ROTATE_ALL 모드 실행
    EX3D_MODE_ROTATE_ALL,           // Test 용 - TEST_ROTATE_INTERVAL마다 모든 수평각 TEST_ROTATE_INCREMENT_ANGLE도씩 변경
}EX3D_MODE;

#define DEFAULT_VANGLE              90      // 기본 수직각, 단위: 도

#define CH1_DEFAULT_HANGLE          270     // Ch1(Left) 기본 수평각, 단위: 도
#define CH2_DEFAULT_HANGLE          90      // Ch2(Rigth) 기본 수평각, 단위: 도

#define CH1_DEFAULT_UPMIX_HANGLE    315     // Ch1(Left) 기본 Upmix 수평각, 단위: 도
#define CH2_DEFAULT_UPMIX_HANGLE    45      // Ch2(Rigth) 기본 Upmix 수평각, 단위: 도
#define CH3_DEFAULT_UPMIX_HANGLE    0       // Ch3(Center) 기본 Upmix 수평각, 단위: 도
#define CH4_DEFAULT_UPMIX_HANGLE    0       // Ch4(LFE) 기본 Upmix 수평각, 단위: 도
#define CH5_DEFAULT_UPMIX_HANGLE    225     // Ch5(Back Left Surround) 기본 Upmix 수평각, 단위: 도
#define CH6_DEFAULT_UPMIX_HANGLE    135     // Ch6(Back Rigth Surround) 기본 Upmix 수평각, 단위: 도
#define CH7_DEFAULT_UPMIX_HANGLE    270     // Ch7(Left Surround) 기본 Upmix 수평각, 단위: 도
#define CH8_DEFAULT_UPMIX_HANGLE    90      // Ch8(Rigth Surround) 기본 Upmix 수평각, 단위: 도

#define TEST_ROTATE_INTERVAL        50      // 회전 증가 시간 간격, 단위: ms
#define TEST_ROTATE_INCREMENT_ANGLE 1       // 회전 증가 각도, 단위: 도
#define TEST_ROTATE_START_ANGLE     0       // 회전 시작 Left channel 각도, 단위: 도
#define TEST_ROTATE_L_R_SPACE_ANGLE 45      // 회전 L,R 채널 간격, 단위: 도

static uint32_t EX3DMode;                  // EX3D Mode 값
static DS_BOOL bEX3D_Open = false;      // EX3D Open 유무 저장
uint8_t bEX3D_On = 1;                // EX3D On 유무 저장
uint8_t EX3D_SF_Idx = 0;               // EX3D 음장 Index 값
static DS_BOOL bEX3D_Upmix;             // EX3D Upmix 유무 저장
static DS_BOOL bSetEX3D_Upmix;          // EX3D Upmix 설정값 저장

static const char a_sfName[] = "exir2k_xmos_game_wm";
static const char b_sfName[] = "exir2k_xmos_movie_wm";
static const char c_sfName[] = "exir2k_xmos_music_wm";
static const char * sfName[] = {
    a_sfName
    ,b_sfName
    ,c_sfName
};

static const uint32_t EX3D_DefaultHAngle[EX3D_SET_CH_NUM] = {
    CH1_DEFAULT_UPMIX_HANGLE, CH2_DEFAULT_UPMIX_HANGLE
    , CH3_DEFAULT_UPMIX_HANGLE, CH4_DEFAULT_UPMIX_HANGLE
    , CH5_DEFAULT_UPMIX_HANGLE, CH6_DEFAULT_UPMIX_HANGLE
    , CH7_DEFAULT_UPMIX_HANGLE, CH8_DEFAULT_UPMIX_HANGLE
};

volatile uint32_t Sys1msTimeCnt;    // Test 용 - EX3D 내부에서 생성한 1ms Timer Count
static uint32_t ReturnEX3DMode;            // Test 용 - EX3D 복귀 Mode 값
static uint32_t EX3DChannels;
static DS_BOOL bSetEX3D_On;             // Test 용 - EX3D On 설정값 저장
static DS_BOOL bOnlyCh1Sound;           // Test 용 - Ch1 외 Mute 처리
static DS_BOOL bEX3DStepAngle;          // Test 용 - EX3D_MODE_STEP_ANGLE 모드 여부 저장
static uint32_t EX3DStepAngle;             // Test 용 - EX3D_MODE_STEP_ANGLE 모드 step angle 값
static uint32_t EX3DTimeCnt;               // Test 용 - EX3D_MODE_xxx_ROTATE 모드 기준 시간 저장
static uint32_t EX3DWaitTimeCnt;           // Test 용 - EX3D_MODE_xxx_ROTATE 모드 대기 시간 저장
static uint32_t EX3DLastAddAngle;          // Test 용 - EX3D_MODE_xxx_ROTATE 모드 마지막 Add 각도값
static DS_BOOL bEnableRotate;

int audio_ex3d_onoff(DS_BOOL onoff);
int audio_ex3d_upmix_onoff(DS_BOOL onoff);
void audio_ex3d_default_mute_angle(void);

int audio_ex3d_init(uint32_t dwChannels, uint32_t dwSampleSize, uint32_t dwSRHz, uint32_t dwAudioDataSize)
{
    int ret = NO_ERR;
    uint32_t i;

    EX3DMode = EX3D_MODE_NONE;

    log_info(("[%s, Enter] bEX3D_Open:%d\n\r", __FUNCTION__, bEX3D_Open));
    if(bEX3D_Open == false) {
        ret = EX3DAudio_Open(dwChannels, dwSampleSize, dwSRHz, dwAudioDataSize, EX3D_UPMIX_BIT, (PCHAR)sfName[EX3D_SF_Idx]);
    } else {
        ret = ERR_OPENED;
    }

    if(ret == NO_ERR) {
        bEX3D_Open = true;
        EX3DChannels = dwChannels;

        for(i = 0; i < EX3D_SET_CH_NUM; i++) {
            EX3D_VAngle[i] = DEFAULT_VANGLE;
            EX3D_HAngle[i] = EX3D_DefaultHAngle[i];
            EX3D_Mute[i] = false;
        }

        bOnlyCh1Sound = false;
        bEX3DStepAngle = false;
        bEnableRotate = false;

        bEX3D_On = true;
        bEX3D_Upmix = true;
        audio_ex3d_onoff(bEX3D_On);
        audio_ex3d_upmix_onoff(bEX3D_Upmix);
        EX3DAudio_SetLFE(true, DEFAULT_LFE_DB);
        EX3DAudio_SetGain(0);
        EX3DAudio_SetOffGain(0);

        log_info(("[%s] channel:%d, sample_rate:%d, data_len:%d, SF Name:%s\n\r", __FUNCTION__, dwChannels, dwSRHz, dwSampleSize, sfName[EX3D_SF_Idx]));
    } else if(ret == ERR_OPENED) {
        log_info(("[%s] Warning - EX3DAudio is already opened\n\r", __FUNCTION__));
    } else {
        bEX3D_Open = false;
        log_info(("[%s] Error: %d\r\n", __FUNCTION__, ret));
    }

    return ret;
}

int audio_ex3d_conv_init(uint32_t dwTileNum, uint32_t dwChannels)
{
    int ret = NO_ERR;

    ret = EX3DConv_Open(dwTileNum, dwChannels, (PCHAR)sfName[EX3D_SF_Idx]);

    return ret;
}

int audio_ex3d_deinit(void)
{
    log_info(("[%s, Enter] bEX3D_Open:%d", __FUNCTION__, bEX3D_Open));

    if(bEX3D_Open == true) {
        EX3DAudio_Close();
        bEX3D_Open = false;
    }

    return NO_ERR;
}

int audio_ex3d_change_parameter(uint32_t dwChannels, uint32_t dwSampleSize, uint32_t dwSRHz, uint32_t dwAudioDataSize)
{
    int ret = EX3DAudio_ChangeParameter(dwChannels, dwSampleSize, dwSRHz, dwAudioDataSize);
    if(ret == NO_ERR) {
        EX3DChannels = dwChannels;

        audio_ex3d_default_mute_angle();

        log_info(("[%s] channel:%d, sample_rate:%d, data_len:%d\n\r", __FUNCTION__, dwChannels, dwSRHz, dwSampleSize));
    } else {
        log_error(("[%s] Error: %d", __FUNCTION__, ret));
    }

    return ret;
}

int audio_ex3d_onoff(DS_BOOL onoff)
{
    int ret;

    EX3DMode = EX3D_MODE_NONE;
    bEX3DStepAngle = false;

    ret = EX3DAudio_SetEX3D(onoff);
    if (ret == NO_ERR) {
        bEX3D_On = onoff;
        log_info(("[%s] Set EX3D:%d\n\r", __FUNCTION__, onoff));
    } else {
        log_error(("[%s] Set EX3D Error:%d\n\r", __FUNCTION__, ret));
    }

    return ret;
}

void audio_ex3d_set_onoff(DS_BOOL onoff)
{
    if(bEX3D_On != onoff) {
        // wait last process
        // while((EX3DMode != EX3D_MODE_NONE) && (EX3DMode < EX3D_MODE_NUM)) {
        //     os_time_dly(1);
        // }

        bSetEX3D_On = onoff;

        EX3DMode = EX3D_MODE_SET_EX3D;

        log_info(("[%s] set mode to EX3D_MODE_SET_EX3D\n\r", __FUNCTION__));
    } else {
        log_info(("[%s] already EX3D is %s\n\r", __FUNCTION__, onoff ? "On" : "Off"));
    }
}

void audio_ex3d_set_sf(uint32_t SF_Idx)
{
    if(bEX3D_On) {
        SF_Idx = SF_Idx % (sizeof(sfName) / sizeof(sfName[0]));
        if(EX3D_SF_Idx != SF_Idx) {
            // wait last process
            // while((EX3DMode != EX3D_MODE_NONE) && (EX3DMode < EX3D_MODE_NUM)) {
            //     os_time_dly(1);
            // }

            EX3D_SF_Idx = SF_Idx;
            ReturnEX3DMode = EX3D_MODE_NONE;

            EX3DMode = EX3D_MODE_SET_SF;

            log_info(("[%s] set mode to EX3D_MODE_SET_SF SF Name:%s, EX3D_SF_Idx:%d\n\r", __FUNCTION__, sfName[EX3D_SF_Idx], EX3D_SF_Idx));
        } else {
            log_info(("[%s] already EX3D SF is %s\n\r", __FUNCTION__, sfName[EX3D_SF_Idx]));
        }
    } else {
        log_info(("[%s] Does not change SF, change only when EX3D is On\n\r", __FUNCTION__));
    }
}

void audio_ex3d_set_angle(uint32_t dwCh1Angle, uint32_t dwCh2Angle,
                          uint32_t dwCh3Angle, uint32_t dwCh4Angle,
                          uint32_t dwCh5Angle, uint32_t dwCh6Angle,
                          uint32_t dwCh7Angle, uint32_t dwCh8Angle)
{
    if(bEX3D_On) {
        // wait last process
        // while((EX3DMode != EX3D_MODE_NONE) && (EX3DMode < EX3D_MODE_NUM)) {
        //     os_time_dly(1);
        // }

        EX3D_HAngle[0] = dwCh1Angle % 360;
        EX3D_HAngle[1] = dwCh2Angle % 360;
        EX3D_HAngle[2] = dwCh3Angle % 360;
        // EX3D_HAngle[3] = dwCh4Angle % 360;   // LFE 제외
        EX3D_HAngle[4] = dwCh5Angle % 360;
        EX3D_HAngle[5] = dwCh6Angle % 360;
        EX3D_HAngle[6] = dwCh7Angle % 360;
        EX3D_HAngle[7] = dwCh8Angle % 360;

        EX3DMode = EX3D_MODE_SET_ANGLE;

        log_info(("[%s] set mode to EX3D_MODE_SET_ANGLE (ch1~ch8): %d, %d, %d, %d, %d, %d, %d, %d\n\r", __FUNCTION__,
                    EX3D_HAngle[0], EX3D_HAngle[1], EX3D_HAngle[2], EX3D_HAngle[3],
                    EX3D_HAngle[4], EX3D_HAngle[5], EX3D_HAngle[6], EX3D_HAngle[7]));
    } else {
        log_info(("[%s] Does not change angle, change only when EX3D is On\n\r", __FUNCTION__));
    }
}

int audio_ex3d_upmix_onoff(DS_BOOL onoff)
{
    int ret;

    EX3DMode = EX3D_MODE_NONE;
    bEX3DStepAngle = false;

    ret = EX3DAudio_SetUpmix(onoff);
    if (ret == NO_ERR) {
        bEX3D_Upmix = onoff;

        audio_ex3d_default_mute_angle();

        log_info(("[%s] Set EX3D Upmix:%d\n\r", __FUNCTION__, onoff));
    } else {
        log_error(("[%s] Set EX3D Upmix Error:%d\n\r", __FUNCTION__, ret));
    }

    return ret;
}

void audio_ex3d_set_upmix(DS_BOOL onoff)
{
    if(bEX3D_Upmix != onoff) {
        // wait last process
        // while((EX3DMode != EX3D_MODE_NONE) && (EX3DMode < EX3D_MODE_NUM)) {
        //     os_time_dly(1);
        // }

        bSetEX3D_Upmix = onoff;

        EX3DMode = EX3D_MODE_SET_UPMIX;

        log_info(("[%s] set mode to EX3D_MODE_SET_UPMIX\n\r", __FUNCTION__));
    } else {
        log_info(("[%s] already EX3D Upmix is %s\n\r", __FUNCTION__, onoff ? "On" : "Off"));
    }
}

void audio_ex3d_default_mute_angle(void)
{
    if(bOnlyCh1Sound == true) {
        bOnlyCh1Sound = false;
        EX3D_Mute[1] = false;
        EX3DAudio_SetMute(2, false);
        if(bEX3D_Upmix == true) {
            EX3D_Mute[2] = false;
            EX3DAudio_SetMute(3, false);

            EX3D_Mute[3] = false;
            EX3DAudio_SetMute(4, false);
            EX3DAudio_SetLFE(true, 0);

            EX3D_Mute[4] = false;
            EX3DAudio_SetMute(5, false);
            EX3D_Mute[5] = false;
            EX3DAudio_SetMute(6, false);
#if (UPMIX_CHANNEL_NUMBER == 8)
            EX3D_Mute[6] = false;
            EX3DAudio_SetMute(7, false);
            EX3D_Mute[7] = false;
            EX3DAudio_SetMute(8, false);
#endif
        }
    }

    // if(bEX3D_On == true) 
    {
        if((EX3DChannels <= 2) && (bEX3D_Upmix == false)) {
            EX3D_HAngle[0] = CH1_DEFAULT_HANGLE;
            EX3D_HAngle[1] = CH2_DEFAULT_HANGLE;
        } else {
            EX3D_HAngle[0] = CH1_DEFAULT_UPMIX_HANGLE;
            EX3D_HAngle[1] = CH2_DEFAULT_UPMIX_HANGLE;
        }

        EX3D_HAngle[2] = CH3_DEFAULT_UPMIX_HANGLE;
        EX3D_HAngle[3] = CH4_DEFAULT_UPMIX_HANGLE;
        EX3D_HAngle[4] = CH5_DEFAULT_UPMIX_HANGLE;
        EX3D_HAngle[5] = CH6_DEFAULT_UPMIX_HANGLE;
        EX3D_HAngle[6] = CH7_DEFAULT_UPMIX_HANGLE;
        EX3D_HAngle[7] = CH8_DEFAULT_UPMIX_HANGLE;

        EX3DAudio_SetAngle(1, DEFAULT_VANGLE, EX3D_HAngle[0], false);
        EX3DAudio_SetAngle(2, DEFAULT_VANGLE, EX3D_HAngle[1], false);
        EX3DAudio_SetAngle(3, DEFAULT_VANGLE, EX3D_HAngle[2], false);
        EX3DAudio_SetAngle(4, DEFAULT_VANGLE, EX3D_HAngle[3], false);
        EX3DAudio_SetAngle(5, DEFAULT_VANGLE, EX3D_HAngle[4], false);
        EX3DAudio_SetAngle(6, DEFAULT_VANGLE, EX3D_HAngle[5], false);
        EX3DAudio_SetAngle(7, DEFAULT_VANGLE, EX3D_HAngle[6], false);
        EX3DAudio_SetAngle(8, DEFAULT_VANGLE, EX3D_HAngle[7], true);
        log_info(("[%s] Set EX3D angle(ch1~ch8): %d, %d, %d, %d, %d, %d, %d, %d\n\r", __FUNCTION__,
            EX3D_HAngle[0], EX3D_HAngle[1], EX3D_HAngle[2], EX3D_HAngle[3],
            EX3D_HAngle[4], EX3D_HAngle[5], EX3D_HAngle[6], EX3D_HAngle[7]));
    }
}

void audio_ex3d_upmix_toggle(void)
{
    bSetEX3D_Upmix = !bEX3D_Upmix;

    EX3DMode = EX3D_MODE_SET_UPMIX;

    log_info(("[%s] set mode to EX3D_MODE_SET_UPMIX\n\r", __FUNCTION__));
}

void audio_ex3d_toggle(void)
{
    if((EX3DMode == EX3D_MODE_ROTATE_MONO) || (EX3DMode == EX3D_MODE_ROTATE_ALL) || (bEX3DStepAngle == true)) {
        bSetEX3D_On = true;
    } else {
        bSetEX3D_On = !bEX3D_On;
    }

    EX3DMode = EX3D_MODE_SET_EX3D;

    log_debug(("[%s] set mode to EX3D_MODE_SET_EX3D\n\r", __FUNCTION__));
}

void audio_ex3d_test_cancel(void)
{
    audio_ex3d_onoff(true);

    log_info(("[%s] set mode to EX3D_MODE_NONE\n\r", __FUNCTION__));
}

void audio_ex3d_step_angle(void)
{
    EX3DMode = EX3D_MODE_STEP_ANGLE;

    log_info(("[%s] set mode to EX3D_MODE_STEP_ANGLE\n\r", __FUNCTION__));
}


void audio_ex3d_rotate_mono_angle_wait(uint32_t msWaitTime)
{
    if(EX3DMode != EX3D_MODE_ROTATE_MONO) {
        EX3DWaitTimeCnt = Sys1msTimeCnt + msWaitTime;

        EX3DMode = EX3D_MODE_ROTATE_MONO_SET_WAIT;

        log_info(("[%s] set mode to EX3D_MODE_ROTATE_MONO_SET_WAIT\n\r", __FUNCTION__));
    }
}

void audio_ex3d_rotate_mono_angle(void)
{
    if(EX3DMode != EX3D_MODE_ROTATE_MONO) {
        EX3DMode = EX3D_MODE_ROTATE_MONO_SET;

        log_info(("[%s] set mode to EX3D_MODE_ROTATE_MONO_SET\n\r", __FUNCTION__));
    }
}

void audio_ex3d_rotate_all_angle_wait(uint32_t msWaitTime)
{
    if(EX3DMode != EX3D_MODE_ROTATE_ALL) {
        EX3DWaitTimeCnt = Sys1msTimeCnt + msWaitTime;

        EX3DMode = EX3D_MODE_ROTATE_ALL_SET_WAIT;

        log_info(("[%s] set mode to EX3D_MODE_ROTATE_ALL_SET_WAIT\n\r", __FUNCTION__));
    }
}

void audio_ex3d_rotate_all_angle(void)
{
    if(EX3DMode != EX3D_MODE_ROTATE_ALL) {
        EX3DMode = EX3D_MODE_ROTATE_ALL_SET;

        log_info(("[%s] set mode to EX3D_MODE_ROTATE_ALL_SET\n\r", __FUNCTION__));
    }
}

void audio_ex3d_rotate_sf(void)
{
    if(bEX3D_On) {
        EX3D_SF_Idx++;
        EX3D_SF_Idx = EX3D_SF_Idx % (sizeof(sfName) / sizeof(sfName[0]));

        ReturnEX3DMode = EX3DMode;
        EX3DMode = EX3D_MODE_SET_SF;

        log_info(("[%s] set mode to EX3D_MODE_SET_SF\n\r", __FUNCTION__));
    } else {
        log_info(("[%s] Does not change the SF, change only when EX3D is On\n\r", __FUNCTION__));
    }
}

void audio_ex3d_enable_rotate_angle(DS_BOOL bEnable)
{
    bEnableRotate = bEnable;

    log_info(("[%s] bEnableRotate:%d\n\r", __FUNCTION__, bEnableRotate));
}

void audio_ex3d_task(void)
{
    uint32_t EX3DElapsedTimeCnt, AddAngle;
    uint32_t HAngle;
    int i;

    if(bEX3D_Open == true) {
        switch(EX3DMode) {
        case EX3D_MODE_NONE:
            break;

        case EX3D_MODE_SET_EX3D:
            audio_ex3d_onoff(bSetEX3D_On);

            EX3DMode = EX3D_MODE_NONE;
            log_debug(("[%s] set EX3D %s\n\r", __FUNCTION__, bEX3D_On ? "On" : "Off"));
            break;

        case EX3D_MODE_SET_SF:
            EX3DAudio_SetSoundField((PCHAR)sfName[EX3D_SF_Idx]);

            EX3DMode = ReturnEX3DMode;
            log_debug(("[%s] set EX3D SF Name:%s, EX3D_SF_Idx:%d\n\r", __FUNCTION__, sfName[EX3D_SF_Idx], EX3D_SF_Idx));
            break;

        case EX3D_MODE_SET_ANGLE:
            EX3DAudio_SetAngle(1, DEFAULT_VANGLE, EX3D_HAngle[0], false);
            EX3DAudio_SetAngle(2, DEFAULT_VANGLE, EX3D_HAngle[1], false);
            EX3DAudio_SetAngle(3, DEFAULT_VANGLE, EX3D_HAngle[2], false);
            EX3DAudio_SetAngle(4, DEFAULT_VANGLE, EX3D_HAngle[3], false);
            EX3DAudio_SetAngle(5, DEFAULT_VANGLE, EX3D_HAngle[4], false);
#if (UPMIX_CHANNEL_NUMBER == 8)
            EX3DAudio_SetAngle(6, DEFAULT_VANGLE, EX3D_HAngle[5], false);
            EX3DAudio_SetAngle(7, DEFAULT_VANGLE, EX3D_HAngle[6], false);
            EX3DAudio_SetAngle(8, DEFAULT_VANGLE, EX3D_HAngle[7], true);
            log_debug(("[%s] Set EX3D angle(ch1~ch8): %d, %d, %d, %d, %d, %d, %d, %d\n\r", __FUNCTION__,
                EX3D_HAngle[0], EX3D_HAngle[1], EX3D_HAngle[2], EX3D_HAngle[3],
                EX3D_HAngle[4], EX3D_HAngle[5], EX3D_HAngle[6], EX3D_HAngle[7]));
#else
            EX3DAudio_SetAngle(6, DEFAULT_VANGLE, EX3D_HAngle[5], true);
            log_debug(("[%s] Set EX3D angle(ch1~ch6): %d, %d, %d, %d, %d, %d\n\r", __FUNCTION__, 
                EX3D_HAngle[0], EX3D_HAngle[1], EX3D_HAngle[2], EX3D_HAngle[3],
                EX3D_HAngle[4], EX3D_HAngle[5]));
#endif

            EX3DMode = EX3D_MODE_NONE;
            break;

        case EX3D_MODE_SET_UPMIX:
            audio_ex3d_upmix_onoff(bSetEX3D_Upmix);

            EX3DMode = EX3D_MODE_NONE;
            log_debug(("[%s] set EX3D Upmix:%s\n\r", __FUNCTION__, bSetEX3D_Upmix ? "On" : "Off"));
            break;

        case EX3D_MODE_STEP_ANGLE:
            if(bEX3DStepAngle == false) {
                bEX3DStepAngle = true;

                if(bEX3D_On == false) {
                    bEX3D_On = true;
                    EX3DAudio_SetEX3D(bEX3D_On);
                }

                bOnlyCh1Sound = true;
                for(i=1; i<8; i++) {
                    EX3D_Mute[i] = true;
                    EX3DAudio_SetMute(i+1, true);
                    if(i == 3) {
                        EX3DAudio_SetLFE(false, 0);
                    }
                }

                EX3DStepAngle = 0;
            } else {
                EX3DStepAngle += 45;
                EX3DStepAngle = EX3DStepAngle % 360;
            }

            EX3DAudio_SetAngle(1, DEFAULT_VANGLE, EX3DStepAngle, true);

            EX3DMode = EX3D_MODE_NONE;
            log_debug(("[%s] set EX3DStepAngle:%d\n\r", __FUNCTION__, EX3DStepAngle));
            break;

        case EX3D_MODE_ROTATE_MONO_SET_WAIT:
            if(EX3DWaitTimeCnt < Sys1msTimeCnt) {
                EX3DMode = EX3D_MODE_ROTATE_MONO_SET;
                log_debug(("[%s] set mode to EX3D_MODE_ROTATE_MONO_SET\n\r", __FUNCTION__));
            }
            break;

        case EX3D_MODE_ROTATE_MONO_SET:
            bEX3DStepAngle = false;

            if(bEX3D_On == false) {
                bEX3D_On = true;
                EX3DAudio_SetEX3D(bEX3D_On);
            }

            bOnlyCh1Sound = true;
            for(i=1; i<8; i++) {
                EX3D_Mute[i] = true;
                EX3DAudio_SetMute(i+1, true);
                if(i == 3) {
                    EX3DAudio_SetLFE(false, 0);
                }
            }

            EX3DLastAddAngle = 360; // Last값을 나올 수 없는 360으로 설정하여 바로 적용될 수 있도록 함
            EX3DTimeCnt = Sys1msTimeCnt;

            EX3DMode = EX3D_MODE_ROTATE_MONO;
            log_debug(("[%s] set mode to EX3D_MODE_ROTATE_MONO\n\r", __FUNCTION__));
            break;

        case EX3D_MODE_ROTATE_MONO:
            if(bEnableRotate == TRUE) {
                EX3DElapsedTimeCnt = Sys1msTimeCnt - EX3DTimeCnt;
            
                // TEST_ROTATE_INTERVAL(ms) 마다 TEST_ROTATE_INCREMENT_ANGLE 각도씩 변경
                AddAngle = (EX3DElapsedTimeCnt / TEST_ROTATE_INTERVAL) * TEST_ROTATE_INCREMENT_ANGLE;
            
                if(EX3DLastAddAngle != AddAngle) {
                    EX3DLastAddAngle = AddAngle;
                    HAngle = (TEST_ROTATE_START_ANGLE + AddAngle) % 360;
                    EX3DAudio_SetAngle(1, DEFAULT_VANGLE, HAngle, true);

                    //log_debug(("[%s] EX3D_MODE_ROTATE_MONO ch1-%d\n\r", __FUNCTION__, HAngle));
                }
            } else {
                EX3DTimeCnt = Sys1msTimeCnt;
            }
            break;

        case EX3D_MODE_ROTATE_ALL_SET_WAIT:
            if(EX3DWaitTimeCnt < Sys1msTimeCnt) {
                EX3DMode = EX3D_MODE_ROTATE_ALL_SET;
                log_debug(("[%s] set mode to EX3D_MODE_ROTATE_ALL_SET\n\r", __FUNCTION__));
            }
            break;

        case EX3D_MODE_ROTATE_ALL_SET:
            bEX3DStepAngle = false;

            if(bEX3D_On == false) {
                bEX3D_On = true;
                EX3DAudio_SetEX3D(bEX3D_On);
            }

            audio_ex3d_default_mute_angle();

            EX3DLastAddAngle = 360; // Last값을 나올 수 없는 360으로 설정하여 바로 적용될 수 있도록 함
            EX3DTimeCnt = Sys1msTimeCnt;

            EX3DMode = EX3D_MODE_ROTATE_ALL;
            log_debug(("[%s] set mode to EX3D_MODE_ROTATE_ALL\n\r", __FUNCTION__));
            break;

        case EX3D_MODE_ROTATE_ALL:
            if(bEnableRotate == TRUE) {
                EX3DElapsedTimeCnt = Sys1msTimeCnt - EX3DTimeCnt;

                // TEST_ROTATE_INTERVAL(ms) 마다 TEST_ROTATE_INCREMENT_ANGLE 각도씩 변경
                AddAngle = (EX3DElapsedTimeCnt / TEST_ROTATE_INTERVAL) * TEST_ROTATE_INCREMENT_ANGLE;

                if(EX3DLastAddAngle != AddAngle) {
                    EX3DLastAddAngle = AddAngle;

                    if(bEX3D_Upmix == true) {
                        HAngle = (CH1_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(1, DEFAULT_VANGLE, HAngle, false);
                        HAngle = (CH2_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(2, DEFAULT_VANGLE, HAngle, false);
                        HAngle = (CH3_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(3, DEFAULT_VANGLE, HAngle, false);
                        HAngle = (CH4_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(4, DEFAULT_VANGLE, HAngle, false);
                        HAngle = (CH5_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(5, DEFAULT_VANGLE, HAngle, false);
                        HAngle = (CH6_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
#if (UPMIX_CHANNEL_NUMBER == 8)
                        EX3DAudio_SetAngle(6, DEFAULT_VANGLE, HAngle, false);
                        HAngle = (CH7_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(7, DEFAULT_VANGLE, HAngle, false);
                        HAngle = (CH8_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(8, DEFAULT_VANGLE, HAngle, true);
#else
                        EX3DAudio_SetAngle(6, DEFAULT_VANGLE, HAngle, true);
#endif
                    } else {
                        uint32_t Ch1HAngle, Ch2HAngle;

                        Ch1HAngle = (CH1_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(1, DEFAULT_VANGLE, Ch1HAngle, false);
                        Ch2HAngle = (CH2_DEFAULT_UPMIX_HANGLE + AddAngle) % 360;
                        EX3DAudio_SetAngle(2, DEFAULT_VANGLE, Ch2HAngle, true);

                        //log_debug(("[%s] EX3D_MODE_ROTATE_STEREO ch1-%d, ch2-%d\n\r", __FUNCTION__, Ch1HAngle, Ch2HAngle));
                    }
                }
            } else {
                EX3DTimeCnt = Sys1msTimeCnt;
            }
            break;

        default:
            log_error(("[%s] Error EX3DMode:%d, Set EX3D_MODE_NONE\n\r", __FUNCTION__, EX3DMode));
            EX3DMode = EX3D_MODE_NONE;
            break;
        }
    }
}

#endif
