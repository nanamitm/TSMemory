#ifndef IDCT_REFERENCE_SSE2_H
#define IDCT_REFERENCE_SSE2_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IDCT_REFERENCE_SSE2_C
extern void __stdcall idct_reference_sse2(short *block);
#endif

#ifdef __cplusplus
}
#endif
	
#endif