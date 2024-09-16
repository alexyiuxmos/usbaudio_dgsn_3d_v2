#ifndef __DSAUDIODEF_H__
#define __DSAUDIODEF_H__

#include "DSmath.h"

#define INT20_MIN   -524288
#define INT20_MAX   524287

#define INT24_MIN   -8388608
#define INT24_MAX   8388607

#define DSLIMITER_ATAN_THRESH 0.95f
#define DSLIMITER_ATAN_RANGE (1.0f - DSLIMITER_ATAN_THRESH)

#define DSLIMITER_ATAN(_x_) (DSLIMITER_ATAN_THRESH < _x_ ?		\
							 (DS_ATAN((_x_ - DSLIMITER_ATAN_THRESH)/DSLIMITER_ATAN_RANGE)/M_PI_2)*DSLIMITER_ATAN_RANGE + DSLIMITER_ATAN_THRESH :	\
							 (_x_ < -DSLIMITER_ATAN_THRESH ? -((DS_ATAN(-(_x_ + DSLIMITER_ATAN_THRESH)/DSLIMITER_ATAN_RANGE)/M_PI_2)*DSLIMITER_ATAN_RANGE + DSLIMITER_ATAN_THRESH) : _x_))

#define DS_DOUBLE_TO_PCM16_ATAN(_pi16PCM_, _pdbOffset_) *_pi16PCM_++ = (INT16)(DSLIMITER_ATAN( *_pdbOffset_ ) * INT16_MAX);	\
														  _pdbOffset_++

#define DS_DOUBLE_TO_PCM24_ATAN(_i32V_, _p24PCM_, _pdbOffset_) _i32V_ = (INT32)(DSLIMITER_ATAN( *_pdbOffset_ ) * INT24_MAX);	\
																*_p24PCM_++ = (BYTE)_i32V_; _i32V_ >>= 8;		\
																*_p24PCM_++ = (BYTE)_i32V_; _i32V_ >>= 8;		\
																*_p24PCM_++ = (BYTE)_i32V_; _pdbOffset_++

#define DS_DOUBLE_TO_FLOAT_ATAN(_pfAData_, _pdbOffset_) *_pfAData_++ = (float)DSLIMITER_ATAN( *_pdbOffset_ );	\
														  _pdbOffset_++

#define DS_DOUBLE_TO_PCM16_CLIPPING(_pi16PCM_, _pdbOffset_) *_pi16PCM_++ = (INT16)((*_pdbOffset_ > 1.0 ? 1.0 : (*_pdbOffset_ < -1.0 ? -1.0 : *_pdbOffset_)) * INT16_MAX);	\
														  _pdbOffset_++

#define DS_DOUBLE_TO_PCM24_CLIPPING(_i32V_, _p24PCM_, _pdbOffset_) _i32V_ = (INT32)((*_pdbOffset_ > 1.0 ? 1.0 : (*_pdbOffset_ < -1.0 ? -1.0 : *_pdbOffset_)) * INT24_MAX);	\
																*_p24PCM_++ = (BYTE)_i32V_; _i32V_ >>= 8;		\
																*_p24PCM_++ = (BYTE)_i32V_; _i32V_ >>= 8;		\
																*_p24PCM_++ = (BYTE)_i32V_; _pdbOffset_++

#define DS_DOUBLE_TO_FLOAT_CLIPPING(_pfAData_, _pdbOffset_) *_pfAData_++ = (float)(*_pdbOffset_ > 1.0 ? 1.0 : (*_pdbOffset_ < -1.0 ? -1.0 : *_pdbOffset_));	\
														  _pdbOffset_++

#define DS_DOUBLE_TO_PCM16(_pi16PCM_, _pdbOffset_) *_pi16PCM_++ = (INT16)(*_pdbOffset_++ * INT16_MAX)

#define DS_DOUBLE_TO_PCM24(_i32V_, _p24PCM_, _pdbOffset_) _i32V_ = (INT32)(*_pdbOffset_++ * INT24_MAX);	\
														  *_p24PCM_++ = (BYTE)_i32V_; _i32V_ >>= 8;		\
														  *_p24PCM_++ = (BYTE)_i32V_; _i32V_ >>= 8;		\
														  *_p24PCM_++ = (BYTE)_i32V_

#define DS_DOUBLE_TO_FLOAT(_pfAData_, _pdbOffset_) *_pfAData_++ = (float)*_pdbOffset_++

#define DS_PCM16_TO_DOUBLE(_pdbOffset_, _pi16PCM_) *_pdbOffset_++ = *_pi16PCM_++ / (float)INT16_MAX

#endif	// __DSAUDIODEF_H__