/*
 * 	RGB to YCbCr
 *
 * 		Y  =  0.257R + 0.504G + 0.098B +  16
 * 		Cb = -0.148R - 0.291G + 0.439B + 128
 * 		Cr =  0.439R - 0.368G - 0.071B + 128
 */

/*[0x00]*//* 0x80,	0x77,   0x7f,   0x7f, */
static unsigned char rm_default_palette[] = {
/*[0x00]*/ 0x00,        0x00,   0x00,   0x00,
/*[0x01]*/ 0xbf,	0x17,	0x7b,	0x7a,
/*[0x02]*/ 0xff,	0x90,	0x35,	0x22,
/*[0x03]*/ 0xff,	0xa9,	0xa5,	0x0f,
/*[0x04]*/ 0xff,	0x28,	0xef,	0x6d,
/*[0x05]*/ 0xff,	0x6a,	0xca,	0xdd,
/*[0x06]*/ 0xff,	0xea,	0x7f,	0x7f,
/*[0x07]*/ 0xff,	0xd5,	0x7f,	0x7f,
/*[0x08]*/ 0xff,	0xca,	0x7f,	0x7f,
/*[0x09]*/ 0xff,	0xbf,	0x7f,	0x7f,
/*[0x0a]*/ 0xff,	0xb4,	0x7f,	0x7f,
/*[0x0b]*/ 0xff,	0xa9,	0x7f,	0x7f,
/*[0x0c]*/ 0xff,	0x9e,	0x7f,	0x7f,
/*[0x0d]*/ 0xff,	0x93,	0x7f,	0x7f,
/*[0x0e]*/ 0xff,	0x88,	0x7f,	0x7f,
/*[0x0f]*/ 0xff,	0x7d,	0x7f,	0x7f,
/*[0x10]*/ 0xff,	0x4d,	0x70,	0xdf,
/*[0x11]*/ 0xff,	0xca,	0x14,	0x93,
/*[0x12]*/ 0xff,	0x69,	0x7c,	0x3e,
/*[0x13]*/ 0xff,	0x80,	0xb3,	0x2d,
/*[0x14]*/ 0xff,	0x34,	0xa8,	0x8b,
/*[0x15]*/ 0xff,	0x56,	0x96,	0xda,
/*[0x16]*/ 0xff,	0x72,	0x7f,	0x7f,
/*[0x17]*/ 0xff,	0x67,	0x7f,	0x7f,
/*[0x18]*/ 0xff,	0x5c,	0x7f,	0x7f,
/*[0x19]*/ 0xff,	0x52,	0x7f,	0x7f,
/*[0x1a]*/ 0xff,	0x46,	0x7f,	0x7f,
/*[0x1b]*/ 0xff,	0x3b,	0x7f,	0x7f,
/*[0x1c]*/ 0xff,	0x30,	0x7f,	0x7f,
/*[0x1d]*/ 0xff,	0x26,	0x7f,	0x7f,
/*[0x1e]*/ 0xff,	0x1b,	0x7f,	0x7f,
/*[0x1f]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x20]*/ 0xff,	0xa9,	0x62,	0xa8,
/*[0x21]*/ 0xff,	0xb6,	0x5f,	0xa0,
/*[0x22]*/ 0xff,	0xc4,	0x5c,	0x98,
/*[0x23]*/ 0xff,	0xde,	0x56,	0x89,
/*[0x24]*/ 0xff,	0xc3,	0x66,	0x79,
/*[0x25]*/ 0xff,	0xb5,	0x6e,	0x70,
/*[0x26]*/ 0xff,	0xa8,	0x75,	0x66,
/*[0x27]*/ 0xff,	0xae,	0x88,	0x64,
/*[0x28]*/ 0xff,	0xb4,	0x9a,	0x60,
/*[0x29]*/ 0xff,	0xa0,	0x96,	0x73,
/*[0x2a]*/ 0xff,	0x94,	0x95,	0x7c,
/*[0x2b]*/ 0xff,	0x89,	0x94,	0x85,
/*[0x2c]*/ 0xff,	0x93,	0x8f,	0x8d,
/*[0x2d]*/ 0xff,	0x9d,	0x8a,	0x95,
/*[0x2e]*/ 0xff,	0xb1,	0x80,	0xa2,
/*[0x2f]*/ 0xff,	0xad,	0x71,	0xa5,
/*[0x30]*/ 0xff,	0x89,	0x5d,	0xba,
/*[0x31]*/ 0xff,	0xa0,	0x54,	0xad,
/*[0x32]*/ 0xff,	0xb3,	0x4c,	0xa2,
/*[0x33]*/ 0xff,	0xd6,	0x3f,	0x8d,
/*[0x34]*/ 0xff,	0xb0,	0x59,	0x74,
/*[0x35]*/ 0xff,	0x9c,	0x66,	0x64,
/*[0x36]*/ 0xff,	0x81,	0x76,	0x4a,
/*[0x37]*/ 0xff,	0x8d,	0x91,	0x49,
/*[0x38]*/ 0xff,	0x95,	0xa9,	0x45,
/*[0x39]*/ 0xff,	0x81,	0xa0,	0x68,
/*[0x3a]*/ 0xff,	0x72,	0x9e,	0x77,
/*[0x3b]*/ 0xff,	0x62,	0x9c,	0x86,
/*[0x3c]*/ 0xff,	0x6e,	0x95,	0x93,
/*[0x3d]*/ 0xff,	0x7b,	0x8f,	0x9f,
/*[0x3e]*/ 0xff,	0x92,	0x83,	0xb5,
/*[0x3f]*/ 0xff,	0x8e,	0x71,	0xb7,
/*[0x40]*/ 0xff,	0x4d,	0x70,	0xdf,
/*[0x41]*/ 0xff,	0x80,	0x4b,	0xbe,
/*[0x42]*/ 0xff,	0x9b,	0x2f,	0xaf,
/*[0x43]*/ 0xff,	0xca,	0x14,	0x93,
/*[0x44]*/ 0xff,	0x97,	0x42,	0x6b,
/*[0x45]*/ 0xff,	0x6e,	0x66,	0x3b,
/*[0x46]*/ 0xff,	0x51,	0x5a,	0xef,
/*[0x47]*/ 0xff,	0x72,	0x95,	0x38,
/*[0x48]*/ 0xff,	0x80,	0xb3,	0x2d,
/*[0x49]*/ 0xff,	0x56,	0xb0,	0x4c,
/*[0x4a]*/ 0xff,	0x4d,	0xa6,	0x72,
/*[0x4b]*/ 0xff,	0x34,	0xa8,	0x8b,
/*[0x4c]*/ 0xff,	0x3a,	0xa4,	0xa0,
/*[0x4d]*/ 0xff,	0x41,	0xa0,	0xb7,
/*[0x4e]*/ 0xff,	0x56,	0x96,	0xda,
/*[0x4f]*/ 0xff,	0x52,	0x84,	0xdc,
/*[0x50]*/ 0xff,	0x38,	0x74,	0xc0,
/*[0x51]*/ 0xff,	0x59,	0x5c,	0xab,
/*[0x52]*/ 0xff,	0x6d,	0x49,	0x9f,
/*[0x53]*/ 0xff,	0x8d,	0x37,	0x8d,
/*[0x54]*/ 0xff,	0x69,	0x55,	0x72,
/*[0x55]*/ 0xff,	0x4e,	0x6d,	0x52,
/*[0x56]*/ 0xff,	0xd2,	0x0f,	0x92,
/*[0x57]*/ 0xff,	0x51,	0x8e,	0x50,
/*[0x58]*/ 0xff,	0x5b,	0xa2,	0x48,
/*[0x59]*/ 0xff,	0x3d,	0xa0,	0x5e,
/*[0x5a]*/ 0xff,	0x36,	0x9a,	0x76,
/*[0x5b]*/ 0xff,	0x25,	0x9b,	0x88,
/*[0x5c]*/ 0xff,	0x2a,	0x98,	0x95,
/*[0x5d]*/ 0xff,	0x30,	0x95,	0xa4,
/*[0x5e]*/ 0xff,	0x3f,	0x8d,	0xbc,
/*[0x5f]*/ 0xff,	0x3c,	0x82,	0xbe,
/*[0x60]*/ 0xff,	0x2d,	0x76,	0xae,
/*[0x61]*/ 0xff,	0x44,	0x65,	0xa0,
/*[0x62]*/ 0xff,	0x53,	0x59,	0x97,
/*[0x63]*/ 0xff,	0x6b,	0x4b,	0x89,
/*[0x64]*/ 0xff,	0x4f,	0x60,	0x75,
/*[0x65]*/ 0xff,	0x3c,	0x72,	0x5f,
/*[0x66]*/ 0xff,	0x3a,	0x7d,	0x61,
/*[0x67]*/ 0xff,	0x3e,	0x8a,	0x5d,
/*[0x68]*/ 0xff,	0x46,	0x99,	0x58,
/*[0x69]*/ 0xff,	0x30,	0x97,	0x68,
/*[0x6a]*/ 0xff,	0x29,	0x93,	0x79,
/*[0x6b]*/ 0xff,	0x1e,	0x94,	0x85,
/*[0x6c]*/ 0xff,	0x22,	0x91,	0x8f,
/*[0x6d]*/ 0xff,	0x27,	0x8e,	0x99,
/*[0x6e]*/ 0xff,	0x32,	0x89,	0xac,
/*[0x6f]*/ 0xff,	0x2f,	0x80,	0xad,
/*[0x70]*/ 0xff,	0xb0,	0x70,	0x89,
/*[0x71]*/ 0xff,	0x8a,	0x73,	0x89,
/*[0x72]*/ 0xff,	0x69,	0x76,	0x88,
/*[0x73]*/ 0xff,	0x4d,	0x7a,	0x86,
/*[0x74]*/ 0xff,	0x36,	0x7e,	0x84,
/*[0x75]*/ 0xff,	0xa0,	0x62,	0x93,
/*[0x76]*/ 0xff,	0x85,	0x63,	0x94,
/*[0x77]*/ 0xff,	0x6c,	0x65,	0x94,
/*[0x78]*/ 0xff,	0x57,	0x67,	0x93,
/*[0x79]*/ 0xff,	0x45,	0x6b,	0x92,
/*[0x7a]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x7b]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x7c]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x7d]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x7e]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x7f]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x80]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x81]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x82]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x83]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x84]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x85]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x86]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x87]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x88]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x89]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x8a]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x8b]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x8c]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x8d]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x8e]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x8f]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x90]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x91]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x92]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x93]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x94]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x95]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x96]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x97]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x98]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x99]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x9a]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x9b]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x9c]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x9d]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x9e]*/ 0xff,	0x10,	0x80,	0x80,
/*[0x9f]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa0]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa1]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa2]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa3]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa4]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa5]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa6]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa7]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa8]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xa9]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xaa]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xab]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xac]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xad]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xae]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xaf]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb0]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb1]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb2]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb3]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb4]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb5]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb6]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb7]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb8]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xb9]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xba]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xbb]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xbc]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xbd]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xbe]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xbf]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc0]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc1]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc2]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc3]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc4]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc5]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc6]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc7]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc8]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xc9]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xca]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xcb]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xcc]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xcd]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xce]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xcf]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd0]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd1]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd2]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd3]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd4]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd5]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd6]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd7]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd8]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xd9]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xda]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xdb]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xdc]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xdd]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xde]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xdf]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe0]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe1]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe2]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe3]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe4]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe5]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe6]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe7]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe8]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xe9]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xea]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xeb]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xec]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xed]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xee]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xef]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf0]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf1]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf2]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf3]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf4]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf5]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf6]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf7]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf8]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xf9]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xfa]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xfb]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xfc]*/ 0xff,	0x10,	0x80,	0x80,
/*[0xfd]*/ 0xff,	0xea,	0x7f,	0x7f,
/*[0xfe]*/ 0x55,	0x00,	0x80,	0x80,
/*[0xff]*/ 0x00,	0x00,	0x00,	0x00
};
