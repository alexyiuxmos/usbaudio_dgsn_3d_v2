// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#if defined(__XS3A__)

/*  
void real_imag_split(
	int32_t * real_data, 
	int32_t * imag_data, 
	complex_s32_t * comp_data, 
	uint32_t comp_length);
*/


#define NSTACKWORDS     (2)

#define FUNCTION_NAME   real_imag_split

#define real_data_p         r0 
#define imag_data_p         r1 
#define comp_data_p         r2
#define comp_length         r3

#define re                r4
#define im                r5

.text
.issue_mode dual
.align 4

.cc_top FUNCTION_NAME.function,FUNCTION_NAME

FUNCTION_NAME:
    dualentsp NSTACKWORDS

    std r4, r5, sp[0]
    {nop; sub r3, r3, 1}
loop:
    {ldw re, r2[0]; add r2, r2, 4}
    {ldw im, r2[0]; add r2, r2, 4}
    {stw re, r0[0]; add r0, r0, 4}
    {stw im, r1[0]; add r1, r1, 4}
    {bt r3, loop; sub r3, r3, 1}

.L_finish:
        ldd r4, r5, sp[0]
    {                                           ; retsp NSTACKWORDS                     } 

.L_func_end:
.cc_bottom FUNCTION_NAME.function


.global FUNCTION_NAME
.type FUNCTION_NAME,@function
.set FUNCTION_NAME.nstackwords,NSTACKWORDS; .global FUNCTION_NAME.nstackwords
.set FUNCTION_NAME.maxcores,1;              .global FUNCTION_NAME.maxcores
.set FUNCTION_NAME.maxtimers,0;             .global FUNCTION_NAME.maxtimers
.set FUNCTION_NAME.maxchanends,0;           .global FUNCTION_NAME.maxchanends
.size FUNCTION_NAME, .L_func_end - FUNCTION_NAME



#endif //defined(__XS3A__)



