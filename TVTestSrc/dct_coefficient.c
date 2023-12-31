/*******************************************************************
                    dct coefficient module
 *******************************************************************/
#include "dct_coefficient.h"

typedef struct {
	signed   char flag;
	unsigned char run;
	signed   char level;
	unsigned char size;
} DCT_COEFFICIENT_VLC_ELEMENT;

static int get_dct_coefficient_escape_level_mpeg2(VIDEO_STREAM *in);
static int get_dct_coefficient_escape_level_mpeg1(VIDEO_STREAM *in);

static __inline int log2(int arg)
{
	__asm {
		xor eax, eax;
		bsr eax, arg;
		mov arg, eax;
	}

	return arg;
}

int read_dct_dc_coefficient_b14(VIDEO_STREAM *in, int *run, int *level)
{
	int n;
	int code;

	static const DCT_COEFFICIENT_VLC_ELEMENT table_a[] = {
		{1,1,18,17},{1,1,-18,17},{1,1,17,17},{1,1,-17,17}, /* 0000 0000 0001 000x x */
		{1,1,16,17},{1,1,-16,17},{1,1,15,17},{1,1,-15,17}, /* 0000 0000 0001 001x x */
		{1,6,3,17},{1,6,-3,17},{1,16,2,17},{1,16,-2,17},   /* 0000 0000 0001 010x x */
		{1,15,2,17},{1,15,-2,17},{1,14,2,17},{1,14,-2,17}, /* 0000 0000 0001 011x x */
		{1,13,2,17},{1,13,-2,17},{1,12,2,17},{1,12,-2,17}, /* 0000 0000 0001 100x x */
		{1,11,2,17},{1,11,-2,17},{1,31,1,17},{1,31,-1,17}, /* 0000 0000 0001 101x x */
		{1,30,1,17},{1,30,-1,17},{1,29,1,17},{1,29,-1,17}, /* 0000 0000 0001 110x x */
		{1,28,1,17},{1,28,-1,17},{1,27,1,17},{1,27,-1,17}, /* 0000 0000 0001 111x x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_b[] = {
		{1,0,40,16},{1,0,-40,16},{1,0,39,16},{1,0,-39,16}, /* 0000 0000 0010 00xx x */
		{1,0,38,16},{1,0,-38,16},{1,0,37,16},{1,0,-37,16}, /* 0000 0000 0010 01xx x */
		{1,0,36,16},{1,0,-36,16},{1,0,35,16},{1,0,-35,16}, /* 0000 0000 0010 10xx x */
		{1,0,34,16},{1,0,-34,16},{1,0,33,16},{1,0,-33,16}, /* 0000 0000 0010 11xx x */
		{1,0,32,16},{1,0,-32,16},{1,1,14,16},{1,1,-14,16}, /* 0000 0000 0011 00xx x */
		{1,1,13,16},{1,1,-13,16},{1,1,12,16},{1,1,-12,16}, /* 0000 0000 0011 01xx x */
		{1,1,11,16},{1,1,-11,16},{1,1,10,16},{1,1,-10,16}, /* 0000 0000 0011 10xx x */
		{1,1,9,16},{1,1,-9,16},{1,1,8,16},{1,1,-8,16},     /* 0000 0000 0011 11xx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_c[] = {
		{1,0,31,15},{1,0,-31,15},{1,0,30,15},{1,0,-30,15}, /* 0000 0000 0100 0xxx x */
		{1,0,29,15},{1,0,-29,15},{1,0,28,15},{1,0,-28,15}, /* 0000 0000 0100 1xxx x */
		{1,0,27,15},{1,0,-27,15},{1,0,26,15},{1,0,-26,15}, /* 0000 0000 0101 0xxx x */
		{1,0,25,15},{1,0,-25,15},{1,0,24,15},{1,0,-24,15}, /* 0000 0000 0101 0xxx x */
		{1,0,23,15},{1,0,-23,15},{1,0,22,15},{1,0,-22,15}, /* 0000 0000 0110 0xxx x */
		{1,0,21,15},{1,0,-21,15},{1,0,20,15},{1,0,-20,15}, /* 0000 0000 0110 1xxx x */
		{1,0,19,15},{1,0,-19,15},{1,0,18,15},{1,0,-18,15}, /* 0000 0000 0111 0xxx x */
		{1,0,17,15},{1,0,-17,15},{1,0,16,15},{1,0,-16,15}, /* 0000 0000 0111 0xxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_d[] = {
		{1,10,2,14},{1,10,-2,14},{1,9,2,14},{1,9,-2,14},   /* 0000 0000 1000 xxxx x */
		{1,5,3,14},{1,5,-3,14},{1,3,4,14},{1,3,-4,14},     /* 0000 0000 1001 xxxx x */
		{1,2,5,14},{1,2,-5,14},{1,1,7,14},{1,1,-7,14},     /* 0000 0000 1010 xxxx x */
		{1,1,6,14},{1,1,-6,14},{1,0,15,14},{1,0,-15,14},   /* 0000 0000 1011 xxxx x */
		{1,0,14,14},{1,0,-14,14},{1,0,13,14},{1,0,-13,14}, /* 0000 0000 1100 xxxx x */
		{1,0,12,14},{1,0,-12,14},{1,26,1,14},{1,26,-1,14}, /* 0000 0000 1101 xxxx x */
		{1,25,1,14},{1,25,-1,14},{1,24,1,14},{1,24,-1,14}, /* 0000 0000 1110 xxxx x */
		{1,23,1,14},{1,23,-1,14},{1,22,1,14},{1,22,-1,14}, /* 0000 0000 1110 xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_e[] = {
		{1,0,11,13},{1,0,-11,13},{1,8,2,13},{1,8,-2,13},   /* 0000 0001 000x xxxx x */
		{1,4,3,13},{1,4,-3,13},{1,0,10,13},{1,0,-10,13},   /* 0000 0001 001x xxxx x */
		{1,2,4,13},{1,2,-4,13},{1,7,2,13},{1,7,-2,13},     /* 0000 0001 010x xxxx x */
		{1,21,1,13},{1,21,-1,13},{1,20,1,13},{1,20,-1,13}, /* 0000 0001 011x xxxx x */
		{1,0,9,13},{1,0,-9,13},{1,19,1,13},{1,19,-1,13},   /* 0000 0001 100x xxxx x */
		{1,18,1,13},{1,18,-1,13},{1,1,5,13},{1,1,-5,13},   /* 0000 0001 101x xxxx x */
		{1,3,3,13},{1,3,-3,13},{1,0,8,13},{1,0,-8,13},     /* 0000 0001 110x xxxx x */
		{1,6,2,13},{1,6,-2,13},{1,17,1,13},{1,17,-1,13},   /* 0000 0001 111x xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_f[] = {
		{1,16,1,11},{1,16,-1,11},{1,5,2,11},{1,5,-2,11},   /* 0000 0010 0xxx xxxx x */
		{1,0,7,11},{1,0,-7,11},{1,2,3,11},{1,2,-3,11},     /* 0000 0010 1xxx xxxx x */
		{1,1,4,11},{1,1,-4,11},{1,15,1,11},{1,15,-1,11},   /* 0000 0011 0xxx xxxx x */
		{1,14,1,11},{1,14,-1,11},{1,4,2,11},{1,4,-2,11},   /* 0000 0011 1xxx xxxx x */
	};

	/* escape */                                               /* 0000 01xx xxxx xxxx x */

	static const DCT_COEFFICIENT_VLC_ELEMENT table_g[] = {
		{1,2,2,8},{1,2,-2,8},{1,9,1,8},{1,9,-1,8},         /* 0000 10xx xxxx xxxx x */
		{1,0,4,8},{1,0,-4,8},{1,8,1,8},{1,8,-1,8},         /* 0000 11xx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_h[] = {
		{1,7,1,7},{1,7,-1,7},{1,6,1,7},{1,6,-1,7},         /* 0001 0xxx xxxx xxxx x */
		{1,1,2,7},{1,1,-2,7},{1,5,1,7},{1,5,-1,7},         /* 0001 1xxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_i[] = {
		{1,13,1,9},{1,13,-1,9},{1,0,6,9},{1,0,-6,9},       /* 0010 000x xxxx xxxx x */
		{1,12,1,9},{1,12,-1,9},{1,11,1,9},{1,11,-1,9},     /* 0010 001x xxxx xxxx x */
		{1,3,2,9},{1,3,-2,9},{1,1,3,9},{1,1,-3,9},         /* 0010 010x xxxx xxxx x */
		{1,0,5,9},{1,0,-5,9},{1,10,1,9},{1,10,-1,9},       /* 0010 011x xxxx xxxx x */
		{1,0,3,6},{1,0,3,6},{1,0,3,6},{1,0,3,6},           /* 0010 100x xxxx xxxx x */
		{1,0,3,6},{1,0,3,6},{1,0,3,6},{1,0,3,6},           /* 0010 101x xxxx xxxx x */
		{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},       /* 0010 110x xxxx xxxx x */
		{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},       /* 0010 111x xxxx xxxx x */
		{1,4,1,6},{1,4,1,6},{1,4,1,6},{1,4,1,6},           /* 0011 000x xxxx xxxx x */
		{1,4,1,6},{1,4,1,6},{1,4,1,6},{1,4,1,6},           /* 0011 001x xxxx xxxx x */
		{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},       /* 0011 010x xxxx xxxx x */
		{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},       /* 0011 011x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 100x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 101x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 110x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 111x xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_j[] = {
		{1,0,2,5},{1,0,-2,5},{1,2,1,5},{1,2,-1,5},         /* 010x xxxx xxxx xxxx x */
		{1,1,1,4},{1,1,1,4},{1,1,-1,4},{1,1,-1,4},         /* 011x xxxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_k[] = {
		{1,0,1,2},{1,0,-1,2},                              /* 1xxx xxxx xxxx xxxx x */
	};

	code = vs_read_bits(in, 17);
	n = log2(code);

	switch(n){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		*run = 0;
		*level = 0;
		return -1;
	case 5:
		code -= 0x20;
		vs_erase_bits(in, table_a[code].size);
		*run = table_a[code].run;
		*level = table_a[code].level;
		return table_a[code].flag;
	case 6:
		code >>= 1;
		code -= 0x20;
		vs_erase_bits(in, table_b[code].size);
		*run = table_b[code].run;
		*level = table_b[code].level;
		return table_b[code].flag;
	case 7:
		code >>= 2;
		code -= 0x20;
		vs_erase_bits(in, table_c[code].size);
		*run = table_c[code].run;
		*level = table_c[code].level;
		return table_c[code].flag;
	case 8:
		code >>= 3;
		code -= 0x20;
		vs_erase_bits(in, table_d[code].size);
		*run = table_d[code].run;
		*level = table_d[code].level;
		return table_d[code].flag;
	case 9:
		code >>= 4;
		code -= 0x20;
		vs_erase_bits(in, table_e[code].size);
		*run = table_e[code].run;
		*level = table_e[code].level;
		return table_e[code].flag;
	case 10:
		code >>= 6;
		code -= 0x10;
		vs_erase_bits(in, table_f[code].size);
		*run = table_f[code].run;
		*level = table_f[code].level;
		return table_e[code].flag;
	case 11:
		/* escape */
		code >>= 5;
		code &= 0x3f;
		vs_erase_bits(in, 12);
		*run = code;
		*level = get_dct_coefficient_escape_level_mpeg2(in);
		return 1;
	case 12:
		code >>= 9;
		code -= 0x08;
		vs_erase_bits(in, table_g[code].size);
		*run = table_g[code].run;
		*level = table_g[code].level;
		return table_g[code].flag;
	case 13:
		code >>= 10;
		code -= 0x08;
		vs_erase_bits(in, table_h[code].size);
		*run = table_h[code].run;
		*level = table_h[code].level;
		return table_h[code].flag;
	case 14:
		code >>= 8;
		code -= 0x40;
		vs_erase_bits(in, table_i[code].size);
		*run = table_i[code].run;
		*level = table_i[code].level;
		return table_i[code].flag;
	case 15:
		code >>= 12;
		code -= 0x08;
		vs_erase_bits(in, table_j[code].size);
		*run = table_j[code].run;
		*level = table_j[code].level;
		return table_j[code].flag;
	case 16:
		code >>= 15;
		code -= 0x2;
		vs_erase_bits(in, table_k[code].size);
		*run = table_k[code].run;
		*level = table_k[code].level;
		return table_k[code].flag;
	}

	*run = 0;
	*level = 0;
	return -1;
}

int read_dct_dc_coefficient_mpeg1(VIDEO_STREAM *in, int *run, int *level)
{
	int n;
	int code;

	static const DCT_COEFFICIENT_VLC_ELEMENT table_a[] = {
		{1,1,18,17},{1,1,-18,17},{1,1,17,17},{1,1,-17,17}, /* 0000 0000 0001 000x x */
		{1,1,16,17},{1,1,-16,17},{1,1,15,17},{1,1,-15,17}, /* 0000 0000 0001 001x x */
		{1,6,3,17},{1,6,-3,17},{1,16,2,17},{1,16,-2,17},   /* 0000 0000 0001 010x x */
		{1,15,2,17},{1,15,-2,17},{1,14,2,17},{1,14,-2,17}, /* 0000 0000 0001 011x x */
		{1,13,2,17},{1,13,-2,17},{1,12,2,17},{1,12,-2,17}, /* 0000 0000 0001 100x x */
		{1,11,2,17},{1,11,-2,17},{1,31,1,17},{1,31,-1,17}, /* 0000 0000 0001 101x x */
		{1,30,1,17},{1,30,-1,17},{1,29,1,17},{1,29,-1,17}, /* 0000 0000 0001 110x x */
		{1,28,1,17},{1,28,-1,17},{1,27,1,17},{1,27,-1,17}, /* 0000 0000 0001 111x x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_b[] = {
		{1,0,40,16},{1,0,-40,16},{1,0,39,16},{1,0,-39,16}, /* 0000 0000 0010 00xx x */
		{1,0,38,16},{1,0,-38,16},{1,0,37,16},{1,0,-37,16}, /* 0000 0000 0010 01xx x */
		{1,0,36,16},{1,0,-36,16},{1,0,35,16},{1,0,-35,16}, /* 0000 0000 0010 10xx x */
		{1,0,34,16},{1,0,-34,16},{1,0,33,16},{1,0,-33,16}, /* 0000 0000 0010 11xx x */
		{1,0,32,16},{1,0,-32,16},{1,1,14,16},{1,1,-14,16}, /* 0000 0000 0011 00xx x */
		{1,1,13,16},{1,1,-13,16},{1,1,12,16},{1,1,-12,16}, /* 0000 0000 0011 01xx x */
		{1,1,11,16},{1,1,-11,16},{1,1,10,16},{1,1,-10,16}, /* 0000 0000 0011 10xx x */
		{1,1,9,16},{1,1,-9,16},{1,1,8,16},{1,1,-8,16},     /* 0000 0000 0011 11xx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_c[] = {
		{1,0,31,15},{1,0,-31,15},{1,0,30,15},{1,0,-30,15}, /* 0000 0000 0100 0xxx x */
		{1,0,29,15},{1,0,-29,15},{1,0,28,15},{1,0,-28,15}, /* 0000 0000 0100 1xxx x */
		{1,0,27,15},{1,0,-27,15},{1,0,26,15},{1,0,-26,15}, /* 0000 0000 0101 0xxx x */
		{1,0,25,15},{1,0,-25,15},{1,0,24,15},{1,0,-24,15}, /* 0000 0000 0101 0xxx x */
		{1,0,23,15},{1,0,-23,15},{1,0,22,15},{1,0,-22,15}, /* 0000 0000 0110 0xxx x */
		{1,0,21,15},{1,0,-21,15},{1,0,20,15},{1,0,-20,15}, /* 0000 0000 0110 1xxx x */
		{1,0,19,15},{1,0,-19,15},{1,0,18,15},{1,0,-18,15}, /* 0000 0000 0111 0xxx x */
		{1,0,17,15},{1,0,-17,15},{1,0,16,15},{1,0,-16,15}, /* 0000 0000 0111 0xxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_d[] = {
		{1,10,2,14},{1,10,-2,14},{1,9,2,14},{1,9,-2,14},   /* 0000 0000 1000 xxxx x */
		{1,5,3,14},{1,5,-3,14},{1,3,4,14},{1,3,-4,14},     /* 0000 0000 1001 xxxx x */
		{1,2,5,14},{1,2,-5,14},{1,1,7,14},{1,1,-7,14},     /* 0000 0000 1010 xxxx x */
		{1,1,6,14},{1,1,-6,14},{1,0,15,14},{1,0,-15,14},   /* 0000 0000 1011 xxxx x */
		{1,0,14,14},{1,0,-14,14},{1,0,13,14},{1,0,-13,14}, /* 0000 0000 1100 xxxx x */
		{1,0,12,14},{1,0,-12,14},{1,26,1,14},{1,26,-1,14}, /* 0000 0000 1101 xxxx x */
		{1,25,1,14},{1,25,-1,14},{1,24,1,14},{1,24,-1,14}, /* 0000 0000 1110 xxxx x */
		{1,23,1,14},{1,23,-1,14},{1,22,1,14},{1,22,-1,14}, /* 0000 0000 1110 xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_e[] = {
		{1,0,11,13},{1,0,-11,13},{1,8,2,13},{1,8,-2,13},   /* 0000 0001 000x xxxx x */
		{1,4,3,13},{1,4,-3,13},{1,0,10,13},{1,0,-10,13},   /* 0000 0001 001x xxxx x */
		{1,2,4,13},{1,2,-4,13},{1,7,2,13},{1,7,-2,13},     /* 0000 0001 010x xxxx x */
		{1,21,1,13},{1,21,-1,13},{1,20,1,13},{1,20,-1,13}, /* 0000 0001 011x xxxx x */
		{1,0,9,13},{1,0,-9,13},{1,19,1,13},{1,19,-1,13},   /* 0000 0001 100x xxxx x */
		{1,18,1,13},{1,18,-1,13},{1,1,5,13},{1,1,-5,13},   /* 0000 0001 101x xxxx x */
		{1,3,3,13},{1,3,-3,13},{1,0,8,13},{1,0,-8,13},     /* 0000 0001 110x xxxx x */
		{1,6,2,13},{1,6,-2,13},{1,17,1,13},{1,17,-1,13},   /* 0000 0001 111x xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_f[] = {
		{1,16,1,11},{1,16,-1,11},{1,5,2,11},{1,5,-2,11},   /* 0000 0010 0xxx xxxx x */
		{1,0,7,11},{1,0,-7,11},{1,2,3,11},{1,2,-3,11},     /* 0000 0010 1xxx xxxx x */
		{1,1,4,11},{1,1,-4,11},{1,15,1,11},{1,15,-1,11},   /* 0000 0011 0xxx xxxx x */
		{1,14,1,11},{1,14,-1,11},{1,4,2,11},{1,4,-2,11},   /* 0000 0011 1xxx xxxx x */
	};

	/* escape */                                               /* 0000 01xx xxxx xxxx x */

	static const DCT_COEFFICIENT_VLC_ELEMENT table_g[] = {
		{1,2,2,8},{1,2,-2,8},{1,9,1,8},{1,9,-1,8},         /* 0000 10xx xxxx xxxx x */
		{1,0,4,8},{1,0,-4,8},{1,8,1,8},{1,8,-1,8},         /* 0000 11xx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_h[] = {
		{1,7,1,7},{1,7,-1,7},{1,6,1,7},{1,6,-1,7},         /* 0001 0xxx xxxx xxxx x */
		{1,1,2,7},{1,1,-2,7},{1,5,1,7},{1,5,-1,7},         /* 0001 1xxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_i[] = {
		{1,13,1,9},{1,13,-1,9},{1,0,6,9},{1,0,-6,9},       /* 0010 000x xxxx xxxx x */
		{1,12,1,9},{1,12,-1,9},{1,11,1,9},{1,11,-1,9},     /* 0010 001x xxxx xxxx x */
		{1,3,2,9},{1,3,-2,9},{1,1,3,9},{1,1,-3,9},         /* 0010 010x xxxx xxxx x */
		{1,0,5,9},{1,0,-5,9},{1,10,1,9},{1,10,-1,9},       /* 0010 011x xxxx xxxx x */
		{1,0,3,6},{1,0,3,6},{1,0,3,6},{1,0,3,6},           /* 0010 100x xxxx xxxx x */
		{1,0,3,6},{1,0,3,6},{1,0,3,6},{1,0,3,6},           /* 0010 101x xxxx xxxx x */
		{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},       /* 0010 110x xxxx xxxx x */
		{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},       /* 0010 111x xxxx xxxx x */
		{1,4,1,6},{1,4,1,6},{1,4,1,6},{1,4,1,6},           /* 0011 000x xxxx xxxx x */
		{1,4,1,6},{1,4,1,6},{1,4,1,6},{1,4,1,6},           /* 0011 001x xxxx xxxx x */
		{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},       /* 0011 010x xxxx xxxx x */
		{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},       /* 0011 011x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 100x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 101x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 110x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 111x xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_j[] = {
		{1,0,2,5},{1,0,-2,5},{1,2,1,5},{1,2,-1,5},         /* 010x xxxx xxxx xxxx x */
		{1,1,1,4},{1,1,1,4},{1,1,-1,4},{1,1,-1,4},         /* 011x xxxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_k[] = {
		{1,0,1,2},{1,0,-1,2},                              /* 1xxx xxxx xxxx xxxx x */
	};

	code = vs_read_bits(in, 17);
	n = log2(code);
	
	switch(n){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		*run = 0;
		*level = 0;
		return -1;
	case 5:
		code -= 0x20;
		vs_erase_bits(in, table_a[code].size);
		*run = table_a[code].run;
		*level = table_a[code].level;
		return table_a[code].flag;
	case 6:
		code >>= 1;
		code -= 0x20;
		vs_erase_bits(in, table_b[code].size);
		*run = table_b[code].run;
		*level = table_b[code].level;
		return table_b[code].flag;
	case 7:
		code >>= 2;
		code -= 0x20;
		vs_erase_bits(in, table_c[code].size);
		*run = table_c[code].run;
		*level = table_c[code].level;
		return table_c[code].flag;
	case 8:
		code >>= 3;
		code -= 0x20;
		vs_erase_bits(in, table_d[code].size);
		*run = table_d[code].run;
		*level = table_d[code].level;
		return table_d[code].flag;
	case 9:
		code >>= 4;
		code -= 0x20;
		vs_erase_bits(in, table_e[code].size);
		*run = table_e[code].run;
		*level = table_e[code].level;
		return table_e[code].flag;
	case 10:
		code >>= 6;
		code -= 0x10;
		vs_erase_bits(in, table_f[code].size);
		*run = table_f[code].run;
		*level = table_f[code].level;
		return table_e[code].flag;
	case 11:
		/* escape */
		code >>= 5;
		code &= 0x3f;
		vs_erase_bits(in, 12);
		*run = code;
		*level = get_dct_coefficient_escape_level_mpeg1(in);
		return 1;
	case 12:
		code >>= 9;
		code -= 0x08;
		vs_erase_bits(in, table_g[code].size);
		*run = table_g[code].run;
		*level = table_g[code].level;
		return table_g[code].flag;
	case 13:
		code >>= 10;
		code -= 0x08;
		vs_erase_bits(in, table_h[code].size);
		*run = table_h[code].run;
		*level = table_h[code].level;
		return table_h[code].flag;
	case 14:
		code >>= 8;
		code -= 0x40;
		vs_erase_bits(in, table_i[code].size);
		*run = table_i[code].run;
		*level = table_i[code].level;
		return table_i[code].flag;
	case 15:
		code >>= 12;
		code -= 0x08;
		vs_erase_bits(in, table_j[code].size);
		*run = table_j[code].run;
		*level = table_j[code].level;
		return table_j[code].flag;
	case 16:
		code >>= 15;
		code -= 0x2;
		vs_erase_bits(in, table_k[code].size);
		*run = table_k[code].run;
		*level = table_k[code].level;
		return table_k[code].flag;
	}

	*run = 0;
	*level = 0;
	return -1;
}

int read_dct_ac_coefficient_b14(VIDEO_STREAM *in, int *run, int *level)
{
	int n;
	int code;

	static const DCT_COEFFICIENT_VLC_ELEMENT table_a[] = {
		{1,1,18,17},{1,1,-18,17},{1,1,17,17},{1,1,-17,17}, /* 0000 0000 0001 000x x */
		{1,1,16,17},{1,1,-16,17},{1,1,15,17},{1,1,-15,17}, /* 0000 0000 0001 001x x */
		{1,6,3,17},{1,6,-3,17},{1,16,2,17},{1,16,-2,17},   /* 0000 0000 0001 010x x */
		{1,15,2,17},{1,15,-2,17},{1,14,2,17},{1,14,-2,17}, /* 0000 0000 0001 011x x */
		{1,13,2,17},{1,13,-2,17},{1,12,2,17},{1,12,-2,17}, /* 0000 0000 0001 100x x */
		{1,11,2,17},{1,11,-2,17},{1,31,1,17},{1,31,-1,17}, /* 0000 0000 0001 101x x */
		{1,30,1,17},{1,30,-1,17},{1,29,1,17},{1,29,-1,17}, /* 0000 0000 0001 110x x */
		{1,28,1,17},{1,28,-1,17},{1,27,1,17},{1,27,-1,17}, /* 0000 0000 0001 111x x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_b[] = {
		{1,0,40,16},{1,0,-40,16},{1,0,39,16},{1,0,-39,16}, /* 0000 0000 0010 00xx x */
		{1,0,38,16},{1,0,-38,16},{1,0,37,16},{1,0,-37,16}, /* 0000 0000 0010 01xx x */
		{1,0,36,16},{1,0,-36,16},{1,0,35,16},{1,0,-35,16}, /* 0000 0000 0010 10xx x */
		{1,0,34,16},{1,0,-34,16},{1,0,33,16},{1,0,-33,16}, /* 0000 0000 0010 11xx x */
		{1,0,32,16},{1,0,-32,16},{1,1,14,16},{1,1,-14,16}, /* 0000 0000 0011 00xx x */
		{1,1,13,16},{1,1,-13,16},{1,1,12,16},{1,1,-12,16}, /* 0000 0000 0011 01xx x */
		{1,1,11,16},{1,1,-11,16},{1,1,10,16},{1,1,-10,16}, /* 0000 0000 0011 10xx x */
		{1,1,9,16},{1,1,-9,16},{1,1,8,16},{1,1,-8,16},     /* 0000 0000 0011 11xx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_c[] = {
		{1,0,31,15},{1,0,-31,15},{1,0,30,15},{1,0,-30,15}, /* 0000 0000 0100 0xxx x */
		{1,0,29,15},{1,0,-29,15},{1,0,28,15},{1,0,-28,15}, /* 0000 0000 0100 1xxx x */
		{1,0,27,15},{1,0,-27,15},{1,0,26,15},{1,0,-26,15}, /* 0000 0000 0101 0xxx x */
		{1,0,25,15},{1,0,-25,15},{1,0,24,15},{1,0,-24,15}, /* 0000 0000 0101 0xxx x */
		{1,0,23,15},{1,0,-23,15},{1,0,22,15},{1,0,-22,15}, /* 0000 0000 0110 0xxx x */
		{1,0,21,15},{1,0,-21,15},{1,0,20,15},{1,0,-20,15}, /* 0000 0000 0110 1xxx x */
		{1,0,19,15},{1,0,-19,15},{1,0,18,15},{1,0,-18,15}, /* 0000 0000 0111 0xxx x */
		{1,0,17,15},{1,0,-17,15},{1,0,16,15},{1,0,-16,15}, /* 0000 0000 0111 0xxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_d[] = {
		{1,10,2,14},{1,10,-2,14},{1,9,2,14},{1,9,-2,14},   /* 0000 0000 1000 xxxx x */
		{1,5,3,14},{1,5,-3,14},{1,3,4,14},{1,3,-4,14},     /* 0000 0000 1001 xxxx x */
		{1,2,5,14},{1,2,-5,14},{1,1,7,14},{1,1,-7,14},     /* 0000 0000 1010 xxxx x */
		{1,1,6,14},{1,1,-6,14},{1,0,15,14},{1,0,-15,14},   /* 0000 0000 1011 xxxx x */
		{1,0,14,14},{1,0,-14,14},{1,0,13,14},{1,0,-13,14}, /* 0000 0000 1100 xxxx x */
		{1,0,12,14},{1,0,-12,14},{1,26,1,14},{1,26,-1,14}, /* 0000 0000 1101 xxxx x */
		{1,25,1,14},{1,25,-1,14},{1,24,1,14},{1,24,-1,14}, /* 0000 0000 1110 xxxx x */
		{1,23,1,14},{1,23,-1,14},{1,22,1,14},{1,22,-1,14}, /* 0000 0000 1110 xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_e[] = {
		{1,0,11,13},{1,0,-11,13},{1,8,2,13},{1,8,-2,13},   /* 0000 0001 000x xxxx x */
		{1,4,3,13},{1,4,-3,13},{1,0,10,13},{1,0,-10,13},   /* 0000 0001 001x xxxx x */
		{1,2,4,13},{1,2,-4,13},{1,7,2,13},{1,7,-2,13},     /* 0000 0001 010x xxxx x */
		{1,21,1,13},{1,21,-1,13},{1,20,1,13},{1,20,-1,13}, /* 0000 0001 011x xxxx x */
		{1,0,9,13},{1,0,-9,13},{1,19,1,13},{1,19,-1,13},   /* 0000 0001 100x xxxx x */
		{1,18,1,13},{1,18,-1,13},{1,1,5,13},{1,1,-5,13},   /* 0000 0001 101x xxxx x */
		{1,3,3,13},{1,3,-3,13},{1,0,8,13},{1,0,-8,13},     /* 0000 0001 110x xxxx x */
		{1,6,2,13},{1,6,-2,13},{1,17,1,13},{1,17,-1,13},   /* 0000 0001 111x xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_f[] = {
		{1,16,1,11},{1,16,-1,11},{1,5,2,11},{1,5,-2,11},   /* 0000 0010 0xxx xxxx x */
		{1,0,7,11},{1,0,-7,11},{1,2,3,11},{1,2,-3,11},     /* 0000 0010 1xxx xxxx x */
		{1,1,4,11},{1,1,-4,11},{1,15,1,11},{1,15,-1,11},   /* 0000 0011 0xxx xxxx x */
		{1,14,1,11},{1,14,-1,11},{1,4,2,11},{1,4,-2,11},   /* 0000 0011 1xxx xxxx x */
	};

	/* escape */                                               /* 0000 01xx xxxx xxxx x */

	static const DCT_COEFFICIENT_VLC_ELEMENT table_g[] = {
		{1,2,2,8},{1,2,-2,8},{1,9,1,8},{1,9,-1,8},         /* 0000 10xx xxxx xxxx x */
		{1,0,4,8},{1,0,-4,8},{1,8,1,8},{1,8,-1,8},         /* 0000 11xx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_h[] = {
		{1,7,1,7},{1,7,-1,7},{1,6,1,7},{1,6,-1,7},         /* 0001 0xxx xxxx xxxx x */
		{1,1,2,7},{1,1,-2,7},{1,5,1,7},{1,5,-1,7},         /* 0001 1xxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_i[] = {
		{1,13,1,9},{1,13,-1,9},{1,0,6,9},{1,0,-6,9},       /* 0010 000x xxxx xxxx x */
		{1,12,1,9},{1,12,-1,9},{1,11,1,9},{1,11,-1,9},     /* 0010 001x xxxx xxxx x */
		{1,3,2,9},{1,3,-2,9},{1,1,3,9},{1,1,-3,9},         /* 0010 010x xxxx xxxx x */
		{1,0,5,9},{1,0,-5,9},{1,10,1,9},{1,10,-1,9},       /* 0010 011x xxxx xxxx x */
		{1,0,3,6},{1,0,3,6},{1,0,3,6},{1,0,3,6},           /* 0010 100x xxxx xxxx x */
		{1,0,3,6},{1,0,3,6},{1,0,3,6},{1,0,3,6},           /* 0010 101x xxxx xxxx x */
		{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},       /* 0010 110x xxxx xxxx x */
		{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},       /* 0010 111x xxxx xxxx x */
		{1,4,1,6},{1,4,1,6},{1,4,1,6},{1,4,1,6},           /* 0011 000x xxxx xxxx x */
		{1,4,1,6},{1,4,1,6},{1,4,1,6},{1,4,1,6},           /* 0011 001x xxxx xxxx x */
		{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},       /* 0011 010x xxxx xxxx x */
		{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},       /* 0011 011x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 100x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 101x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 110x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 111x xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_j[] = {
		{1,0,2,5},{1,0,-2,5},{1,2,1,5},{1,2,-1,5},         /* 010x xxxx xxxx xxxx x */
		{1,1,1,4},{1,1,1,4},{1,1,-1,4},{1,1,-1,4},         /* 011x xxxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_k[] = {
		{0,0,0,2},{0,0,0,2},{1,0,1,3},{1,0,-1,3},          /* 1xxx xxxx xxxx xxxx x */
	};

	code = vs_read_bits(in, 17);
	n = log2(code);

	switch(n){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		*run = 0;
		*level = 0;
		return -1;
	case 5:
		code -= 0x20;
		vs_erase_bits(in, table_a[code].size);
		*run = table_a[code].run;
		*level = table_a[code].level;
		return table_a[code].flag;
	case 6:
		code >>= 1;
		code -= 0x20;
		vs_erase_bits(in, table_b[code].size);
		*run = table_b[code].run;
		*level = table_b[code].level;
		return table_b[code].flag;
	case 7:
		code >>= 2;
		code -= 0x20;
		vs_erase_bits(in, table_c[code].size);
		*run = table_c[code].run;
		*level = table_c[code].level;
		return table_c[code].flag;
	case 8:
		code >>= 3;
		code -= 0x20;
		vs_erase_bits(in, table_d[code].size);
		*run = table_d[code].run;
		*level = table_d[code].level;
		return table_d[code].flag;
	case 9:
		code >>= 4;
		code -= 0x20;
		vs_erase_bits(in, table_e[code].size);
		*run = table_e[code].run;
		*level = table_e[code].level;
		return table_e[code].flag;
	case 10:
		code >>= 6;
		code -= 0x10;
		vs_erase_bits(in, table_f[code].size);
		*run = table_f[code].run;
		*level = table_f[code].level;
		return table_e[code].flag;
	case 11:
		/* escape */
		code >>= 5;
		code &= 0x3f;
		vs_erase_bits(in, 12);
		*run = code;
		*level = get_dct_coefficient_escape_level_mpeg2(in);
		return 1;
	case 12:
		code >>= 9;
		code -= 0x08;
		vs_erase_bits(in, table_g[code].size);
		*run = table_g[code].run;
		*level = table_g[code].level;
		return table_g[code].flag;
	case 13:
		code >>= 10;
		code -= 0x08;
		vs_erase_bits(in, table_h[code].size);
		*run = table_h[code].run;
		*level = table_h[code].level;
		return table_h[code].flag;
	case 14:
		code >>= 8;
		code -= 0x40;
		vs_erase_bits(in, table_i[code].size);
		*run = table_i[code].run;
		*level = table_i[code].level;
		return table_i[code].flag;
	case 15:
		code >>= 12;
		code -= 0x08;
		vs_erase_bits(in, table_j[code].size);
		*run = table_j[code].run;
		*level = table_j[code].level;
		return table_j[code].flag;
	case 16:
		code >>= 14;
		code -= 0x4;
		vs_erase_bits(in, table_k[code].size);
		*run = table_k[code].run;
		*level = table_k[code].level;
		return table_k[code].flag;
	}

	*run = 0;
	*level = 0;
	return -1;
}

int read_dct_ac_coefficient_b15(VIDEO_STREAM *in, int *run, int *level)
{
	int n;
	int code;
	
	static const DCT_COEFFICIENT_VLC_ELEMENT table_a[] = {
		{1,1,18,17},{1,1,-18,17},{1,1,17,17},{1,1,-17,17}, /* 0000 0000 0001 000x x */
		{1,1,16,17},{1,1,-16,17},{1,1,15,17},{1,1,-15,17}, /* 0000 0000 0001 001x x */
		{1,6,3,17},{1,6,-3,17},{1,16,2,17},{1,16,-2,17},   /* 0000 0000 0001 010x x */
		{1,15,2,17},{1,15,-2,17},{1,14,2,17},{1,14,-2,17}, /* 0000 0000 0001 011x x */
		{1,13,2,17},{1,13,-2,17},{1,12,2,17},{1,12,-2,17}, /* 0000 0000 0001 100x x */
		{1,11,2,17},{1,11,-2,17},{1,31,1,17},{1,31,-1,17}, /* 0000 0000 0001 101x x */
		{1,30,1,17},{1,30,-1,17},{1,29,1,17},{1,29,-1,17}, /* 0000 0000 0001 110x x */
		{1,28,1,17},{1,28,-1,17},{1,27,1,17},{1,27,-1,17}, /* 0000 0000 0001 111x x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_b[] = {
		{1,0,40,16},{1,0,-40,16},{1,0,39,16},{1,0,-39,16}, /* 0000 0000 0010 00xx x */
		{1,0,38,16},{1,0,-38,16},{1,0,37,16},{1,0,-37,16}, /* 0000 0000 0010 01xx x */
		{1,0,36,16},{1,0,-36,16},{1,0,35,16},{1,0,-35,16}, /* 0000 0000 0010 10xx x */
		{1,0,34,16},{1,0,-34,16},{1,0,33,16},{1,0,-33,16}, /* 0000 0000 0010 11xx x */
		{1,0,32,16},{1,0,-32,16},{1,1,14,16},{1,1,-14,16}, /* 0000 0000 0011 00xx x */
		{1,1,13,16},{1,1,-13,16},{1,1,12,16},{1,1,-12,16}, /* 0000 0000 0011 01xx x */
		{1,1,11,16},{1,1,-11,16},{1,1,10,16},{1,1,-10,16}, /* 0000 0000 0011 10xx x */
		{1,1,9,16},{1,1,-9,16},{1,1,8,16},{1,1,-8,16},     /* 0000 0000 0011 11xx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_c[] = {
		{1,0,31,15},{1,0,-31,15},{1,0,30,15},{1,0,-30,15}, /* 0000 0000 0100 0xxx x */
		{1,0,29,15},{1,0,-29,15},{1,0,28,15},{1,0,-28,15}, /* 0000 0000 0100 1xxx x */
		{1,0,27,15},{1,0,-27,15},{1,0,26,15},{1,0,-26,15}, /* 0000 0000 0101 0xxx x */
		{1,0,25,15},{1,0,-25,15},{1,0,24,15},{1,0,-24,15}, /* 0000 0000 0101 0xxx x */
		{1,0,23,15},{1,0,-23,15},{1,0,22,15},{1,0,-22,15}, /* 0000 0000 0110 0xxx x */
		{1,0,21,15},{1,0,-21,15},{1,0,20,15},{1,0,-20,15}, /* 0000 0000 0110 1xxx x */
		{1,0,19,15},{1,0,-19,15},{1,0,18,15},{1,0,-18,15}, /* 0000 0000 0111 0xxx x */
		{1,0,17,15},{1,0,-17,15},{1,0,16,15},{1,0,-16,15}, /* 0000 0000 0111 0xxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_d[] = {
		{1,10,2,14},{1,10,-2,14},{1,9,2,14},{1,9,-2,14},   /* 0000 0000 1000 xxxx x */
		{1,5,3,14},{1,5,-3,14},{1,3,4,14},{1,3,-4,14},     /* 0000 0000 1001 xxxx x */
		{1,2,5,14},{1,2,-5,14},{1,1,7,14},{1,1,-7,14},     /* 0000 0000 1010 xxxx x */
		{1,1,6,14},{1,1,-6,14},{-1,0,0,0},{-1,0,0,0},      /* 0000 0000 1011 xxxx x */
		{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},       /* 0000 0000 1100 xxxx x */
		{-1,0,0,0},{-1,0,0,0},{1,26,1,14},{1,26,-1,14},    /* 0000 0000 1101 xxxx x */
		{1,25,1,14},{1,25,-1,14},{1,24,1,14},{1,24,-1,14}, /* 0000 0000 1110 xxxx x */
		{1,23,1,14},{1,23,-1,14},{1,22,1,14},{1,22,-1,14}, /* 0000 0000 1110 xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_e[] = {
		{-1,0,0,0},{-1,0,0,0},{1,8,2,13},{1,8,-2,13},      /* 0000 0001 000x xxxx x */
		{1,4,3,13},{1,4,-3,13},{-1,0,0,0},{-1,0,0,0},      /* 0000 0001 001x xxxx x */
		{-1,0,0,0},{-1,0,0,0},{1,7,2,13},{1,7,-2,13},      /* 0000 0001 010x xxxx x */
		{1,21,1,13},{1,21,-1,13},{1,20,1,13},{1,20,-1,13}, /* 0000 0001 011x xxxx x */
		{-1,0,0,0},{-1,0,0,0},{1,19,1,13},{1,19,-1,13},    /* 0000 0001 100x xxxx x */
		{1,18,1,13},{1,18,-1,13},{-1,0,0,0},{-1,0,0,0},    /* 0000 0001 101x xxxx x */
		{1,3,3,13},{1,3,-3,13},{-1,0,0,0},{-1,0,0,0},      /* 0000 0001 110x xxxx x */
		{1,6,2,13},{1,6,-2,13},{1,17,1,13},{1,17,-1,13},   /* 0000 0001 111x xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_f[] = {
		{1,5,2,10},{1,5,2,10},{1,5,-2,10},{1,5,-2,10},     /* 0000 0010 0xxx xxxx x */
		{1,14,1,10},{1,14,1,10},{1,14,-1,10},{1,14,-1,10}, /* 0000 0010 1xxx xxxx x */
		{1,2,4,11},{1,2,-4,11},{1,16,1,11},{1,16,-1,11},   /* 0000 0011 0xxx xxxx x */
		{1,15,1,10},{1,15,1,10},{1,15,-1,10},{1,15,-1,10}, /* 0000 0011 1xxx xxxx x */
	};

	/* escape */                                               /* 0000 01xx xxxx xxxx x */

	static const DCT_COEFFICIENT_VLC_ELEMENT table_g[] = {
		{1,7,1,8},{1,7,-1,8},{1,8,1,8},{1,8,-1,8},         /* 0000 10xx xxxx xxxx x */
		{1,6,1,8},{1,6,-1,8},{1,2,2,8},{1,2,-2,8},         /* 0000 11xx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_h[] = {
		{1,0,7,7},{1,0,-7,7},{1,0,6,7},{1,0,-6,7},         /* 0001 0xxx xxxx xxxx x */
		{1,4,1,7},{1,4,-1,7},{1,5,1,7},{1,5,-1,7},         /* 0001 1xxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_i[] = {
		{1,1,5,9},{1,1,-5,9},{1,11,1,9},{1,11,-1,9},       /* 0010 000x xxxx xxxx x */
		{1,0,11,9},{1,0,-11,9},{1,0,10,9},{1,0,-10,9},     /* 0010 001x xxxx xxxx x */
		{1,13,1,9},{1,13,-1,9},{1,12,1,9},{1,12,-1,9},     /* 0010 010x xxxx xxxx x */
		{1,3,2,9},{1,3,-2,9},{1,1,4,9},{1,1,-4,9},         /* 0010 011x xxxx xxxx x */
		{1,2,1,6},{1,2,1,6},{1,2,1,6},{1,2,1,6},           /* 0010 100x xxxx xxxx x */
		{1,2,1,6},{1,2,1,6},{1,2,1,6},{1,2,1,6},           /* 0010 101x xxxx xxxx x */
		{1,2,-1,6},{1,2,-1,6},{1,2,-1,6},{1,2,-1,6},       /* 0010 110x xxxx xxxx x */
		{1,2,-1,6},{1,2,-1,6},{1,2,-1,6},{1,2,-1,6},       /* 0010 110x xxxx xxxx x */
		{1,1,2,6},{1,1,2,6},{1,1,2,6},{1,1,2,6},           /* 0011 000x xxxx xxxx x */
		{1,1,2,6},{1,1,2,6},{1,1,2,6},{1,1,2,6},           /* 0011 001x xxxx xxxx x */
		{1,1,-2,6},{1,1,-2,6},{1,1,-2,6},{1,1,-2,6},       /* 0011 010x xxxx xxxx x */
		{1,1,-2,6},{1,1,-2,6},{1,1,-2,6},{1,1,-2,6},       /* 0011 011x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 100x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 101x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 110x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 111x xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_j[] = {
		{1,1,1,4},{1,1,1,4},{1,1,-1,4},{1,1,-1,4},         /* 010x xxxx xxxx xxxx x */
		{0,0,0,4},{0,0,0,4},{1,0,3,5},{1,0,-3,5},          /* 011x xxxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_k[] = {
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1000 000x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1000 001x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1000 010x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1000 011x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1000 100x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1000 101x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1000 110x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1000 111x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1001 000x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1001 001x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1001 010x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1001 011x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1001 100x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1001 101x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1001 110x xxxx xxxx x */
		{1,0,1,3},{1,0,1,3},{1,0,1,3},{1,0,1,3},           /* 1001 111x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1010 000x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1010 001x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1010 010x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1010 011x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1010 100x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1010 101x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1010 110x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1010 111x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1011 000x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1011 001x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1011 010x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1011 011x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1011 100x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1011 101x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1011 110x xxxx xxxx x */
		{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},{1,0,-1,3},       /* 1011 111x xxxx xxxx x */
		{1,0,2,4},{1,0,2,4},{1,0,2,4},{1,0,2,4},           /* 1100 000x xxxx xxxx x */
		{1,0,2,4},{1,0,2,4},{1,0,2,4},{1,0,2,4},           /* 1100 001x xxxx xxxx x */
		{1,0,2,4},{1,0,2,4},{1,0,2,4},{1,0,2,4},           /* 1100 010x xxxx xxxx x */
		{1,0,2,4},{1,0,2,4},{1,0,2,4},{1,0,2,4},           /* 1100 011x xxxx xxxx x */
		{1,0,2,4},{1,0,2,4},{1,0,2,4},{1,0,2,4},           /* 1100 100x xxxx xxxx x */
		{1,0,2,4},{1,0,2,4},{1,0,2,4},{1,0,2,4},           /* 1100 101x xxxx xxxx x */
		{1,0,2,4},{1,0,2,4},{1,0,2,4},{1,0,2,4},           /* 1100 110x xxxx xxxx x */
		{1,0,2,4},{1,0,2,4},{1,0,2,4},{1,0,2,4},           /* 1100 111x xxxx xxxx x */
		{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},       /* 1101 000x xxxx xxxx x */
		{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},       /* 1101 001x xxxx xxxx x */
		{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},       /* 1101 010x xxxx xxxx x */
		{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},       /* 1101 011x xxxx xxxx x */
		{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},       /* 1101 100x xxxx xxxx x */
		{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},       /* 1101 101x xxxx xxxx x */
		{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},       /* 1101 110x xxxx xxxx x */
		{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},{1,0,-2,4},       /* 1101 111x xxxx xxxx x */
		{1,0,4,6},{1,0,4,6},{1,0,4,6},{1,0,4,6},           /* 1110 000x xxxx xxxx x */
		{1,0,4,6},{1,0,4,6},{1,0,4,6},{1,0,4,6},           /* 1110 001x xxxx xxxx x */
		{1,0,-4,6},{1,0,-4,6},{1,0,-4,6},{1,0,-4,6},       /* 1110 010x xxxx xxxx x */
		{1,0,-4,6},{1,0,-4,6},{1,0,-4,6},{1,0,-4,6},       /* 1110 011x xxxx xxxx x */
		{1,0,5,6},{1,0,5,6},{1,0,5,6},{1,0,5,6},           /* 1110 100x xxxx xxxx x */
		{1,0,5,6},{1,0,5,6},{1,0,5,6},{1,0,5,6},           /* 1110 101x xxxx xxxx x */
		{1,0,-5,6},{1,0,-5,6},{1,0,-5,6},{1,0,-5,6},       /* 1110 110x xxxx xxxx x */
		{1,0,-5,6},{1,0,-5,6},{1,0,-5,6},{1,0,-5,6},       /* 1110 111x xxxx xxxx x */
		{1,9,1,8},{1,9,1,8},{1,9,-1,8},{1,9,-1,8},         /* 1111 000x xxxx xxxx x */
		{1,1,3,8},{1,1,3,8},{1,1,-3,8},{1,1,-3,8},         /* 1111 001x xxxx xxxx x */
		{1,10,1,8},{1,10,1,8},{1,10,-1,8},{1,10,-1,8},     /* 1111 010x xxxx xxxx x */
		{1,0,8,8},{1,0,8,8},{1,0,-8,8},{1,0,-8,8},         /* 1111 011x xxxx xxxx x */
		{1,0,9,8},{1,0,9,8},{1,0,-9,8},{1,0,-9,8},         /* 1111 100x xxxx xxxx x */
		{1,0,12,9},{1,0,-12,9},{1,0,13,9},{1,0,-13,9},     /* 1111 101x xxxx xxxx x */
		{1,2,3,9},{1,2,-3,9},{1,4,2,9},{1,4,-2,9},         /* 1111 110x xxxx xxxx x */
		{1,0,14,9},{1,0,-14,9},{1,0,15,9},{1,0,-15,9},     /* 1111 111x xxxx xxxx x */
	};

	code = vs_read_bits(in, 17);
	n = log2(code);

	switch(n){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		*run = 0;
		*level = 0;
		return -1;
	case 5:
		code -= 0x20;
		vs_erase_bits(in, table_a[code].size);
		*run = table_a[code].run;
		*level = table_a[code].level;
		return table_a[code].flag;
	case 6:
		code >>= 1;
		code -= 0x20;
		vs_erase_bits(in, table_b[code].size);
		*run = table_b[code].run;
		*level = table_b[code].level;
		return table_b[code].flag;
	case 7:
		code >>= 2;
		code -= 0x20;
		vs_erase_bits(in, table_c[code].size);
		*run = table_c[code].run;
		*level = table_c[code].level;
		return table_c[code].flag;
	case 8:
		code >>= 3;
		code -= 0x20;
		vs_erase_bits(in, table_d[code].size);
		*run = table_d[code].run;
		*level = table_d[code].level;
		return table_d[code].flag;
	case 9:
		code >>= 4;
		code -= 0x20;
		vs_erase_bits(in, table_e[code].size);
		*run = table_e[code].run;
		*level = table_e[code].level;
		return table_e[code].flag;
	case 10:
		code >>= 6;
		code -= 0x10;
		vs_erase_bits(in, table_f[code].size);
		*run = table_f[code].run;
		*level = table_f[code].level;
		return table_f[code].flag;
	case 11:
		/* escape */
		code >>= 5;
		code &= 0x3f;
		vs_erase_bits(in, 12);
		*run = code;
		*level = get_dct_coefficient_escape_level_mpeg2(in);
		return 1;
	case 12:
		code >>= 9;
		code -= 0x08;
		vs_erase_bits(in, table_g[code].size);
		*run = table_g[code].run;
		*level = table_g[code].level;
		return table_g[code].flag;
	case 13:
		code >>= 10;
		code -= 0x08;
		vs_erase_bits(in, table_h[code].size);
		*run = table_h[code].run;
		*level = table_h[code].level;
		return table_h[code].flag;
	case 14:
		code >>= 8;
		code -= 0x40;
		vs_erase_bits(in, table_i[code].size);
		*run = table_i[code].run;
		*level = table_i[code].level;
		return table_i[code].flag;
	case 15:
		code >>= 12;
		code -= 0x08;
		vs_erase_bits(in, table_j[code].size);
		*run = table_j[code].run;
		*level = table_j[code].level;
		return table_j[code].flag;
	case 16:
		code >>= 8;
		code -= 0x100;
		vs_erase_bits(in, table_k[code].size);
		*run = table_k[code].run;
		*level = table_k[code].level;
		return table_k[code].flag;
	}
	
	*run = 0;
	*level = 0;
	return -1;
}

int read_dct_ac_coefficient_mpeg1(VIDEO_STREAM *in, int *run, int *level)
{
	int n;
	int code;

	static const DCT_COEFFICIENT_VLC_ELEMENT table_a[] = {
		{1,1,18,17},{1,1,-18,17},{1,1,17,17},{1,1,-17,17}, /* 0000 0000 0001 000x x */
		{1,1,16,17},{1,1,-16,17},{1,1,15,17},{1,1,-15,17}, /* 0000 0000 0001 001x x */
		{1,6,3,17},{1,6,-3,17},{1,16,2,17},{1,16,-2,17},   /* 0000 0000 0001 010x x */
		{1,15,2,17},{1,15,-2,17},{1,14,2,17},{1,14,-2,17}, /* 0000 0000 0001 011x x */
		{1,13,2,17},{1,13,-2,17},{1,12,2,17},{1,12,-2,17}, /* 0000 0000 0001 100x x */
		{1,11,2,17},{1,11,-2,17},{1,31,1,17},{1,31,-1,17}, /* 0000 0000 0001 101x x */
		{1,30,1,17},{1,30,-1,17},{1,29,1,17},{1,29,-1,17}, /* 0000 0000 0001 110x x */
		{1,28,1,17},{1,28,-1,17},{1,27,1,17},{1,27,-1,17}, /* 0000 0000 0001 111x x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_b[] = {
		{1,0,40,16},{1,0,-40,16},{1,0,39,16},{1,0,-39,16}, /* 0000 0000 0010 00xx x */
		{1,0,38,16},{1,0,-38,16},{1,0,37,16},{1,0,-37,16}, /* 0000 0000 0010 01xx x */
		{1,0,36,16},{1,0,-36,16},{1,0,35,16},{1,0,-35,16}, /* 0000 0000 0010 10xx x */
		{1,0,34,16},{1,0,-34,16},{1,0,33,16},{1,0,-33,16}, /* 0000 0000 0010 11xx x */
		{1,0,32,16},{1,0,-32,16},{1,1,14,16},{1,1,-14,16}, /* 0000 0000 0011 00xx x */
		{1,1,13,16},{1,1,-13,16},{1,1,12,16},{1,1,-12,16}, /* 0000 0000 0011 01xx x */
		{1,1,11,16},{1,1,-11,16},{1,1,10,16},{1,1,-10,16}, /* 0000 0000 0011 10xx x */
		{1,1,9,16},{1,1,-9,16},{1,1,8,16},{1,1,-8,16},     /* 0000 0000 0011 11xx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_c[] = {
		{1,0,31,15},{1,0,-31,15},{1,0,30,15},{1,0,-30,15}, /* 0000 0000 0100 0xxx x */
		{1,0,29,15},{1,0,-29,15},{1,0,28,15},{1,0,-28,15}, /* 0000 0000 0100 1xxx x */
		{1,0,27,15},{1,0,-27,15},{1,0,26,15},{1,0,-26,15}, /* 0000 0000 0101 0xxx x */
		{1,0,25,15},{1,0,-25,15},{1,0,24,15},{1,0,-24,15}, /* 0000 0000 0101 0xxx x */
		{1,0,23,15},{1,0,-23,15},{1,0,22,15},{1,0,-22,15}, /* 0000 0000 0110 0xxx x */
		{1,0,21,15},{1,0,-21,15},{1,0,20,15},{1,0,-20,15}, /* 0000 0000 0110 1xxx x */
		{1,0,19,15},{1,0,-19,15},{1,0,18,15},{1,0,-18,15}, /* 0000 0000 0111 0xxx x */
		{1,0,17,15},{1,0,-17,15},{1,0,16,15},{1,0,-16,15}, /* 0000 0000 0111 0xxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_d[] = {
		{1,10,2,14},{1,10,-2,14},{1,9,2,14},{1,9,-2,14},   /* 0000 0000 1000 xxxx x */
		{1,5,3,14},{1,5,-3,14},{1,3,4,14},{1,3,-4,14},     /* 0000 0000 1001 xxxx x */
		{1,2,5,14},{1,2,-5,14},{1,1,7,14},{1,1,-7,14},     /* 0000 0000 1010 xxxx x */
		{1,1,6,14},{1,1,-6,14},{1,0,15,14},{1,0,-15,14},   /* 0000 0000 1011 xxxx x */
		{1,0,14,14},{1,0,-14,14},{1,0,13,14},{1,0,-13,14}, /* 0000 0000 1100 xxxx x */
		{1,0,12,14},{1,0,-12,14},{1,26,1,14},{1,26,-1,14}, /* 0000 0000 1101 xxxx x */
		{1,25,1,14},{1,25,-1,14},{1,24,1,14},{1,24,-1,14}, /* 0000 0000 1110 xxxx x */
		{1,23,1,14},{1,23,-1,14},{1,22,1,14},{1,22,-1,14}, /* 0000 0000 1110 xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_e[] = {
		{1,0,11,13},{1,0,-11,13},{1,8,2,13},{1,8,-2,13},   /* 0000 0001 000x xxxx x */
		{1,4,3,13},{1,4,-3,13},{1,0,10,13},{1,0,-10,13},   /* 0000 0001 001x xxxx x */
		{1,2,4,13},{1,2,-4,13},{1,7,2,13},{1,7,-2,13},     /* 0000 0001 010x xxxx x */
		{1,21,1,13},{1,21,-1,13},{1,20,1,13},{1,20,-1,13}, /* 0000 0001 011x xxxx x */
		{1,0,9,13},{1,0,-9,13},{1,19,1,13},{1,19,-1,13},   /* 0000 0001 100x xxxx x */
		{1,18,1,13},{1,18,-1,13},{1,1,5,13},{1,1,-5,13},   /* 0000 0001 101x xxxx x */
		{1,3,3,13},{1,3,-3,13},{1,0,8,13},{1,0,-8,13},     /* 0000 0001 110x xxxx x */
		{1,6,2,13},{1,6,-2,13},{1,17,1,13},{1,17,-1,13},   /* 0000 0001 111x xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_f[] = {
		{1,16,1,11},{1,16,-1,11},{1,5,2,11},{1,5,-2,11},   /* 0000 0010 0xxx xxxx x */
		{1,0,7,11},{1,0,-7,11},{1,2,3,11},{1,2,-3,11},     /* 0000 0010 1xxx xxxx x */
		{1,1,4,11},{1,1,-4,11},{1,15,1,11},{1,15,-1,11},   /* 0000 0011 0xxx xxxx x */
		{1,14,1,11},{1,14,-1,11},{1,4,2,11},{1,4,-2,11},   /* 0000 0011 1xxx xxxx x */
	};

	/* escape */                                               /* 0000 01xx xxxx xxxx x */

	static const DCT_COEFFICIENT_VLC_ELEMENT table_g[] = {
		{1,2,2,8},{1,2,-2,8},{1,9,1,8},{1,9,-1,8},         /* 0000 10xx xxxx xxxx x */
		{1,0,4,8},{1,0,-4,8},{1,8,1,8},{1,8,-1,8},         /* 0000 11xx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_h[] = {
		{1,7,1,7},{1,7,-1,7},{1,6,1,7},{1,6,-1,7},         /* 0001 0xxx xxxx xxxx x */
		{1,1,2,7},{1,1,-2,7},{1,5,1,7},{1,5,-1,7},         /* 0001 1xxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_i[] = {
		{1,13,1,9},{1,13,-1,9},{1,0,6,9},{1,0,-6,9},       /* 0010 000x xxxx xxxx x */
		{1,12,1,9},{1,12,-1,9},{1,11,1,9},{1,11,-1,9},     /* 0010 001x xxxx xxxx x */
		{1,3,2,9},{1,3,-2,9},{1,1,3,9},{1,1,-3,9},         /* 0010 010x xxxx xxxx x */
		{1,0,5,9},{1,0,-5,9},{1,10,1,9},{1,10,-1,9},       /* 0010 011x xxxx xxxx x */
		{1,0,3,6},{1,0,3,6},{1,0,3,6},{1,0,3,6},           /* 0010 100x xxxx xxxx x */
		{1,0,3,6},{1,0,3,6},{1,0,3,6},{1,0,3,6},           /* 0010 101x xxxx xxxx x */
		{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},       /* 0010 110x xxxx xxxx x */
		{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},{1,0,-3,6},       /* 0010 111x xxxx xxxx x */
		{1,4,1,6},{1,4,1,6},{1,4,1,6},{1,4,1,6},           /* 0011 000x xxxx xxxx x */
		{1,4,1,6},{1,4,1,6},{1,4,1,6},{1,4,1,6},           /* 0011 001x xxxx xxxx x */
		{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},       /* 0011 010x xxxx xxxx x */
		{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},{1,4,-1,6},       /* 0011 011x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 100x xxxx xxxx x */
		{1,3,1,6},{1,3,1,6},{1,3,1,6},{1,3,1,6},           /* 0011 101x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 110x xxxx xxxx x */
		{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},{1,3,-1,6},       /* 0011 111x xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_j[] = {
		{1,0,2,5},{1,0,-2,5},{1,2,1,5},{1,2,-1,5},         /* 010x xxxx xxxx xxxx x */
		{1,1,1,4},{1,1,1,4},{1,1,-1,4},{1,1,-1,4},         /* 011x xxxx xxxx xxxx x */
	};

	static const DCT_COEFFICIENT_VLC_ELEMENT table_k[] = {
		{0,0,0,2},{0,0,0,2},{1,0,1,3},{1,0,-1,3},          /* 1xxx xxxx xxxx xxxx x */
	};

	code = vs_read_bits(in, 17);
	n = log2(code);

	switch(n){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		*run = 0;
		*level = 0;
		return -1;
	case 5:
		code -= 0x20;
		vs_erase_bits(in, table_a[code].size);
		*run = table_a[code].run;
		*level = table_a[code].level;
		return table_a[code].flag;
	case 6:
		code >>= 1;
		code -= 0x20;
		vs_erase_bits(in, table_b[code].size);
		*run = table_b[code].run;
		*level = table_b[code].level;
		return table_b[code].flag;
	case 7:
		code >>= 2;
		code -= 0x20;
		vs_erase_bits(in, table_c[code].size);
		*run = table_c[code].run;
		*level = table_c[code].level;
		return table_c[code].flag;
	case 8:
		code >>= 3;
		code -= 0x20;
		vs_erase_bits(in, table_d[code].size);
		*run = table_d[code].run;
		*level = table_d[code].level;
		return table_d[code].flag;
	case 9:
		code >>= 4;
		code -= 0x20;
		vs_erase_bits(in, table_e[code].size);
		*run = table_e[code].run;
		*level = table_e[code].level;
		return table_e[code].flag;
	case 10:
		code >>= 6;
		code -= 0x10;
		vs_erase_bits(in, table_f[code].size);
		*run = table_f[code].run;
		*level = table_f[code].level;
		return table_e[code].flag;
	case 11:
		/* escape */
		code >>= 5;
		code &= 0x3f;
		vs_erase_bits(in, 12);
		*run = code;
		*level = get_dct_coefficient_escape_level_mpeg1(in);
		return 1;
	case 12:
		code >>= 9;
		code -= 0x08;
		vs_erase_bits(in, table_g[code].size);
		*run = table_g[code].run;
		*level = table_g[code].level;
		return table_g[code].flag;
	case 13:
		code >>= 10;
		code -= 0x08;
		vs_erase_bits(in, table_h[code].size);
		*run = table_h[code].run;
		*level = table_h[code].level;
		return table_h[code].flag;
	case 14:
		code >>= 8;
		code -= 0x40;
		vs_erase_bits(in, table_i[code].size);
		*run = table_i[code].run;
		*level = table_i[code].level;
		return table_i[code].flag;
	case 15:
		code >>= 12;
		code -= 0x08;
		vs_erase_bits(in, table_j[code].size);
		*run = table_j[code].run;
		*level = table_j[code].level;
		return table_j[code].flag;
	case 16:
		code >>= 14;
		code -= 0x4;
		vs_erase_bits(in, table_k[code].size);
		*run = table_k[code].run;
		*level = table_k[code].level;
		return table_k[code].flag;
	}

	*run = 0;
	*level = 0;
	return -1;
}

static int get_dct_coefficient_escape_level_mpeg2(VIDEO_STREAM *in)
{
	int n;
	int code;
	
	static const unsigned int sign[] = {
		0, 0xFFFFF000,
	};

	code = vs_get_bits(in, 12);
	n = code >> 11;
	code |= sign[n];

	return code;
}

static int get_dct_coefficient_escape_level_mpeg1(VIDEO_STREAM *in)
{
	int code;
	
	code = vs_get_bits(in, 8);
	if(code == 0){
		code = vs_get_bits(in, 8);
	}else if(code==128){
		code = vs_get_bits(in, 8) - 256;
	}else if(code>128){
		code -= 256;
	}

	return code;
}

