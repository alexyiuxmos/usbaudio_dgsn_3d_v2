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

#define _PEAKLIMITER_C_

#include "peakLimiter.h"

#ifndef max
#define max(a, b)   (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)   (((a) < (b)) ? (a) : (b))
#endif

POINT_T m_fVal_0p1;
POINT_T m_fVal_1p0;
POINT_T m_fVal_1p11111111;

/* create limiter */
void PeakLimiter(
                float         maxAttackMsIn,
                float         releaseMsIn,
                float         thresholdIn,
                int  maxChannelsIn,
                int  maxSampleRateIn
                )
{

	/* calc m_attack time in samples */
	m_attack = (int)(maxAttackMsIn * maxSampleRateIn / 1000);

	if (m_attack < 1) /* m_attack time is too short */
		m_attack = 1;

	/* length of m_pMaxBuffer sections */
	m_sectionLen = (int)sqrt((float)m_attack+1);
	/* sqrt(m_attack+1) leads to the minimum
	 of the number of maximum operators:
	 nMaxOp = m_sectionLen + (m_attack+1)/m_sectionLen */

	/* alloc limiter struct */

	m_nbrMaxBufferSection = (m_attack+1)/m_sectionLen;
	if (m_nbrMaxBufferSection*m_sectionLen < (m_attack+1))
		m_nbrMaxBufferSection++; /* create a full section for the last samples */

	/* alloc maximum and delay buffers */
	m_pMaxBuffer = (PPOINT_T)DS_malloc(sizeof(POINT_T) * m_nbrMaxBufferSection * m_sectionLen);
	m_pDelayBuffer = (PPOINT_T)DS_malloc(sizeof(POINT_T) * m_attack * maxChannelsIn);
	m_pMaxBufferSlow = (PPOINT_T)DS_malloc(sizeof(POINT_T) * m_nbrMaxBufferSection);
	m_pIndexMaxInSection = (int *)DS_malloc(sizeof(int) * m_nbrMaxBufferSection);

	if ((m_pMaxBuffer==NULL) || (m_pDelayBuffer==NULL) || (m_pMaxBufferSlow==NULL)) {
		destroyLimiter();
		return;
	}

	/* init parameters & states */
	m_maxBufferIndex = 0;
	m_delayBufferIndex = 0;
	m_maxBufferSlowIndex = 0;
	m_maxBufferSectionIndex = 0;
	m_maxBufferSectionCounter = 0;
	m_maxMaxBufferSlow = 0;
	m_indexMaxBufferSlow = 0;
	m_maxCurrentSection = 0;

	m_attackMs      = maxAttackMsIn;
	m_maxAttackMs   = maxAttackMsIn;
    m_attackConst   = FIXED_CONV( DS_POW( 0.1, 1.0 / (m_attack + 1)) );
	m_releaseMs     = releaseMsIn;
	m_releaseConst  = FIXED_CONV( DS_POW(0.1, 1.0 / (m_releaseMs * maxSampleRateIn / 1000 + 1)) );
	m_threshold     = FIXED_CONV(thresholdIn);
	m_channels      = maxChannelsIn;
	m_maxChannels   = maxChannelsIn;
	m_sampleRate    = maxSampleRateIn;
	m_maxSampleRate = maxSampleRateIn;


	m_fadedGain = FIXED_CONV(1.0);
	m_smoothState = FIXED_CONV(1.0);

    m_fVal_0p1 = FIXED_CONV(0.1f);
    m_fVal_1p0 = FIXED_CONV(1.0f);
    m_fVal_1p11111111 = FIXED_CONV(1.11111111f);

	memset(m_pMaxBuffer,0,sizeof(POINT_T)*m_nbrMaxBufferSection * m_sectionLen);
	memset(m_pDelayBuffer,0,sizeof(POINT_T)*m_attack * maxChannelsIn);
	memset(m_pMaxBufferSlow,0,sizeof(POINT_T)*m_nbrMaxBufferSection);
	memset(m_pIndexMaxInSection,0,sizeof( int)*m_nbrMaxBufferSection);
}

void PeakLimiterDestroyer()
{
    destroyLimiter();
}

/* reset limiter */
int resetLimiter()
{
	m_maxBufferIndex = 0;
	m_delayBufferIndex = 0;
	m_maxBufferSlowIndex = 0;
	m_maxBufferSectionIndex = 0;
	m_maxBufferSectionCounter = 0;
	m_fadedGain = FIXED_CONV(1.0);
	m_smoothState = FIXED_CONV(1.0);
	m_maxMaxBufferSlow = 0;
	m_indexMaxBufferSlow = 0;
	m_maxCurrentSection = 0;


	memset(m_pMaxBuffer,0,sizeof(POINT_T)*m_nbrMaxBufferSection * m_sectionLen);
	memset(m_pDelayBuffer,0,sizeof(POINT_T)*m_attack * m_maxChannels);
	memset(m_pMaxBufferSlow,0,sizeof(POINT_T)*m_nbrMaxBufferSection);
	memset(m_pIndexMaxInSection,0,sizeof(int)*m_nbrMaxBufferSection);

	return LIMITER_OK;
}

/* destroy limiter */
int destroyLimiter()
{
    if (m_pMaxBuffer) {
        DS_free(m_pMaxBuffer);
        m_pMaxBuffer = NULL;
    }
    if (m_pDelayBuffer) {
        DS_free(m_pDelayBuffer);
        m_pDelayBuffer = NULL;
    }
    if (m_pMaxBufferSlow) {
        DS_free(m_pMaxBufferSlow);
        m_pMaxBufferSlow = NULL;
    }
    if (m_pIndexMaxInSection) {
        DS_free(m_pIndexMaxInSection);
        m_pIndexMaxInSection = NULL;
    }

    return LIMITER_OK;
}

/* apply limiter */
int applyLimiter_E(const POINT_T *samplesIn,POINT_T *samplesOut, int nSamples)
{
	memcpy(samplesOut,samplesIn,nSamples*sizeof(POINT_T));
	return applyLimiter_E_I(samplesOut,nSamples);
}

/* apply limiter */
int applyLimiter_E_I(POINT_T *samples, int nSamples)
{
	int i, j;
	POINT_T tmp, gain, maximum;

    for (i = 0; i < nSamples; i++) {
        /* get maximum absolute sample value of all channels that are greater in absoulte value to m_threshold */
        m_pMaxBuffer[m_maxBufferIndex] = m_threshold;
        for (j = 0; j < m_channels; j++) {
#if defined(USE_FIXED_POINT)
            m_pMaxBuffer[m_maxBufferIndex] = max(m_pMaxBuffer[m_maxBufferIndex], (POINT_T)abs(samples[i * m_channels + j]));
#else
            m_pMaxBuffer[m_maxBufferIndex] = max(m_pMaxBuffer[m_maxBufferIndex], (POINT_T)fabs(samples[i * m_channels + j]));
#endif
        }

        /* search maximum in the current section */
        if (m_pIndexMaxInSection[m_maxBufferSlowIndex] == m_maxBufferIndex) // if we have just changed the sample containg the old maximum value
        {
            // need to compute the maximum on the whole section 
            m_maxCurrentSection = m_pMaxBuffer[m_maxBufferSectionIndex];
            for (j = 1; j < m_sectionLen; j++) {
                if (m_pMaxBuffer[m_maxBufferSectionIndex + j] > m_maxCurrentSection)
                {
                    m_maxCurrentSection = m_pMaxBuffer[m_maxBufferSectionIndex + j];
                    m_pIndexMaxInSection[m_maxBufferSlowIndex] = m_maxBufferSectionIndex + j;
                }
            }
        }
        else // just need to compare the new value the cthe current maximum value
        {
            if (m_pMaxBuffer[m_maxBufferIndex] > m_maxCurrentSection)
            {
                m_maxCurrentSection = m_pMaxBuffer[m_maxBufferIndex];
                m_pIndexMaxInSection[m_maxBufferSlowIndex] = m_maxBufferIndex;
            }
        }

        // find maximum of slow (downsampled) max buffer
        maximum = m_maxMaxBufferSlow;
        if (m_maxCurrentSection > maximum)
        {
            maximum = m_maxCurrentSection;
        }

        m_maxBufferIndex++;
        m_maxBufferSectionCounter++;

        /* if m_pMaxBuffer section is finished, or end of m_pMaxBuffer is reached,
        store the maximum of this section and open up a new one */
        if ((m_maxBufferSectionCounter >= m_sectionLen) || (m_maxBufferIndex >= m_attack + 1)) {
            m_maxBufferSectionCounter = 0;

            tmp = m_pMaxBufferSlow[m_maxBufferSlowIndex] = m_maxCurrentSection;
            j = 0;
            if (m_indexMaxBufferSlow == m_maxBufferSlowIndex)
            {
                j = 1;
            }
            m_maxBufferSlowIndex++;
            if (m_maxBufferSlowIndex >= m_nbrMaxBufferSection)
            {
                m_maxBufferSlowIndex = 0;
            }
            if (m_indexMaxBufferSlow == m_maxBufferSlowIndex)
            {
                j = 1;
            }
            m_maxCurrentSection = m_pMaxBufferSlow[m_maxBufferSlowIndex];
            m_pMaxBufferSlow[m_maxBufferSlowIndex] = 0;  /* zero out the value representing the new section */

            /* compute the maximum over all the section */
            if (j)
            {
                m_maxMaxBufferSlow = 0;
                for (j = 0; j < m_nbrMaxBufferSection; j++)
                {
                    if (m_pMaxBufferSlow[j] > m_maxMaxBufferSlow)
                    {
                        m_maxMaxBufferSlow = m_pMaxBufferSlow[j];
                        m_indexMaxBufferSlow = j;
                    }
                }
            }
            else
            {
                if (tmp > m_maxMaxBufferSlow)
                {
                    m_maxMaxBufferSlow = tmp;
                    m_indexMaxBufferSlow = m_maxBufferSlowIndex;
                }
            }

            m_maxBufferSectionIndex += m_sectionLen;
        }

        if (m_maxBufferIndex >= (m_attack + 1))
        {
            m_maxBufferIndex = 0;
            m_maxBufferSectionIndex = 0;
        }

        /* needed current gain */
        if (maximum > m_threshold)
        {
            gain = POINT_DIV_2(m_threshold, maximum);
        }
        else
        {
            gain = m_fVal_1p0;
        }

        /*avoid overshoot */

        if (gain < m_smoothState) {
			POINT_T fVal = POINT_MUL_2(m_fVal_0p1, m_smoothState);
            fVal = gain - fVal;
            fVal = POINT_MUL_2(fVal, m_fVal_1p11111111);
            m_fadedGain = min( m_fadedGain, fVal );
        }
        else
        {
            m_fadedGain = gain;
        }


        /* smoothing gain */
        if (m_fadedGain < m_smoothState)
        {
            m_smoothState = POINT_MUL_2(m_attackConst, (m_smoothState - m_fadedGain)) + m_fadedGain;  /* m_attack */
            /*avoid overshoot */
            if (gain > m_smoothState)
            {
                m_smoothState = gain;
            }
        }
        else
        {
            m_smoothState = POINT_MUL_2(m_releaseConst, (m_smoothState - m_fadedGain)) + m_fadedGain; /* release */
        }

        /* fill delay line, apply gain */
        for (j = 0; j < m_channels; j++)
        {
            tmp = m_pDelayBuffer[m_delayBufferIndex * m_channels + j];
            m_pDelayBuffer[m_delayBufferIndex * m_channels + j] = samples[i * m_channels + j];

            tmp = POINT_MUL_2(tmp, m_smoothState);
            if (tmp > m_threshold) tmp = m_threshold;
            if (tmp < -m_threshold) tmp = -m_threshold;

            samples[i * m_channels + j] = tmp;
        }

        m_delayBufferIndex++;
        if (m_delayBufferIndex >= m_attack)
            m_delayBufferIndex = 0;

    }

    return LIMITER_OK;
}

/* apply limiter */
int applyLimiter(const POINT_T **samplesIn,POINT_T **samplesOut, int nSamples)
{
	int ind;
	for(ind=0;ind<m_channels;ind++)
	{
		memcpy(samplesOut[ind],samplesIn[ind],nSamples*sizeof(POINT_T));
	}
	return applyLimiter_I(samplesOut,nSamples);
}

/* apply limiter */
int applyLimiter_I( POINT_T **samples,int nSamples)
{
	int i, j;
	POINT_T tmp, gain, maximum;
    
    for (i = 0; i < nSamples; i++) {
        /* get maximum absolute sample value of all channels that are greater in absoulte value to m_threshold */
        m_pMaxBuffer[m_maxBufferIndex] = m_threshold;
        for (j = 0; j < m_channels; j++) {
#if defined(USE_FIXED_POINT)
            m_pMaxBuffer[m_maxBufferIndex] = max(m_pMaxBuffer[m_maxBufferIndex], (POINT_T)abs(samples[j][i]));
#else
            m_pMaxBuffer[m_maxBufferIndex] = max(m_pMaxBuffer[m_maxBufferIndex], (POINT_T)fabs(samples[j][i]));
#endif
        }
                
        /* search maximum in the current section */
        if (m_pIndexMaxInSection[m_maxBufferSlowIndex] == m_maxBufferIndex) // if we have just changed the sample containg the old maximum value
        {
            // need to compute the maximum on the whole section 
            m_maxCurrentSection = m_pMaxBuffer[m_maxBufferSectionIndex];
            for (j = 1; j < m_sectionLen; j++) {
                if (m_pMaxBuffer[m_maxBufferSectionIndex + j] > m_maxCurrentSection)
                {
                    m_maxCurrentSection = m_pMaxBuffer[m_maxBufferSectionIndex + j];
                    m_pIndexMaxInSection[m_maxBufferSlowIndex] = m_maxBufferSectionIndex+j;
                }
            }
        }
        else // just need to compare the new value the cthe current maximum value
        {
            if (m_pMaxBuffer[m_maxBufferIndex] > m_maxCurrentSection)
            {
                m_maxCurrentSection = m_pMaxBuffer[m_maxBufferIndex];
                m_pIndexMaxInSection[m_maxBufferSlowIndex] = m_maxBufferIndex;
            }
        }
        
        // find maximum of slow (downsampled) max buffer
        maximum = m_maxMaxBufferSlow;
        if (m_maxCurrentSection > maximum)
        {
            maximum = m_maxCurrentSection;
        }
    
        m_maxBufferIndex++;
        m_maxBufferSectionCounter++;
        
        /* if m_pMaxBuffer section is finished, or end of m_pMaxBuffer is reached,
         store the maximum of this section and open up a new one */
        if ((m_maxBufferSectionCounter >= m_sectionLen)||(m_maxBufferIndex >= m_attack+1)) {
            m_maxBufferSectionCounter = 0;
            
            tmp = m_pMaxBufferSlow[m_maxBufferSlowIndex] = m_maxCurrentSection;
            j = 0;
            if (m_indexMaxBufferSlow == m_maxBufferSlowIndex)
            {
                j = 1;
            }
            m_maxBufferSlowIndex++;
            if (m_maxBufferSlowIndex >= m_nbrMaxBufferSection)
            {
                m_maxBufferSlowIndex = 0;
            }
            if (m_indexMaxBufferSlow == m_maxBufferSlowIndex)
            {
                j = 1;
            }
            m_maxCurrentSection = m_pMaxBufferSlow[m_maxBufferSlowIndex];
            m_pMaxBufferSlow[m_maxBufferSlowIndex] = 0;  /* zero out the value representing the new section */

            /* compute the maximum over all the section */
            if (j)
            {
                m_maxMaxBufferSlow = 0;
                for (j = 0; j < m_nbrMaxBufferSection; j++)
                {
                    if (m_pMaxBufferSlow[j] > m_maxMaxBufferSlow)
                    {
                        m_maxMaxBufferSlow = m_pMaxBufferSlow[j];
                        m_indexMaxBufferSlow = j;
                    }
                }
            }
            else
            {
                if (tmp > m_maxMaxBufferSlow)
                {
                    m_maxMaxBufferSlow = tmp;
                    m_indexMaxBufferSlow = m_maxBufferSlowIndex;
                }
            }
            
            m_maxBufferSectionIndex += m_sectionLen;
        }
        
        if (m_maxBufferIndex >= (m_attack+1))
        {
            m_maxBufferIndex = 0;
            m_maxBufferSectionIndex = 0;
        }
        
        /* needed current gain */
        if (maximum > m_threshold)
        {
            gain = POINT_DIV_2(m_threshold, maximum);
        }
        else
        {
            gain = m_fVal_1p0;
        }
        
        /*avoid overshoot */
 
        if (gain < m_smoothState) {
			POINT_T fVal = POINT_MUL_2(m_fVal_0p1, m_smoothState);
            fVal = gain - fVal;
            fVal = POINT_MUL_2(fVal, m_fVal_1p11111111);
            m_fadedGain = min( m_fadedGain, fVal );
        }
        else 
        {
            m_fadedGain = gain;
        }


        /* smoothing gain */
        if (m_fadedGain < m_smoothState)
        {
            m_smoothState = POINT_MUL_2(m_attackConst, (m_smoothState - m_fadedGain)) + m_fadedGain;  /* m_attack */
            /*avoid overshoot */
            if (gain > m_smoothState)
            {
                m_smoothState = gain;
            }
        }
        else
        {
            m_smoothState = POINT_MUL_2(m_releaseConst, (m_smoothState - m_fadedGain)) + m_fadedGain; /* release */
        }
        
        /* fill delay line, apply gain */
        for (j = 0; j < m_channels; j++)
        {
            tmp = m_pDelayBuffer[m_delayBufferIndex * m_channels + j];
            m_pDelayBuffer[m_delayBufferIndex * m_channels + j] = samples[j][i];
            
            tmp = POINT_MUL_2(tmp, m_smoothState);
            if (tmp > m_threshold) tmp = m_threshold;
            if (tmp < -m_threshold) tmp = -m_threshold;
            
            samples[j][i] = tmp;
        }
        
        m_delayBufferIndex++;
        if (m_delayBufferIndex >= m_attack)
            m_delayBufferIndex = 0;
        
    }
    
    return LIMITER_OK;
}

/* get delay in samples */
int getLimiterDelay()
{
	return m_attack;
}

/* get m_attack in Ms */
float getLimiterAttack()
{
	return m_attackMs;
}

/* get delay in samples */
int getLimiterSampleRate()
{
	return m_sampleRate;
}

/* get delay in samples */
float getLimiterRelease()
{
	return m_releaseMs;
}
 
/* get maximum gain reduction of last processed block */
POINT_T getLimiterMaxGainReduction()
{
	return -20 * (POINT_T)log10(m_smoothState);
}

/* set number of channels */
int setLimiterNChannels(int nChannelsIn)
{
	if (nChannelsIn > m_maxChannels) return LIMITER_INVALID_PARAMETER;

	m_channels = nChannelsIn;
	resetLimiter();

	return LIMITER_OK;
}

/* set sampling rate */
int setLimiterSampleRate(int sampleRateIn)
{
	if (sampleRateIn > m_maxSampleRate) return LIMITER_INVALID_PARAMETER;

	/* update m_attack/release constants */
	m_attack = (int)(m_attackMs * sampleRateIn / 1000);

	if (m_attack < 1) /* m_attack time is too short */
		m_attack = 1;

	/* length of m_pMaxBuffer sections */
	m_sectionLen = (int)sqrt((float)m_attack+1);

	m_nbrMaxBufferSection    = (m_attack+1)/m_sectionLen;
	if (m_nbrMaxBufferSection*m_sectionLen < (m_attack+1))
		m_nbrMaxBufferSection++;
    m_attackConst   = FIXED_CONV( DS_POW(0.1, 1.0 / (m_attack + 1)) );
	m_releaseConst  = FIXED_CONV( DS_POW(0.1, 1.0 / (m_releaseMs * sampleRateIn / 1000 + 1)) );
	m_sampleRate    = sampleRateIn;

	/* reset */
	resetLimiter();

	return LIMITER_OK;
}

/* set m_attack time */
int setLimiterAttack(float attackMsIn)
{
	if (attackMsIn > m_maxAttackMs) return LIMITER_INVALID_PARAMETER;

	/* calculate attack time in samples */
	m_attack = (int)(attackMsIn * m_sampleRate / 1000);

	if (m_attack < 1) /* attack time is too short */
		m_attack=1;

	/* length of m_pMaxBuffer sections */
	m_sectionLen = (int)sqrt((float)m_attack+1);

	m_nbrMaxBufferSection   = (m_attack+1)/m_sectionLen;
	if (m_nbrMaxBufferSection*m_sectionLen < (m_attack+1))
	m_nbrMaxBufferSection++;
    m_attackConst  = FIXED_CONV( DS_POW(0.1, 1.0 / (m_attack + 1)) );
	m_attackMs     = attackMsIn;

	/* reset */
	resetLimiter();

	return LIMITER_OK;
}

/* set release time */
int setLimiterRelease(float releaseMsIn)
{
    m_releaseConst = FIXED_CONV( DS_POW(0.1, 1.0 / (releaseMsIn * m_sampleRate / 1000 + 1)) );
	m_releaseMs = releaseMsIn;

	return LIMITER_OK;
}

/* set limiter threshold */
int setLimiterThreshold(float thresholdIn)
{
	m_threshold = FIXED_CONV(thresholdIn);

	return LIMITER_OK;
}

/* set limiter threshold */
POINT_T getLimiterThreshold()
{
	return m_threshold;
}
