#ifdef USE_CLASS_HID_COLORS
const ULONG Class_HID_colors[96] =
{
	0x96969696,0x96969696,0x96969696,
	0x2d2d2d2d,0x28282828,0x9e9e9e9e,
	0x00000000,0x65656565,0x9a9a9a9a,
	0x35353535,0x75757575,0xaaaaaaaa,
	0x65656565,0x8a8a8a8a,0xbabababa,
	0x0c0c0c0c,0x61616161,0xffffffff,
	0x24242424,0x5d5d5d5d,0x24242424,
	0x35353535,0x8a8a8a8a,0x35353535,
	0x86868686,0xb2b2b2b2,0x3d3d3d3d,
	0x0c0c0c0c,0xe3e3e3e3,0x00000000,
	0x4d4d4d4d,0x9e9e9e9e,0x8e8e8e8e,
	0x82828282,0x00000000,0x00000000,
	0xdfdfdfdf,0x35353535,0x35353535,
	0xdbdbdbdb,0x65656565,0x39393939,
	0xdbdbdbdb,0x8e8e8e8e,0x41414141,
	0xdfdfdfdf,0xbabababa,0x45454545,
	0xefefefef,0xe7e7e7e7,0x14141414,
	0x82828282,0x61616161,0x4d4d4d4d,
	0xa6a6a6a6,0x7e7e7e7e,0x61616161,
	0xcacacaca,0x9a9a9a9a,0x75757575,
	0x9a9a9a9a,0x55555555,0xaaaaaaaa,
	0xffffffff,0x00000000,0xffffffff,
	0xffffffff,0xffffffff,0xffffffff,
	0xdfdfdfdf,0xdfdfdfdf,0xdfdfdfdf,
	0xcacacaca,0xcacacaca,0xcacacaca,
	0xbabababa,0xbabababa,0xbabababa,
	0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
	0x8a8a8a8a,0x8a8a8a8a,0x8a8a8a8a,
	0x65656565,0x65656565,0x65656565,
	0x4d4d4d4d,0x4d4d4d4d,0x4d4d4d4d,
	0x3c3c3c3c,0x3c3c3c3c,0x3b3b3b3b,
	0x00000000,0x00000000,0x00000000,
};
#endif

#define CLASS_HID_WIDTH        16
#define CLASS_HID_HEIGHT       16
#define CLASS_HID_DEPTH         5
#define CLASS_HID_COMPRESSION   1
#define CLASS_HID_MASKING       2

#ifdef USE_CLASS_HID_HEADER
const struct BitMapHeader Class_HID_header =
{ 16,16,88,140,5,2,1,0,0,1,1,800,600 };
#endif

#ifdef USE_CLASS_HID_BODY
const UBYTE Class_HID_body[223] = {
0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,
0x00,0xff,0x00,0xff,0x00,0x01,0x02,0x80,0x01,0x0b,0x40,0x01,0x03,0x00,0x01,
0x0c,0xc0,0x01,0x0f,0xc0,0x01,0x1f,0xa0,0x01,0x1f,0xf0,0x01,0x1f,0xe0,0x01,
0x20,0x10,0x01,0x3f,0xf0,0x01,0x4d,0x00,0x01,0x79,0xe0,0x01,0x79,0xe0,0x01,
0x06,0x18,0x01,0x7f,0xf8,0x01,0x52,0x0c,0x01,0x73,0xf8,0x01,0x73,0xf8,0x01,
0x8c,0x04,0x01,0xff,0xfc,0x01,0xf8,0x24,0x01,0xff,0xfe,0x01,0xff,0xfc,0x01,
0x00,0x02,0x01,0xff,0xfe,0x01,0xe6,0x71,0x01,0x67,0xf8,0x01,0x67,0xf9,0x01,
0x98,0x05,0x01,0xff,0xfd,0x01,0x81,0xa1,0x01,0x01,0xe5,0x01,0x81,0xe3,0x01,
0xbe,0x1f,0x01,0xbf,0xff,0x01,0x9b,0xd5,0x01,0xd0,0xdc,0x01,0x60,0xd3,0x01,
0xff,0x2f,0xff,0xff,0x01,0x54,0x9a,0x01,0x57,0x78,0x01,0x38,0x07,0x01,0x7f,
0xff,0x01,0x7f,0xff,0x01,0x3a,0xe1,0x01,0x34,0xed,0x01,0x0f,0x1e,0x01,0x3f,
0xff,0x01,0x3f,0xff,0x01,0x09,0x13,0x01,0x08,0xe3,0x01,0x07,0xfc,0x01,0x0f,
0xff,0x01,0x0f,0xff,0x01,0x03,0x0c,0x01,0x03,0x0c,0x01,0x00,0xf0,0x01,0x03,
0xfc,0x01,0x03,0xfc,0x01,0x00,0xf0,0x01,0x00,0xf0,0xff,0x00,0x01,0x00,0xf0,
0x01,0x00,0xf0,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00, };
#endif
