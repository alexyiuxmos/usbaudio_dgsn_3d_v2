/*
Copyright (c) 2016, UMR STMS 9912 - Ircam-Centre Pompidou / CNRS / UPMC
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "digisonic.h"

#include <string.h>
#include "DSmath.h"

#ifndef __peaklimiter_h__
#define __peaklimiter_h__

enum {
    LIMITER_OK = 0,

    __error_codes_start = -100,

    LIMITER_INVALID_HANDLE,
    LIMITER_INVALID_PARAMETER,

    __error_codes_end
};

#define MAX_PEAKLIMITER_SAMPLE_RATE     48000

#define PEAKLIMITER_ATTACK_DEFAULT_MS      (20.0f)               /* default attack  time in ms */
#define PEAKLIMITER_RELEASE_DEFAULT_MS     (20.0f)              /* default release time in ms */

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_PEAKLIMITER_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

EXTERN int  m_attack;
EXTERN POINT_T  m_attackConst, m_releaseConst;
EXTERN float    m_attackMs, m_releaseMs, m_maxAttackMs;
EXTERN POINT_T  m_threshold;
EXTERN int m_channels, m_maxChannels;
EXTERN int m_sampleRate, m_maxSampleRate;
EXTERN POINT_T  m_fadedGain;
EXTERN POINT_T* m_pMaxBuffer;
EXTERN POINT_T* m_pMaxBufferSlow;
EXTERN POINT_T* m_pDelayBuffer;
EXTERN int m_maxBufferIndex, m_maxBufferSlowIndex, m_delayBufferIndex;
EXTERN int m_sectionLen, m_nbrMaxBufferSection;
EXTERN int m_maxBufferSectionIndex, m_maxBufferSectionCounter;
EXTERN POINT_T  m_smoothState;
EXTERN POINT_T  m_maxMaxBufferSlow, m_maxCurrentSection;
EXTERN int m_indexMaxBufferSlow, *m_pIndexMaxInSection;

EXTERN void PeakLimiter(float maxAttackMs, float releaseMs, float threshold, int maxChannels, int maxSampleRate);
EXTERN void PeakLimiterDestroyer();
EXTERN int resetLimiter();
EXTERN int destroyLimiter();
EXTERN int applyLimiter_E(const POINT_T* samplesIn, POINT_T* samplesOut, int nSamples);
EXTERN int applyLimiter(const POINT_T** samplesIn, POINT_T** samplesOut, int nSamples);
EXTERN int applyLimiter_I(POINT_T** samples, int nSamples);
EXTERN int applyLimiter_E_I(POINT_T* samples, int nSamples);
EXTERN int getLimiterDelay();
EXTERN int getLimiterSampleRate();
EXTERN float getLimiterAttack();
EXTERN float getLimiterRelease();
EXTERN POINT_T getLimiterThreshold();
EXTERN POINT_T getLimiterMaxGainReduction();
EXTERN int setLimiterNChannels(int nChannels);
EXTERN int setLimiterSampleRate(int sampleRate);
EXTERN int setLimiterAttack(float attackMs);
EXTERN int setLimiterRelease(float releaseMs);
EXTERN int setLimiterThreshold(float threshold);

#endif /* __peaklimiter_h__ */
