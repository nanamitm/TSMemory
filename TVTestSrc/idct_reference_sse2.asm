;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; お約束
.586
.mmx
.xmm
.model flat
_TEXT64 segment page public use32 'CODE'
		align 16

;-------------------------------------------------------------------
; IEEE1180 result (itr=10000, min=-300, max=299)
; Mean Error:
;     0.00000  0.00010  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00000 -0.00010  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00010  0.00000  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00000  0.00000 -0.00010  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00010  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000
;     0.00010  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000
;   overall ME:   0.00000313
;   worst ME:   0.00010000
;
; Mean Square Error:
;     0.00000  0.00010  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00010  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00010  0.00000  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00010  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00010  0.00000  0.00000  0.00000
;     0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000
;     0.00010  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000  0.00000
;   overall MSE:   0.00000937
;   worst MSE:   0.00010000
;
; Peek Error
;       0    1    0    0    0    0    0    0
;       0    0    0    0    0    0    0    0
;       0    0    0    0    1    0    0    0
;       0    0    0    1    0    0    0    0
;       0    0    0    0    0    0    1    0
;       0    0    0    0    1    0    0    0
;       0    0    0    0    0    0    0    0
;       1    0    0    0    0    0    0    0
;   worst peek error: 1
;-------------------------------------------------------------------
; 等価疑似コード
; void idct_reference_nosimd(short *coef)
; {
; 	int i,j,k;
; 	float src[4];
; 	float sum[4*4];
; 	float buf[8*8];
; 	
; 	{ // 垂直方向 1D idct + 転置 (入力の左半分 -> 右半分の順で処理)
; 		const short *sp;
; 		const float *tp;
; 		float *dp;
; 		sp = coef;
; 		dp = buf;
; 		for (i=0; i<2; i++) {
; 			tp = cos_table;
; 			for (j=0; j<2; j++) {
; 				zero_fill(sum, 4*4);
; 				for (k=0; k<8; k++) {
; 					load_4_short(src, sp);
; 					sp += 8;
; 					fma_4(sum+ 0, src, tp[0]);
; 					fma_4(sum+ 4, src, tp[1]);
; 					fma_4(sum+ 8, src, tp[2]);
; 					fma_4(sum+12, src, tp[3]);
; 					tp += 4;
; 				}
; 				transpose_4x4(sum);
; 				store_4_float(dp+ 0, sum+ 0);
; 				store_4_float(dp+ 8, sum+ 4);
; 				store_4_float(dp+16, sum+ 8);
; 				store_4_float(dp+24, sum+12);
; 				sp -= (8*8); // 入力を先頭に巻き戻す
; 				dp += 4; // 出力を右のブロックに進める
; 			}
; 			sp += 4; // 入力を右のブロックに進める
; 			dp += (4*8-8); // 出力を下のブロックに進める
; 		}
; 	}
;
; 	{ // 水平方向 1D idct + 転置 + clip (出力の上半分 -> 下半分の順で処理)
; 		const float *sp;
; 		const float *tp;
; 		short *dp;
; 		sp = buf;
; 		dp = coef;
; 		for (i=0; i<2; i++) {
; 			tp = cos_table;
; 			for (j=0; j<2; j++) {
; 				zero_fill(sum, 4*4);
; 				for (k=0; k<8; k++) {
; 					load_4_float(src, sp);
; 					sp += 8;
; 					fma_4(sum+ 0, src, tp[0]);
; 					fma_4(sum+ 4, src, tp[1]);
; 					fma_4(sum+ 8, src, tp[2]);
; 					fma_4(sum+12, src, tp[3]);
; 					tp += 4;
; 				}
; 				transpose_4x4(sum);
; 				store_4_short_with_clip(dp+ 0, sum+ 0);
; 				store_4_short_with_clip(dp+ 8, sum+ 4);
; 				store_4_short_with_clip(dp+16, sum+ 8);
; 				store_4_short_with_clip(dp+24, sum+12);
; 				sp -= (8*8); // 入力を先頭に巻き戻す
; 				dp += 4; // 出力を右のブロックに進める
; 			}
; 			sp += 4; // 入力を右のブロックに進める
; 			dp += (4*8-8); // 出力を下のブロックに進める
; 		}
; 	}
; }
;-------------------------------------------------------------------
; 定数
cos_table      dd 03eb504f3h ; 0.353553f : sqrt(0.125) * cos((PI/8.0)*0*(0+0.5));
               dd 03eb504f3h ; 0.353553f : sqrt(0.125) * cos((PI/8.0)*0*(1+0.5));
               dd 03eb504f3h ; 0.353553f : sqrt(0.125) * cos((PI/8.0)*0*(2+0.5));
               dd 03eb504f3h ; 0.353553f : sqrt(0.125) * cos((PI/8.0)*0*(3+0.5));
  
               dd 03efb14beh ; 0.490393f :         0.5 * cos((PI/8.0)*1*(0+0.5));
               dd 03ed4db31h ; 0.415735f :         0.5 * cos((PI/8.0)*1*(1+0.5));
               dd 03e8e39dah ; 0.277785f :         0.5 * cos((PI/8.0)*1*(2+0.5));
               dd 03dc7c5c2h ; 0.097545f :         0.5 * cos((PI/8.0)*1*(3+0.5));
  
               dd 03eec835eh ; 0.461940f :         0.5 * cos((PI/8.0)*2*(0+0.5));
               dd 03e43ef15h ; 0.191342f :         0.5 * cos((PI/8.0)*2*(1+0.5));
               dd 0be43ef15h ;-0.191342f :         0.5 * cos((PI/8.0)*2*(2+0.5));
               dd 0beec835eh ;-0.461940f :         0.5 * cos((PI/8.0)*2*(3+0.5));
  
               dd 03ed4db31h ; 0.415735f :         0.5 * cos((PI/8.0)*3*(0+0.5));
               dd 0bdc7c5c2h ;-0.097545f :         0.5 * cos((PI/8.0)*3*(1+0.5));
               dd 0befb14beh ;-0.490393f :         0.5 * cos((PI/8.0)*3*(2+0.5));
               dd 0be8e39dah ;-0.277785f :         0.5 * cos((PI/8.0)*3*(3+0.5));
  
               dd 03eb504f3h ; 0.353553f :         0.5 * cos((PI/8.0)*4*(0+0.5));
               dd 0beb504f3h ;-0.353553f :         0.5 * cos((PI/8.0)*4*(1+0.5));
               dd 0beb504f3h ;-0.353553f :         0.5 * cos((PI/8.0)*4*(2+0.5));
               dd 03eb504f3h ; 0.353553f :         0.5 * cos((PI/8.0)*4*(3+0.5));
  
               dd 03e8e39dah ; 0.277785f :         0.5 * cos((PI/8.0)*5*(0+0.5));
               dd 0befb14beh ;-0.490393f :         0.5 * cos((PI/8.0)*5*(1+0.5));
               dd 03dc7c5c2h ; 0.097545f :         0.5 * cos((PI/8.0)*5*(2+0.5));
               dd 03ed4db31h ; 0.415735f :         0.5 * cos((PI/8.0)*5*(3+0.5));
  
               dd 03e43ef15h ; 0.191342f :         0.5 * cos((PI/8.0)*6*(0+0.5));
               dd 0beec835eh ;-0.461940f :         0.5 * cos((PI/8.0)*6*(1+0.5));
               dd 03eec835eh ; 0.461940f :         0.5 * cos((PI/8.0)*6*(2+0.5));
               dd 0be43ef15h ;-0.191342f :         0.5 * cos((PI/8.0)*6*(3+0.5));
  
               dd 03dc7c5c2h ; 0.097545f :         0.5 * cos((PI/8.0)*7*(0+0.5));
               dd 0be8e39dah ;-0.277785f :         0.5 * cos((PI/8.0)*7*(1+0.5));
               dd 03ed4db31h ; 0.415735f :         0.5 * cos((PI/8.0)*7*(2+0.5));
               dd 0befb14beh ;-0.490393f :         0.5 * cos((PI/8.0)*7*(3+0.5));
  
               dd 03eb504f3h ; 0.353553f : sqrt(0.125) * cos((PI/8.0)*0*(4+0.5));
               dd 03eb504f3h ; 0.353553f : sqrt(0.125) * cos((PI/8.0)*0*(5+0.5));
               dd 03eb504f3h ; 0.353553f : sqrt(0.125) * cos((PI/8.0)*0*(6+0.5));
               dd 03eb504f3h ; 0.353553f : sqrt(0.125) * cos((PI/8.0)*0*(7+0.5));
  
               dd 0bdc7c5c2h ;-0.097545f :         0.5 * cos((PI/8.0)*1*(4+0.5));
               dd 0be8e39dah ;-0.277785f :         0.5 * cos((PI/8.0)*1*(5+0.5));
               dd 0bed4db31h ;-0.415735f :         0.5 * cos((PI/8.0)*1*(6+0.5));
               dd 0befb14beh ;-0.490393f :         0.5 * cos((PI/8.0)*1*(7+0.5));
  
               dd 0beec835eh ;-0.461940f :         0.5 * cos((PI/8.0)*2*(4+0.5));
               dd 0be43ef15h ;-0.191342f :         0.5 * cos((PI/8.0)*2*(5+0.5));
               dd 03e43ef15h ; 0.191342f :         0.5 * cos((PI/8.0)*2*(6+0.5));
               dd 03eec835eh ; 0.461940f :         0.5 * cos((PI/8.0)*2*(7+0.5));
                                        
               dd 03e8e39dah ; 0.277785f :         0.5 * cos((PI/8.0)*3*(4+0.5));
               dd 03efb14beh ; 0.490393f :         0.5 * cos((PI/8.0)*3*(5+0.5));
               dd 03dc7c5c2h ; 0.097545f :         0.5 * cos((PI/8.0)*3*(6+0.5));
               dd 0bed4db31h ;-0.415735f :         0.5 * cos((PI/8.0)*3*(7+0.5));
                                        
               dd 03eb504f3h ; 0.353553f :         0.5 * cos((PI/8.0)*4*(4+0.5));
               dd 0beb504f3h ;-0.353553f :         0.5 * cos((PI/8.0)*4*(5+0.5));
               dd 0beb504f3h ;-0.353553f :         0.5 * cos((PI/8.0)*4*(6+0.5));
               dd 03eb504f3h ; 0.353553f :         0.5 * cos((PI/8.0)*4*(7+0.5));
                                        
               dd 0bed4db31h ;-0.415735f :         0.5 * cos((PI/8.0)*5*(4+0.5));
               dd 0bdc7c5c2h ;-0.097545f :         0.5 * cos((PI/8.0)*5*(5+0.5));
               dd 03efb14beh ; 0.490393f :         0.5 * cos((PI/8.0)*5*(6+0.5));
               dd 0be8e39dah ;-0.277785f :         0.5 * cos((PI/8.0)*5*(7+0.5));
                                        
               dd 0be43ef15h ;-0.191342f :         0.5 * cos((PI/8.0)*6*(4+0.5));
               dd 03eec835eh ; 0.461940f :         0.5 * cos((PI/8.0)*6*(5+0.5));
               dd 0beec835eh ;-0.461940f :         0.5 * cos((PI/8.0)*6*(6+0.5));
               dd 03e43ef15h ; 0.191342f :         0.5 * cos((PI/8.0)*6*(7+0.5));
                                        
               dd 03efb14beh ; 0.490393f :         0.5 * cos((PI/8.0)*7*(4+0.5));
               dd 0bed4db31h ;-0.415735f :         0.5 * cos((PI/8.0)*7*(5+0.5));
               dd 03e8e39dah ; 0.277785f :         0.5 * cos((PI/8.0)*7*(6+0.5));
               dd 0bdc7c5c2h ;-0.097545f :         0.5 * cos((PI/8.0)*7*(7+0.5));

half_table     dd 03f000000h ; 0.500000f 
               dd 03f000000h ; 0.500000f 
               dd 03f000000h ; 0.500000f 
               dd 03f000000h ; 0.500000f 

min_max_table  dw -256;
               dw -256;
               dw -256;
               dw -256;
               dw -256;
               dw -256;
               dw -256;
               dw -256;

               dw  255;
               dw  255;
               dw  255;
               dw  255;
               dw  255;
               dw  255;
               dw  255;
               dw  255;

;-------------------------------------------------------------------
PUBLIC              C _idct_reference_sse2@4
;      void __stdcall  idct_reference_sse2(
; [esp + 4] = short *block,
; )
_idct_reference_sse2@4 PROC
;-------------------------------------------------------------------
; レジスタ退避
                push       esi
                push       edi
                push       eax
                push       ebx

;-------------------------------------------------------------------
; 引数からのデータ受け取り
                mov        esi, [esp+16+4]
;-------------------------------------------------------------------
; ローカル変数領域確保
                mov        edi, esp;
                sub        esp, 272; 8*8*sizeof(float) + 4*sizeof(int32_t)
                and        esp, 0fffffff0h; 16byte align
                mov        [esp+256], edi; base stack ptr
                mov        [esp+260], esi; block ptr
;-------------------------------------------------------------------
; 垂直 IDCT
                mov        ebx, esp;
                mov        edi, 2;
idct_vertical_level_0_head:
                mov        eax, 2;
                mov        [esp+268], edi; loop counter 'i'
                lea        edi, cos_table;
;-------------------------------------------------------------------
; core
idct_vertical_level_1_head:
                mov        [esp+264], eax; loop counter 'j'
                xorps      xmm0, xmm0; zero clear
                xorps      xmm1, xmm1;
                xorps      xmm2, xmm2;
                xorps      xmm3, xmm3;
                mov        eax, 8; loop counter 'k'
idct_vertical_level_2_head:
                movq       xmm6, qword ptr [esi];
                movaps     xmm7, [edi];
                lea        esi, [esi+16]; 16=8*sizeof(short)
                lea        edi, [edi+16]; 16=4*sizeof(float)
                punpcklwd  xmm6, xmm6;
                psrad      xmm6, 16;
                cvtdq2ps   xmm6, xmm6;
                movaps     xmm4, xmm7;
                movaps     xmm5, xmm7;
                shufps     xmm4, xmm4, 00000000b; all cos_table[k*4+0]
                shufps     xmm5, xmm5, 01010101b; all cos_table[k*4+1]
                mulps      xmm4, xmm6;
                mulps      xmm5, xmm6;
                addps      xmm0, xmm4;
                addps      xmm1, xmm5;
                movaps     xmm4, xmm7;
                shufps     xmm7, xmm7, 10101010b; all cos_table[k*4+2]
                shufps     xmm4, xmm4, 11111111b; all cos_table[k*4+3]
                mulps      xmm7, xmm6;
                mulps      xmm6, xmm4;
                addps      xmm2, xmm7;
                addps      xmm3, xmm6;
                dec        eax;
                jnz idct_vertical_level_2_head;
                movaps     xmm4, xmm0;
                movaps     xmm6, xmm2;
                unpcklps   xmm0, xmm1; 11_01_10_00
                unpcklps   xmm2, xmm3; 31_21_30_20
                unpckhps   xmm4, xmm1; 13_03_12_02
                unpckhps   xmm6, xmm3; 33_23_32_22
                movaps     xmm1, xmm0;
                movaps     xmm3, xmm4;
                movlhps    xmm0, xmm2; 30_20_10_00
                movhlps    xmm2, xmm1; 31_21_11_01
                movlhps    xmm4, xmm6; 32_22_12_02
                movhlps    xmm6, xmm3; 33_23_13_03
                movaps     [ebx+ 0], xmm0;
                movaps     [ebx+32], xmm2;
                movaps     [ebx+64], xmm4;
                movaps     [ebx+96], xmm6;
                mov        eax, [esp+264]; loop counter 'j'
                lea        ebx, [ebx+16];
                sub        esi, 128; 120=(8*8)*sizeof(short)
                dec        eax;
                jnz idct_vertical_level_1_head;
                lea        ebx, [ebx+96]; 96=((4-1)*8)*sizeof(float)
                add        esi, 8; 8=4*sizeof(short)
                mov        edi, [esp+268]; loop counter 'i'
                dec        edi;
                jnz idct_vertical_level_0_head;

;-------------------------------------------------------------------
; 水平 IDCT
                mov        esi, esp;
                mov        ebx, [esp+260]; // block ptr
                mov        edi, 2;
idct_horizontal_level_0_head:
                mov        eax, 2;
                mov        [esp+268], edi; loop counter 'i'
                lea        edi, cos_table;
;-------------------------------------------------------------------
; core
idct_horizontal_level_1_head:
                mov        [esp+264], eax; loop counter 'j'
                xorps      xmm0, xmm0; zero clear
                xorps      xmm1, xmm1;
                xorps      xmm2, xmm2;
                xorps      xmm3, xmm3;
                mov        eax, 8; loop counter 'k'
idct_horizontal_level_2_head: 
                movaps     xmm6, [esi];
                movaps     xmm7, [edi];
                lea        esi, [esi+32]; 32=8*sizeof(float)
                lea        edi, [edi+16]; 16=4*sizeof(float)
                movaps     xmm4, xmm7;
                movaps     xmm5, xmm7;
                shufps     xmm4, xmm4, 00000000b; all cos_table[k*4+0]
                shufps     xmm5, xmm5, 01010101b; all cos_table[k*4+1]
                mulps      xmm4, xmm6;
                mulps      xmm5, xmm6;
                addps      xmm0, xmm4;
                addps      xmm1, xmm5;
                movaps     xmm4, xmm7;
                shufps     xmm7, xmm7, 10101010b; all cos_table[k*4+2]
                shufps     xmm4, xmm4, 11111111b; all cos_table[k*4+3]
                mulps      xmm7, xmm6;
                mulps      xmm6, xmm4;
                addps      xmm2, xmm7;
                addps      xmm3, xmm6;
                dec        eax;
                jnz idct_horizontal_level_2_head;
                movaps     xmm7, half_table
                movaps     xmm4, xmm0;
                movaps     xmm6, xmm2;
                unpcklps   xmm0, xmm1; 11_01_10_00
                unpcklps   xmm2, xmm3; 31_21_30_20
                unpckhps   xmm4, xmm1; 13_03_12_02
                unpckhps   xmm6, xmm3; 33_23_32_22
                movaps     xmm1, xmm0;
                movaps     xmm3, xmm4;
                movlhps    xmm0, xmm2; 30_20_10_00
                movhlps    xmm2, xmm1; 31_21_11_01
                movlhps    xmm4, xmm6; 32_22_12_02
                movhlps    xmm6, xmm3; 33_23_13_03
                addps      xmm0, xmm7;
                addps      xmm2, xmm7;
                addps      xmm4, xmm7;
                addps      xmm6, xmm7;
                lea        eax, min_max_table;
                ; convert to int with floor
                cvtps2dq   xmm1, xmm0;
                cvtps2dq   xmm3, xmm2;
                cvtdq2ps   xmm5, xmm1;
                cvtdq2ps   xmm7, xmm3;
                cmpltps    xmm0, xmm5;
                cmpltps    xmm2, xmm7;
                paddd      xmm1, xmm0;
                paddd      xmm3, xmm2;
                cvtps2dq   xmm5, xmm4;
                cvtps2dq   xmm7, xmm6;
                cvtdq2ps   xmm0, xmm5;
                cvtdq2ps   xmm2, xmm7;
                cmpltps    xmm4, xmm0;
                cmpltps    xmm6, xmm2;
                paddd      xmm5, xmm4;
                paddd      xmm7, xmm6;
                ; convert to short + clip
                packssdw   xmm1, xmm5;
                packssdw   xmm3, xmm7;
                pmaxsw     xmm1, [eax+ 0];
                pmaxsw     xmm3, [eax+ 0];
                pminsw     xmm1, [eax+16];
                pminsw     xmm3, [eax+16];
                movq       qword ptr [ebx+ 0], xmm1;
                movq       qword ptr [ebx+16], xmm3;
                psrldq     xmm1, 8;
                psrldq     xmm3, 8;
                movq       qword ptr [ebx+32], xmm1;
                movq       qword ptr [ebx+48], xmm3;
                mov        eax, [esp+264]; loop counter 'j'
                lea        ebx, [ebx+8];
                sub        esi, 256; 256=(8*8)*sizeof(float)
                dec        eax;
                jnz idct_horizontal_level_1_head;
                lea        ebx, [ebx+48]; 48=((4-1)*8)*sizeof(short)
                add        esi, 16; 16=4*sizeof(float)
                mov        edi, [esp+268]; loop counter 'i'
                dec        edi;
                jnz idct_horizontal_level_0_head;
;-------------------------------------------------------------------
; 後始末
                mov        esp, [esp+256];
                pop        ebx
                pop        eax
                pop        edi
                pop        esi

                ret        4
;-------------------------------------------------------------------
_idct_reference_sse2@4 ENDP
;-------------------------------------------------------------------
; 終了

END
