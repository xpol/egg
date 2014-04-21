#include "egg.h"

#include <string.h>
#include <stdlib.h>

#include "internal.h"

typedef struct EGGContext {
	EGGErrorCode errorcode;

	_EGGImage surface;
	void* buffers[2];
	EGGubyte current;
	EGGubyte alpha;
	EGGboolean colorkey;
	EGGuint colorkey_value;
} EGGContext;

/**
 * The drawing information for drawing a image region.
 */
typedef struct EGGMetrics
{
    int sx; /** The source region x position in the image. */
    int sy; /** The source region y position in the image. */ 
    int dx; /** The destination x position in the drawing context. */
    int dy; /** The destination y position in the drawing context. */
    int w;  /** The width of the region */
    int h;  /** The height of the region */
} EGGMetrics;

static const unsigned short expand_5_to_6[] = {
    0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,
    0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1E,
    0x20,0x22,0x24,0x26,0x28,0x2A,0x2C,0x2E,
    0x30,0x32,0x34,0x36,0x38,0x3A,0x3C,0x3F,
};
static const unsigned short expand_5bits_to_RGB565_G6[] = {
    0x00<<5,0x02<<5,0x04<<5,0x06<<5,0x08<<5,0x0A<<5,0x0C<<5,0x0E<<5,
    0x10<<5,0x12<<5,0x14<<5,0x16<<5,0x18<<5,0x1A<<5,0x1C<<5,0x1E<<5,
    0x20<<5,0x22<<5,0x24<<5,0x26<<5,0x28<<5,0x2A<<5,0x2C<<5,0x2E<<5,
    0x30<<5,0x32<<5,0x34<<5,0x36<<5,0x38<<5,0x3A<<5,0x3C<<5,0x3F<<5,
};

static const unsigned short expand_4_to_R5[16] = {
    0x00<<11,0x02<<11,0x04<<11,0x06<<11,0x08<<11,0x0A<<11,0x0C<<11,0x0E<<11,0x10<<11,0x12<<11,0x14<<11,0x16<<11,0x18<<11,0x1A<<11,0x1C<<11,0x1F<<11
};
static const unsigned short expand_4_to_G6[16] = {
    0x00<<5,0x04<<5,0x08<<5,0x0C<<5,0x10<<5,0x15<<5,0x19<<5,0x1D<<5,0x21<<5,0x25<<5,0x2A<<5,0x2E<<5,0x32<<5,0x36<<5,0x3A<<5,0x3F<<5
};


// Generated by tools/BxL.lua writen by xpol.
// TO regenerate `BxL.lua > BxL.inline.c`


static const unsigned char _4x4[16][16] = {
// L:  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15  
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //  0
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}, //  1
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02}, //  2
    {0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x03}, //  3
    {0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x04}, //  4
    {0x00,0x00,0x00,0x01,0x01,0x01,0x02,0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,0x05}, //  5
    {0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x02,0x03,0x03,0x04,0x04,0x04,0x05,0x05,0x06}, //  6
    {0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07}, //  7
    {0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x08}, //  8
    {0x00,0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x04,0x05,0x06,0x06,0x07,0x07,0x08,0x09}, //  9
    {0x00,0x00,0x01,0x02,0x02,0x03,0x04,0x04,0x05,0x06,0x06,0x07,0x08,0x08,0x09,0x0A}, // 10
    {0x00,0x00,0x01,0x02,0x02,0x03,0x04,0x05,0x05,0x06,0x07,0x08,0x08,0x09,0x0A,0x0B}, // 11
    {0x00,0x00,0x01,0x02,0x03,0x04,0x04,0x05,0x06,0x07,0x08,0x08,0x09,0x0A,0x0B,0x0C}, // 12
    {0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D}, // 13
    {0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E}, // 14
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}, // 15
};
static const unsigned char _5x4[32][16] = {
// L:  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15  
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //  0
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}, //  1
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02}, //  2
    {0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x03}, //  3
    {0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x04}, //  4
    {0x00,0x00,0x00,0x01,0x01,0x01,0x02,0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,0x05}, //  5
    {0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x02,0x03,0x03,0x04,0x04,0x04,0x05,0x05,0x06}, //  6
    {0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07}, //  7
    {0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x08}, //  8
    {0x00,0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x04,0x05,0x06,0x06,0x07,0x07,0x08,0x09}, //  9
    {0x00,0x00,0x01,0x02,0x02,0x03,0x04,0x04,0x05,0x06,0x06,0x07,0x08,0x08,0x09,0x0A}, // 10
    {0x00,0x00,0x01,0x02,0x02,0x03,0x04,0x05,0x05,0x06,0x07,0x08,0x08,0x09,0x0A,0x0B}, // 11
    {0x00,0x00,0x01,0x02,0x03,0x04,0x04,0x05,0x06,0x07,0x08,0x08,0x09,0x0A,0x0B,0x0C}, // 12
    {0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D}, // 13
    {0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E}, // 14
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}, // 15
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x10}, // 16
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x11}, // 17
    {0x00,0x01,0x02,0x03,0x04,0x06,0x07,0x08,0x09,0x0A,0x0C,0x0D,0x0E,0x0F,0x10,0x12}, // 18
    {0x00,0x01,0x02,0x03,0x05,0x06,0x07,0x08,0x0A,0x0B,0x0C,0x0D,0x0F,0x10,0x11,0x13}, // 19
    {0x00,0x01,0x02,0x04,0x05,0x06,0x08,0x09,0x0A,0x0C,0x0D,0x0E,0x10,0x11,0x12,0x14}, // 20
    {0x00,0x01,0x02,0x04,0x05,0x07,0x08,0x09,0x0B,0x0C,0x0E,0x0F,0x10,0x12,0x13,0x15}, // 21
    {0x00,0x01,0x02,0x04,0x05,0x07,0x08,0x0A,0x0B,0x0D,0x0E,0x10,0x11,0x13,0x14,0x16}, // 22
    {0x00,0x01,0x03,0x04,0x06,0x07,0x09,0x0A,0x0C,0x0D,0x0F,0x10,0x12,0x13,0x15,0x17}, // 23
    {0x00,0x01,0x03,0x04,0x06,0x08,0x09,0x0B,0x0C,0x0E,0x10,0x11,0x13,0x14,0x16,0x18}, // 24
    {0x00,0x01,0x03,0x05,0x06,0x08,0x0A,0x0B,0x0D,0x0F,0x10,0x12,0x14,0x15,0x17,0x19}, // 25
    {0x00,0x01,0x03,0x05,0x06,0x08,0x0A,0x0C,0x0D,0x0F,0x11,0x13,0x14,0x16,0x18,0x1A}, // 26
    {0x00,0x01,0x03,0x05,0x07,0x09,0x0A,0x0C,0x0E,0x10,0x12,0x13,0x15,0x17,0x19,0x1B}, // 27
    {0x00,0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C}, // 28
    {0x00,0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F,0x11,0x13,0x15,0x17,0x19,0x1B,0x1D}, // 29
    {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1E}, // 30
    {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1F}, // 31
};
static const unsigned char _6x4[64][16] = {
// L:  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15  
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //  0
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}, //  1
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02}, //  2
    {0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x03}, //  3
    {0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x04}, //  4
    {0x00,0x00,0x00,0x01,0x01,0x01,0x02,0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,0x05}, //  5
    {0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x02,0x03,0x03,0x04,0x04,0x04,0x05,0x05,0x06}, //  6
    {0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07}, //  7
    {0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x08}, //  8
    {0x00,0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x04,0x05,0x06,0x06,0x07,0x07,0x08,0x09}, //  9
    {0x00,0x00,0x01,0x02,0x02,0x03,0x04,0x04,0x05,0x06,0x06,0x07,0x08,0x08,0x09,0x0A}, // 10
    {0x00,0x00,0x01,0x02,0x02,0x03,0x04,0x05,0x05,0x06,0x07,0x08,0x08,0x09,0x0A,0x0B}, // 11
    {0x00,0x00,0x01,0x02,0x03,0x04,0x04,0x05,0x06,0x07,0x08,0x08,0x09,0x0A,0x0B,0x0C}, // 12
    {0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D}, // 13
    {0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E}, // 14
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}, // 15
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x10}, // 16
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x11}, // 17
    {0x00,0x01,0x02,0x03,0x04,0x06,0x07,0x08,0x09,0x0A,0x0C,0x0D,0x0E,0x0F,0x10,0x12}, // 18
    {0x00,0x01,0x02,0x03,0x05,0x06,0x07,0x08,0x0A,0x0B,0x0C,0x0D,0x0F,0x10,0x11,0x13}, // 19
    {0x00,0x01,0x02,0x04,0x05,0x06,0x08,0x09,0x0A,0x0C,0x0D,0x0E,0x10,0x11,0x12,0x14}, // 20
    {0x00,0x01,0x02,0x04,0x05,0x07,0x08,0x09,0x0B,0x0C,0x0E,0x0F,0x10,0x12,0x13,0x15}, // 21
    {0x00,0x01,0x02,0x04,0x05,0x07,0x08,0x0A,0x0B,0x0D,0x0E,0x10,0x11,0x13,0x14,0x16}, // 22
    {0x00,0x01,0x03,0x04,0x06,0x07,0x09,0x0A,0x0C,0x0D,0x0F,0x10,0x12,0x13,0x15,0x17}, // 23
    {0x00,0x01,0x03,0x04,0x06,0x08,0x09,0x0B,0x0C,0x0E,0x10,0x11,0x13,0x14,0x16,0x18}, // 24
    {0x00,0x01,0x03,0x05,0x06,0x08,0x0A,0x0B,0x0D,0x0F,0x10,0x12,0x14,0x15,0x17,0x19}, // 25
    {0x00,0x01,0x03,0x05,0x06,0x08,0x0A,0x0C,0x0D,0x0F,0x11,0x13,0x14,0x16,0x18,0x1A}, // 26
    {0x00,0x01,0x03,0x05,0x07,0x09,0x0A,0x0C,0x0E,0x10,0x12,0x13,0x15,0x17,0x19,0x1B}, // 27
    {0x00,0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C}, // 28
    {0x00,0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F,0x11,0x13,0x15,0x17,0x19,0x1B,0x1D}, // 29
    {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1E}, // 30
    {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1F}, // 31
    {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x11,0x13,0x15,0x17,0x19,0x1B,0x1D,0x20}, // 32
    {0x00,0x02,0x04,0x06,0x08,0x0B,0x0D,0x0F,0x11,0x13,0x16,0x18,0x1A,0x1C,0x1E,0x21}, // 33
    {0x00,0x02,0x04,0x06,0x09,0x0B,0x0D,0x0F,0x12,0x14,0x16,0x18,0x1B,0x1D,0x1F,0x22}, // 34
    {0x00,0x02,0x04,0x07,0x09,0x0B,0x0E,0x10,0x12,0x15,0x17,0x19,0x1C,0x1E,0x20,0x23}, // 35
    {0x00,0x02,0x04,0x07,0x09,0x0C,0x0E,0x10,0x13,0x15,0x18,0x1A,0x1C,0x1F,0x21,0x24}, // 36
    {0x00,0x02,0x04,0x07,0x09,0x0C,0x0E,0x11,0x13,0x16,0x18,0x1B,0x1D,0x20,0x22,0x25}, // 37
    {0x00,0x02,0x05,0x07,0x0A,0x0C,0x0F,0x11,0x14,0x16,0x19,0x1B,0x1E,0x20,0x23,0x26}, // 38
    {0x00,0x02,0x05,0x07,0x0A,0x0D,0x0F,0x12,0x14,0x17,0x1A,0x1C,0x1F,0x21,0x24,0x27}, // 39
    {0x00,0x02,0x05,0x08,0x0A,0x0D,0x10,0x12,0x15,0x18,0x1A,0x1D,0x20,0x22,0x25,0x28}, // 40
    {0x00,0x02,0x05,0x08,0x0A,0x0D,0x10,0x13,0x15,0x18,0x1B,0x1E,0x20,0x23,0x26,0x29}, // 41
    {0x00,0x02,0x05,0x08,0x0B,0x0E,0x10,0x13,0x16,0x19,0x1C,0x1E,0x21,0x24,0x27,0x2A}, // 42
    {0x00,0x02,0x05,0x08,0x0B,0x0E,0x11,0x14,0x16,0x19,0x1C,0x1F,0x22,0x25,0x28,0x2B}, // 43
    {0x00,0x02,0x05,0x08,0x0B,0x0E,0x11,0x14,0x17,0x1A,0x1D,0x20,0x23,0x26,0x29,0x2C}, // 44
    {0x00,0x03,0x06,0x09,0x0C,0x0F,0x12,0x15,0x18,0x1B,0x1E,0x21,0x24,0x27,0x2A,0x2D}, // 45
    {0x00,0x03,0x06,0x09,0x0C,0x0F,0x12,0x15,0x18,0x1B,0x1E,0x21,0x24,0x27,0x2A,0x2E}, // 46
    {0x00,0x03,0x06,0x09,0x0C,0x0F,0x12,0x15,0x19,0x1C,0x1F,0x22,0x25,0x28,0x2B,0x2F}, // 47
    {0x00,0x03,0x06,0x09,0x0C,0x10,0x13,0x16,0x19,0x1C,0x20,0x23,0x26,0x29,0x2C,0x30}, // 48
    {0x00,0x03,0x06,0x09,0x0D,0x10,0x13,0x16,0x1A,0x1D,0x20,0x23,0x27,0x2A,0x2D,0x31}, // 49
    {0x00,0x03,0x06,0x0A,0x0D,0x10,0x14,0x17,0x1A,0x1E,0x21,0x24,0x28,0x2B,0x2E,0x32}, // 50
    {0x00,0x03,0x06,0x0A,0x0D,0x11,0x14,0x17,0x1B,0x1E,0x22,0x25,0x28,0x2C,0x2F,0x33}, // 51
    {0x00,0x03,0x06,0x0A,0x0D,0x11,0x14,0x18,0x1B,0x1F,0x22,0x26,0x29,0x2D,0x30,0x34}, // 52
    {0x00,0x03,0x07,0x0A,0x0E,0x11,0x15,0x18,0x1C,0x1F,0x23,0x26,0x2A,0x2D,0x31,0x35}, // 53
    {0x00,0x03,0x07,0x0A,0x0E,0x12,0x15,0x19,0x1C,0x20,0x24,0x27,0x2B,0x2E,0x32,0x36}, // 54
    {0x00,0x03,0x07,0x0B,0x0E,0x12,0x16,0x19,0x1D,0x21,0x24,0x28,0x2C,0x2F,0x33,0x37}, // 55
    {0x00,0x03,0x07,0x0B,0x0E,0x12,0x16,0x1A,0x1D,0x21,0x25,0x29,0x2C,0x30,0x34,0x38}, // 56
    {0x00,0x03,0x07,0x0B,0x0F,0x13,0x16,0x1A,0x1E,0x22,0x26,0x29,0x2D,0x31,0x35,0x39}, // 57
    {0x00,0x03,0x07,0x0B,0x0F,0x13,0x17,0x1B,0x1E,0x22,0x26,0x2A,0x2E,0x32,0x36,0x3A}, // 58
    {0x00,0x03,0x07,0x0B,0x0F,0x13,0x17,0x1B,0x1F,0x23,0x27,0x2B,0x2F,0x33,0x37,0x3B}, // 59
    {0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24,0x28,0x2C,0x30,0x34,0x38,0x3C}, // 60
    {0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24,0x28,0x2C,0x30,0x34,0x38,0x3D}, // 61
    {0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x21,0x25,0x29,0x2D,0x31,0x35,0x39,0x3E}, // 62
    {0x00,0x04,0x08,0x0C,0x10,0x15,0x19,0x1D,0x21,0x25,0x2A,0x2E,0x32,0x36,0x3A,0x3F}, // 63
};


#define expand_4_to_B5  _5x4[0x1F]

#define ALPHA_BITS          (4)
#define ALPHA_LEVELS        (1<<ALPHA_BITS)
#define ALPHA_MAX           (ALPHA_LEVELS-1)



#define RGB565_R5bits(px)          ((px) >>11)
#define RGB565_G6bits(px)         (((px) & 0x07E0) >> 5 )
#define RGB565_B5bits(px)          ((px) & 0x001F)

#define BGR565_R(px)         RGB565_B5bits(px)
#define BGR565_G(px)         RGB565_G5(px)
#define BGR565_B(px)         RGB565_R5bits(px)

#define RGBA5551_R5bits(px)      ((px) >> 11) 
#define RGBA5551_G5bits(px)     (((px) & 0x07C0) >> 6)
#define RGBA5551_G6bits(px)     expand_5_to_6[RGBA5551_G5bits(px)]
#define RGBA5551_B5(px)     (((px) & 0x003E) >> 1)
#define RGBA5551_A(px)      ((px) & 0x0001)

#define ARGB1555_R5bits(px)     (((px)>>10) & 0x001F)
#define ARGB1555_G5bits(px)     (((px)>>5 ) & 0x001F)
#define ARGB1555_G6bits(px)     expand_5_to_6[ARGB1555_G5bits(px)]
#define ARGB1555_B5bits(px)     ((px) & 0x1F)


#define ARGB1555_A(px)       ((px) >> 15)

#define ARGB1555_R(px)      (((px) & 0x7C00) >> 10) 
#define ARGB1555_G(px)      (((px) & 0x03E0) >> 5)
#define ARGB1555_B(px)       ((px) & 0x001F)
#define ARGB1555_A(px)       ((px) >> 15)

#define RGBA4444_R5(px)     (((px) & 0xF000) >> (12-1)) 
#define RGBA4444_G6(px)     (((px) & 0x0F00) >> (8-2))
#define RGBA4444_B5(px)     (((px) & 0x00F0) >> (4-1))
#define RGBA4444_A5(px)     (((px) & 0x000F) << (ALPHA_BITS-4))

#define RGBA4444_R(px)      ((px)>>12)
#define RGBA4444_G(px)     (((px)>>8)&0xF)
#define RGBA4444_B(px)     (((px)>>4)&0xF)
#define RGBA4444_A(px)     ((px)&0xF)

#define ARGB8888_R5(px)     (((px) & 0x00FF0000) >> (16+3)) 
#define ARGB8888_G6(px)     (((px) & 0x0000FF00) >> (8+2))
#define ARGB8888_B5(px)     (((px) & 0x000000FF) >> (0+3))
#define ARGB8888_A4(px)     (((px) & 0xFF000000) >> (24+4))
#define ARGB8888_R8bits(px) (((px)>>16) & 0x000000FF)
#define ARGB8888_G8bits(px) (((px)>>8 ) & 0x000000FF)
#define ARGB8888_B8bits(px) ( (px)      & 0x000000FF)
#define ARGB8888_A8bits(px) (((px)>>24) & 0x000000FF)
#define ARGB8888_TO_RGB565   | ((px>>3)&0x1F)




#define INVERT_ALPHA(global_A8, source_A4)        ((ALPHA_LEVELS-1) - _5x4[global_A8>>(8-ALPHA_BITS)][source_A4])

/**
 * lookup tables:
 * for TABLE = _<B>x<L>[1<<B][1<<L]:
 *  TABLE[a][b] == (uint8)a * b / ((1 << L) - 1);
 *  and TABLE[a][b] have mas B bits valid bits.
 */


#define def_y_iterator() \
    const unsigned char* yi_src; \
    unsigned char* yi_dst; \
    unsigned char* yi_dst_end


#define def_x_iterator(T, metrics) \
    T px; \
    const T* xi_src; \
    unsigned short* xi_dst; \
    unsigned short* xi_dst_end

#define yi_init_dst(dst, metrics) \
    (yi_dst = dst->pixels + metrics->dy * dst->pitch + (metrics->dx<<1))

#define yi_init_dst_end(dst, metrics) \
    (yi_dst_end = yi_dst + metrics->h * dst->pitch)

#define yi_init_src(src, metrics) \
    (yi_src= src->pixels + metrics->sy * src->pitch + (metrics->sx*src->depth/8))


#define yi_init(src, dst, metrics) \
    yi_init_dst(dst, metrics), yi_init_dst_end(dst, metrics),\
    yi_init_src(src, metrics)

#define yi_hasnext()    (yi_dst < yi_dst_end)
#define yi_next()       (yi_src += src->pitch, yi_dst += dst->pitch)

#define each_row(src, dst, metrics) \
    for (yi_init(src, dst, metrics); yi_hasnext(); yi_next())



#define xi_init(T, metrics) \
    xi_dst = (unsigned short*)yi_dst, \
    xi_dst_end = xi_dst + metrics->w, \
    xi_src = (const T*)yi_src, \
    px = *xi_src

#define xi_hasnext()    (xi_dst < xi_dst_end)
#define xi_next()       (px = *(++xi_src), ++xi_dst)


#define each_pixel(T, metrics) \
    for (xi_init(T, metrics); xi_hasnext(); xi_next())

/**
 * Odd version iterator, which is for the bits per pixel < 8 formats.
 */

#define def_x_iterator_odd(T, metrics) \
    def_x_iterator(T, metrics); \
    int x

#define yi_init_odd(src, dst, metrics) \
    yi_init_dst(dst, metrics), yi_init_dst_end(dst, metrics),\
    (yi_src= src->memory + metrics->sy * src->pitch) /* just seek line begin (x = 0) */


#define each_row_odd(src, dst, metrics) \
    for (yi_init_odd(src, dst, metrics); yi_hasnext(); yi_next())


#define xi_init_odd(T, metrics, BITS)   \
    (xi_init(T, metrics), x = metrics->sx, px = ___nth_## BITS ##_bits(x, xi_src))

#define xi_next_odd(BITS)             \
    (++xi_dst, ++x, px = ___nth_## BITS ##_bits(x, xi_src) )

#define each_pixel_odd(T, metrics, BITS) \
    for (xi_init_odd(T, metrics, BITS); xi_hasnext(); xi_next_odd(BITS))


typedef void (*blit_fn)(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics);

static void _blit_565_no_extra_alpha(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics)
{
    def_y_iterator();
    size_t sz;
    (void)(alpha); // where alpha == 0xFF

    if (src->w == dst->w && metrics->w == src->w // full screen src
        && metrics->sx == 0 && metrics->dx == 0) // place at (0, y)
    {
        // copy all lines at once!
        yi_init_src(src, metrics);
        yi_init_dst(dst, metrics);
        sz = dst->pitch * metrics->h;
        memcpy(yi_dst, yi_src, sz);
    }
    else
    {
        // copy line by line
        sz = metrics->w*2; // 2 bytes per pixel.
        each_row(src, dst, metrics){
            memcpy(yi_dst, yi_src, sz); // GCC's memcpy is very much optimized
        }
    }
}

static void _blit_565(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics)
{
    def_y_iterator();
    def_x_iterator(unsigned short, metrics);

    unsigned short dpx;
    int a, dindex;
    unsigned r, g, b;

    a = alpha >> 4;
    dindex = 0xF - a;

    each_row(src, dst, metrics) { each_pixel(unsigned short, metrics) {
        r = _5x4[RGB565_R5bits(px)][a];
        g = _6x4[RGB565_G6bits(px)][a];
        b = _5x4[RGB565_B5bits(px)][a];

        dpx = *xi_dst;
        r += _5x4[RGB565_R5bits(dpx)][dindex];
        g += _6x4[RGB565_G6bits(dpx)][dindex];
        b += _5x4[RGB565_B5bits(dpx)][dindex];

        // can we check that  tint.a < max(tint.r, tint.g, tint.b) to ensure the follow will not happen?
        // saturation_check(r, g, b);

        *xi_dst = (unsigned short)((r << 11) | (g << 5) | b);
    }}

}




static void _blit_565kc_no_extra_alpha(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics)
{

    def_y_iterator();
    def_x_iterator(unsigned short, metrics);
    (void)(alpha);

    each_row(src, dst, metrics){each_pixel(unsigned short, metrics)
    {
       if (px == 0xF81E) // not key color
           continue;
       *xi_dst = px;
    }}
}



static void _blit_565kc(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics)
{
    def_y_iterator();
    def_x_iterator(unsigned short, metrics);

    unsigned short dpx;
    int a, dindex;
    unsigned r, g, b;

    a = alpha >> 4;
    dindex = 0xF- a;

    each_row(src, dst, metrics) { each_pixel(unsigned short, metrics) {
        if (px == 0xF81E) // not key color
            continue;

        r = RGB565_R5bits(px);
        g = RGB565_G6bits(px);
        b = RGB565_B5bits(px);

        dpx = *xi_dst;
        r += _5x4[RGB565_R5bits(dpx)][dindex];
        g += _6x4[RGB565_G6bits(dpx)][dindex];
        b += _5x4[RGB565_B5bits(dpx)][dindex];

        *xi_dst = (unsigned short)((r << 11) | (g << 5) | b);
    }}

}


static void _blit_5551_no_extra_alpha(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics)
{
    def_y_iterator();
    def_x_iterator(unsigned short, metrics);

    (void)(alpha);

    each_row(src, dst, metrics) { each_pixel(unsigned short, metrics) {
        if (RGBA5551_A(px))
            *xi_dst = (px & 0xF800) | expand_5bits_to_RGB565_G6[RGBA5551_G5bits(px)] | RGBA5551_B5(px);
    }}
    
}


static void _blit_5551(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics)
{
    def_y_iterator();
    def_x_iterator(unsigned short, metrics);

    unsigned short dpx;
    int dindex;
    unsigned r, g, b, a;

    a = alpha >> 4;
    dindex = 0xF - a;

    each_row(src, dst, metrics) { each_pixel(unsigned short, metrics) {
        if (RGBA5551_A(px))
        {
            r = RGBA5551_R5bits(px);
            g = RGBA5551_G6bits(px);
            b = RGBA5551_B5(px);

            dpx = *xi_dst;
            r += _5x4[RGB565_R5bits(dpx)][dindex];
            g += _6x4[RGB565_G6bits(dpx)][dindex];
            b += _5x4[RGB565_B5bits(dpx)][dindex];

            *xi_dst = (unsigned short)((r << 11) | (g << 5) | b);
        }
    }}
}

static void _blit_4444_no_extra_alpha(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics)
{
    def_y_iterator();
    def_x_iterator(unsigned short, metrics);

    unsigned short dpx;
    int dindex;
    unsigned r, g, b, a;

    (void)(alpha);
    each_row(src, dst, metrics) { each_pixel(unsigned short, metrics) {
        a = RGBA4444_A(px);
        if (a) {
            if (a == 0xF)
                *xi_dst = expand_4_to_R5[px>>12]|expand_4_to_G6[(px>>8)&0x0F]|expand_4_to_B5[(px>>4)&0x0F];
            else
            {
                dindex = 0xF - a;
                dpx = *xi_dst;
                // SRC(r,g,b,a) is a premult alpha colors.
                r = RGBA4444_R5(px) + _5x4[RGB565_R5bits(dpx)][dindex];
                g = RGBA4444_G6(px) + _6x4[RGB565_G6bits(dpx)][dindex];
                b = RGBA4444_B5(px) + _5x4[RGB565_B5bits(dpx)][dindex];

                *xi_dst = (unsigned short)((r << 11) | (g << 5) | b);
            }
        }
    }}
}

static void _blit_4444(const _EGGImage *src, _EGGImage *dst, unsigned char alpha, const EGGMetrics* metrics)
{
    def_y_iterator();
    def_x_iterator(unsigned short, metrics);

    unsigned short dpx;
    int dindex;
    unsigned r, g, b, a;

    const int a5 = alpha >> 3;
    const int a6 = alpha >> 2;
    const int a4 = alpha >> 4;

    each_row(src, dst, metrics) { each_pixel(unsigned short, metrics) {
        a = _4x4[RGBA4444_A(px)][a4];

        if (a==0)
            continue;

        r = _5x4[a5][ px>>12    ];
        g = _6x4[a6][(px>>8)&0xF];
        b = _5x4[a5][(px>>4)&0xF];

        if (a < ALPHA_MAX)
        {
            dindex = ALPHA_MAX - a;
            dpx = *xi_dst;
            // SRC(r,g,b,a) is a premult alpha colors.
            r += _5x4[RGB565_R5bits(dpx)][dindex];
            g += _6x4[RGB565_G6bits(dpx)][dindex];
            b += _5x4[RGB565_B5bits(dpx)][dindex];
        }

        *xi_dst = (unsigned short)((r << 11) | (g << 5) | b);
    }}
}

static EGGContext context = {0};


void eggInit(void* buffers[2], EGGImageFormat format, EGGuint w, EGGuint h, EGGint pitch)
{
	if (format != EGG_RGB565)
	{
		context.errorcode = EGG_ILLEGAL_ARGUMENT_ERROR;
		return;
	}

	context.surface.format = format;
	context.surface.w = w;
	context.surface.h = h;
	context.surface.pitch = pitch;
	context.surface.depth = 16;
	context.surface.pixels = buffers[0];
	context.buffers[0] = buffers[0];
	context.buffers[1] = buffers[1];
	context.current = 0;


	// defaults
	context.alpha = 255;
	context.colorkey = EGG_FALSE;
	context.colorkey_value = 0xFF00FF;
}

void eggExit()
{
	memset(&context, 0, sizeof(context));
}

EGGErrorCode eggGetError()
{
	EGGErrorCode code = context.errorcode;
	context.errorcode = EGG_OK;
	return code;
}

void eggSeti(EGGParamType param, int value)
{
	switch (param)
	{
	case EGG_ALPHA:
		if (value < 0 || value > 255)
			context.errorcode = EGG_ILLEGAL_ARGUMENT_ERROR;
		context.alpha = value;
		break;
	case EGG_COLORKEY:
		context.colorkey = value;
		break;
	case EGG_COLORKEY_VALUE:
		context.colorkey_value = value;
		break;
	default:
		context.errorcode = EGG_ILLEGAL_ARGUMENT_ERROR;
		break;
	}
}

int eggGeti(EGGParamType param)
{
	switch (param)
	{
	case EGG_ALPHA:		return context.alpha;
	case EGG_COLORKEY:	return context.colorkey;
	case EGG_COLORKEY_VALUE: return context.colorkey_value;
	default:
		context.errorcode = EGG_ILLEGAL_ARGUMENT_ERROR;
		return 0;// undefined!
	}
}


static const blit_fn blit_functions[2/*alpha*/][2/*keycolor*/][3/*format*/] = {
	{
		{
			_blit_565_no_extra_alpha,
			_blit_5551_no_extra_alpha,
			_blit_4444_no_extra_alpha
		},
		{
			_blit_565kc_no_extra_alpha,
			_blit_5551_no_extra_alpha,
			_blit_4444_no_extra_alpha,
		},
	},
	{
		{
			_blit_565,
			_blit_5551,
			_blit_4444,
		},

		{
			_blit_565kc,
			_blit_5551,
			_blit_4444,
		}
	}
};


const EGGMetrics* get_final_metrics(EGGMetrics* metrics, const int limtX, const int limtY)
{
    int dxend, dyend;

#define check_axis(START, END, LENGTH, LIMIT) \
    if (START > LIMIT) return NULL; \
    END = START + LENGTH; if (END < 0 ) return NULL
        
    check_axis(metrics->dx, dxend, metrics->w, limtX);
    check_axis(metrics->dy, dyend, metrics->h, limtY);

#undef check_axis

/**
    SRC:
         START
          |
     -----+------------------------+------------->
          \<------- LENGTH ------->\
           \                        \
    DEST:   \                        \
             \                        \
           DSTART  0                 DEND
    ----------+----+---------------+---+--------->
                   |<--- LIMIT --->|
 */
#define clipaxis(START, LENGTH, DSTART, DEND, LIMIT)    \
        if (DSTART < 0) { START -= DSTART; LENGTH += DSTART; DSTART = 0; }  \
        if (DEND > LIMIT) LENGTH -= DEND - LIMIT

    clipaxis(metrics->sx, metrics->w, metrics->dx, dxend, limtX);
    clipaxis(metrics->sy, metrics->h, metrics->dy, dyend, limtY);

#undef clipaxis

    if (metrics->w <= 0 || metrics->h <= 0)
        return NULL;

    return metrics;
}


blit_fn get_blit_function(EGGbyte alpha, EGGboolean colorkey, EGGImageFormat fmt)
{
	int ia = alpha != 255;
	int ikc = colorkey != 0;
	int ifmt = fmt - EGG_RGB565;
	if (ifmt >= sizeof(blit_functions[0][0])/ sizeof(blit_functions[0][0][0]))
		return NULL;
	return blit_functions[ia][ikc][ifmt];
}


void eggWritePixels(const void * data, EGGImageFormat fmt, EGGint dx, EGGint dy, EGGint width,  EGGint height, EGGint pitch)
{
	_EGGImage img;
	union EGGImageCast cast = {&img};

	img.format = fmt;
	img.w = width;
	img.h = height;
	img.pitch = pitch;
	img.depth = 16;
	img.pixels = (EGGubyte*)data;

	eggDrawImage(cast.handle, dx, dy);
}

EGG_API void eggFlush()
{
	context.current = !context.current;
	context.surface.pixels = context.buffers[context.current];
}

EGG_API void* eggDisplayBuffer()
{
	return context.buffers[!context.current];
}



EGGImage eggCreateImage(EGGImageFormat fmt, EGGint width, EGGint height)
{
	EGGint pitch = width * 16/ 8;
	union EGGImageCast cast;
	cast.image = malloc(sizeof(*cast.image) + pitch * height);
	if (!cast.image)
		return EGG_INVALID_HANDLE;

	cast.image->sig[0] = 'I';
	cast.image->sig[1] = 'M';
	cast.image->sig[2] = 'G';
	cast.image->sig[3] = '\0';

	cast.image->depth = 16; // we currently only support 16 bit images
	cast.image->format = fmt;
	cast.image->w = width;
	cast.image->h = height;
	cast.image->pitch = pitch;
	cast.image->pixels = (EGGubyte*)(cast.image)+sizeof(*cast.image);
	
	return cast.handle;
}

static void copy16(EGGImage image, const void * data, EGGint pitch, EGGint x, EGGint y, EGGint width, EGGint height)
{
	int i;
	union EGGImageCast cast;
	const EGGubyte* src;
	EGGubyte* dst;

	cast.handle = image;

	dst = cast.image->pixels + y*cast.image->pitch + x*2;
	src = (const EGGubyte*)(data);

	for (i = 0; i < height; i++)
	{
		memcpy(dst, src, pitch);
		dst += cast.image->pitch;
		src += pitch;
	}
}



void eggImageSubData(EGGImage image, const void * data, EGGint pitch, EGGint x, EGGint y, EGGint width, EGGint height)
{
	union EGGImageCast cast;
	_EGGImage* img = (_EGGImage*)(image);
	int left, bottom;
	cast.handle = image;

	if (x >= (EGGint)cast.image->w || y >= (EGGint)cast.image->h)
		return; // on overlaps

	left = x + width;
	bottom = y + height;
	if (left < 0 || bottom < 0)
		return; // on overlaps

	if (left > (EGGint)cast.image->w)
		width -= left - cast.image->w;

	if (bottom > (EGGint)cast.image->h)
		height -= bottom - cast.image->h;

	if (x < 0)
	{
		width += x;
		x = 0;
	}
	if (y < 0)
	{
		height += y;
		y = 0;
	}

	copy16(image, data, pitch, x, y, width, height);
}

void eggDestroyImage( EGGImage image )
{
	union EGGImageCast cast;

	if (image == EGG_INVALID_HANDLE)
		return;
	cast.handle = image;

	if (strcmp(cast.image->sig, "IMG") != 0)
		return;

	free(cast.image);
}




EGG_API void eggDrawImage( EGGImage image, EGGint dx, EGGint dy )
{
	union EGGImageCast cast;
	EGGMetrics metrics;
	blit_fn blit;	

	cast.handle = image;
	
	if (context.surface.pixels == NULL)
	{
		context.errorcode = EGG_NO_CONTEXT_ERROR;
		return;
	}

	if (context.alpha == 0) // nothing to draw due to global alpha is zero
		return;

	blit = get_blit_function(context.alpha, context.colorkey, cast.image->format);
	if (!blit)
	{
		context.errorcode = EGG_UNSUPPORTED_IMAGE_FORMAT_ERROR;
		return;
	}

	metrics.dx = dx;
	metrics.dy = dy;
	metrics.sx = 0;
	metrics.sy = 0;
	metrics.w = cast.image->w;
	metrics.h = cast.image->h;

	if (!get_final_metrics(&metrics, context.surface.w, context.surface.h))
		return;

	blit(cast.image, &context.surface, context.alpha, &metrics);
}
