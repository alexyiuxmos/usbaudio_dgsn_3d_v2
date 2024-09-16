#ifndef __DSPOINT_H__
#define __DSPOINT_H__

#include <dsp_vector.h>

#define FIXED_POINT_32_TYPE
// #define FIXED_POINT_16_TYPE

#define F0(N) F ## N
#define F(N) F0(N)

#define Q0(N) Q ## N
#define Q(N) Q0(N)

// Convert from floating point to fixed point Q format.
// The number indicates the fractional bits or the position of the binary point
#if defined(FIXED_POINT_32_TYPE)

#define Q31(f) (int)((signed long long)((f) * ((unsigned long long)1 << (31+20)) + (1<<19)) >> 20)
#define Q30(f) (int)((signed long long)((f) * ((unsigned long long)1 << (30+20)) + (1<<19)) >> 20)
#define Q29(f) (int)((signed long long)((f) * ((unsigned long long)1 << (29+20)) + (1<<19)) >> 20)
#define Q28(f) (int)((signed long long)((f) * ((unsigned long long)1 << (28+20)) + (1<<19)) >> 20)
#define Q27(f) (int)((signed long long)((f) * ((unsigned long long)1 << (27+20)) + (1<<19)) >> 20)
#define Q26(f) (int)((signed long long)((f) * ((unsigned long long)1 << (26+20)) + (1<<19)) >> 20)
#define Q25(f) (int)((signed long long)((f) * ((unsigned long long)1 << (25+20)) + (1<<19)) >> 20)
#define Q24(f) (int)((signed long long)((f) * ((unsigned long long)1 << (24+20)) + (1<<19)) >> 20)
#define Q23(f) (int)((signed long long)((f) * ((unsigned long long)1 << (23+20)) + (1<<19)) >> 20)
#define Q22(f) (int)((signed long long)((f) * ((unsigned long long)1 << (22+20)) + (1<<19)) >> 20)
#define Q21(f) (int)((signed long long)((f) * ((unsigned long long)1 << (21+20)) + (1<<19)) >> 20)
#define Q20(f) (int)((signed long long)((f) * ((unsigned long long)1 << (20+20)) + (1<<19)) >> 20)
#define Q19(f) (int)((signed long long)((f) * ((unsigned long long)1 << (19+20)) + (1<<19)) >> 20)
#define Q18(f) (int)((signed long long)((f) * ((unsigned long long)1 << (18+20)) + (1<<19)) >> 20)
#define Q17(f) (int)((signed long long)((f) * ((unsigned long long)1 << (17+20)) + (1<<19)) >> 20)
#define Q16(f) (int)((signed long long)((f) * ((unsigned long long)1 << (16+20)) + (1<<19)) >> 20)
#define Q15(f) (int)((signed long long)((f) * ((unsigned long long)1 << (15+20)) + (1<<19)) >> 20)
#define Q14(f) (int)((signed long long)((f) * ((unsigned long long)1 << (14+20)) + (1<<19)) >> 20)
#define Q13(f) (int)((signed long long)((f) * ((unsigned long long)1 << (13+20)) + (1<<19)) >> 20)
#define Q12(f) (int)((signed long long)((f) * ((unsigned long long)1 << (12+20)) + (1<<19)) >> 20)
#define Q11(f) (int)((signed long long)((f) * ((unsigned long long)1 << (11+20)) + (1<<19)) >> 20)
#define Q10(f) (int)((signed long long)((f) * ((unsigned long long)1 << (10+20)) + (1<<19)) >> 20)
#define Q9(f)  (int)((signed long long)((f) * ((unsigned long long)1 << (9+20)) + (1<<19)) >> 20)
#define Q8(f)  (int)((signed long long)((f) * ((unsigned long long)1 << (8+20)) + (1<<19)) >> 20)

#elif defined(FIXED_POINT_16_TYPE)

#define Q15(f) (short)((signed long)((f) * ((unsigned long)1 << (15+15)) + (1<<14)) >> 15)
#define Q14(f) (short)((signed long)((f) * ((unsigned long)1 << (14+15)) + (1<<14)) >> 15)
#define Q13(f) (short)((signed long)((f) * ((unsigned long)1 << (13+15)) + (1<<14)) >> 15)
#define Q12(f) (short)((signed long)((f) * ((unsigned long)1 << (12+15)) + (1<<14)) >> 15)
#define Q11(f) (short)((signed long)((f) * ((unsigned long)1 << (11+15)) + (1<<14)) >> 15)
#define Q10(f) (short)((signed long)((f) * ((unsigned long)1 << (10+15)) + (1<<14)) >> 15)
#define Q9(f)  (short)((signed long)((f) * ((unsigned long)1 << (9+15)) + (1<<14)) >> 15)
#define Q8(f)  (short)((signed long)((f) * ((unsigned long)1 << (8+15)) + (1<<14)) >> 15)

#endif

// Convert from fixed point to double precision floating point
// The number indicates the fractional bits or the position of the binary point
#if defined(FIXED_POINT_32_TYPE)
#define F31(x) ((double)(x)/(double)(uint32_t)(1<<31)) // needs uint32_t cast because bit 31 is 1
#define F30(x) ((double)(x)/(double)(1<<30))
#define F29(x) ((double)(x)/(double)(1<<29))
#define F28(x) ((double)(x)/(double)(1<<28))
#define F27(x) ((double)(x)/(double)(1<<27))
#define F26(x) ((double)(x)/(double)(1<<26))
#define F25(x) ((double)(x)/(double)(1<<25))
#define F24(x) ((double)(x)/(double)(1<<24))
#define F23(x) ((double)(x)/(double)(1<<23))
#define F22(x) ((double)(x)/(double)(1<<22))
#define F21(x) ((double)(x)/(double)(1<<21))
#define F20(x) ((double)(x)/(double)(1<<20))
#define F19(x) ((double)(x)/(double)(1<<19))
#define F18(x) ((double)(x)/(double)(1<<18))
#define F17(x) ((double)(x)/(double)(1<<17))
#define F16(x) ((double)(x)/(double)(1<<16))
#endif

// short
#define F15(x) ((double)(x)/(double)(1<<15))
#define F14(x) ((double)(x)/(double)(1<<14))
#define F13(x) ((double)(x)/(double)(1<<13))
#define F12(x) ((double)(x)/(double)(1<<12))
#define F11(x) ((double)(x)/(double)(1<<11))
#define F10(x) ((double)(x)/(double)(1<<10))
#define F9(x)  ((double)(x)/(double)(1<<9))
#define F8(x)  ((double)(x)/(double)(1<<8))

/*
FIXED_CONV  - 우측 인자의 data type(연산식일 경우 최종 결과값)은 부동소수점 형식(float or double) 이어야 함
FLOAT_CONV  - 우측 인자의 data type(연산식일 경우 최종 결과값)은 POINT_T 형식 이어야 함
POINT_MUL_2 - 우측 인자의 data type(연산식일 경우 최종 결과값)은 A, B 모두 POINT_T 형식 이어야 함
POINT_DIV_2 - 우측 인자의 data type(연산식일 경우 최종 결과값)은 A, B 모두 POINT_T 형식 이어야 함
*/

#if !defined(USE_FIXED_POINT)

    typedef float POINT_T;
    typedef float * PPOINT_T;

    #define FIXED_CONV							// 부동소수점을 사용할 경우 변환없이 typedef된 POINT_T 값 그대로 사용
    #define FLOAT_CONV							// 부동소수점을 사용할 경우 변환없이 typedef된 POINT_T 값 그대로 사용
    #define POINT_MUL_2(A, B)   ((A) * (B))		// 결과:A x B, A:입력값1, B:입력값2
    #define POINT_DIV_2(A, B)   ((A) / (B))		// 결과:A / B, A:입력값1, B:입력값2

#else

	#if defined(FIXED_POINT_32_TYPE)

        #define QVAL     16	// range: 8~31, float 형 가수 길이 사용
                            // range별 float 최대값
                            // 31:    0.99999999930151, 30:    1.99999999860302, 29:    3.99999999720603, 28:    7.99999999441206
                            // 27:   15.99999998882410, 26:   31.99999997764830, 25:   63.99999995529650, 24:  127.99999991059300
                            // 23:  255.99999982118600, 22:  511.99999964237200, 21: 1023.99999928474000, 20: 2047.99999856949000

        typedef int32_t POINT_T;
        typedef int32_t * PPOINT_T;
        typedef int64_t EX_POINT_T;

	#elif defined(FIXED_POINT_16_TYPE)

        #define QVAL    14	// range: 8~15, 
                            // range별 float 최대값
                            // 15:  0.99995422363281, 14:  1.99990844726562, 13:  3.99981689453125, 12:  7.99963378906250
                            // 11: 15.99926757812500, 10: 31.99853515625000, 9:  63.99707031250000, 8: 127.99414062500000


        typedef int16_t POINT_T;
        typedef int16_t * PPOINT_T;
        typedef int32_t EX_POINT_T;

	#endif

    #define FIXED_CONV			Q(QVAL)		// 고정소수점을 사용할 경우 float -> 고정소수점 변환 매크로 함수사용
    #define FLOAT_CONV			F(QVAL)		// 고정소수점을 사용할 경우 고정소수점 -> float 변환 매크로 함수 사용

    #define POINT_MUL_2(A, B)	((POINT_T)(((((EX_POINT_T)(A)) * (B)) + (1<<(QVAL-1))) >> QVAL))	// 결과:A x B, A:입력값1, B:입력값2
    #define POINT_DIV_2(A, B)	((POINT_T)((((EX_POINT_T)(A)) * (1<<QVAL)) / (B)))					// 결과:A / B, A:입력값1, B:입력값2

#endif

#endif  // __DSPOINT_H__
