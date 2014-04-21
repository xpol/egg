#include "egg.h"


#if defined(_MSC_VER)
#if (_MSC_VER >= 1700)
#include <stdint.h>
#else
typedef   signed __int8		int8_t;
typedef   signed __int16    int16_t;
typedef   signed __int32    int32_t;
typedef   signed __int64    int64_t;
typedef unsigned __int8		uint8_t;
typedef unsigned __int16	uint16_t;
typedef unsigned __int32	uint32_t;
typedef unsigned __int64    uint64_t;

#endif
#endif

#include <stdlib.h>
#include "internal.h"

	/* supported channel type formats */
	//kPVR3TexturePixelFormat_BGRA_8888 = 0x0808080861726762ULL
	//kPVR3TexturePixelFormat_RGBA_8888 = 0x0808080861626772ULL
#define kPVR3TexturePixelFormat_RGBA_4444	0x0404040461626772ULL
#define kPVR3TexturePixelFormat_RGBA_5551	0x0105050561626772ULL
#define kPVR3TexturePixelFormat_RGB_565		0x0005060500626772ULL
	//kPVR3TexturePixelFormat_RGB_888 = 0x0008080800626772ULL
	//kPVR3TexturePixelFormat_A_8 = 0x0000000800000061ULL
	//kPVR3TexturePixelFormat_L_8 = 0x000000080000006cULL
	//kPVR3TexturePixelFormat_LA_88 = 0x000008080000616cULL

typedef struct PVR3Format{
	uint64_t pvr;
	EGGint egg;
	int depth;
	EGGboolean require_pre_multiplied;
} PVR3Format;

static const PVR3Format PVRFORMATS[] = {
	{kPVR3TexturePixelFormat_RGBA_4444, EGG_RGBA4444, 16, EGG_TRUE},
	{kPVR3TexturePixelFormat_RGBA_5551, EGG_RGBA5551, 16, EGG_FALSE},
	{kPVR3TexturePixelFormat_RGB_565, EGG_RGB565, 16, EGG_FALSE},
};

typedef struct {
	uint32_t version;
	uint32_t flags;
	uint64_t pixelFormat;
	uint32_t colorSpace;
	uint32_t channelType;
	uint32_t height;
	uint32_t width;
	uint32_t depth;
	uint32_t numberOfSurfaces;
	uint32_t numberOfFaces;
	uint32_t numberOfMipmaps;
	uint32_t metadataLength;
} PVR3Header;

/*
static uint32_t __bswap_32( uint32_t val )
{
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
	return (val << 16) | (val >> 16);
}

static uint64_t __bswap_64( uint64_t val )
{
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
	return (val << 32) | (val >> 32);
}*/

const PVR3Format* getFormat(uint64_t pvrfmt)
{
	int i;
	for (i = 0; i < sizeof(PVRFORMATS)/sizeof(PVRFORMATS[0]); i++)
	{
		if (PVRFORMATS[i].pvr == pvrfmt)
			return &PVRFORMATS[i];
	}
	return NULL;
}

EGGImage eggLoadPVRImage(void* io, EGGReader readfn, EGGSeek seekfn)
{
	PVR3Header header;
	EGGboolean pre_multiplied;
	EGGImage img;
	
	const PVR3Format* fmt;
	unsigned y;
	size_t pitch;
	EGGubyte* linebuffer;

	readfn(io, &header, sizeof(header));

	if (header.version != 0x50565203)  // header.version != 0x03525650 not supported yet.
		return EGG_INVALID_HANDLE;

	pre_multiplied = (header.flags == 0x02);

	fmt = getFormat(header.pixelFormat);
	if (!fmt)
		return EGG_INVALID_HANDLE;

	if (!pre_multiplied && fmt->require_pre_multiplied)
		return EGG_INVALID_HANDLE; // We don't want convert at run time.

	seekfn(io, EGG_SEEK_CUR, header.metadataLength); // We currently don't care any meta data.

	img = eggCreateImage(fmt->egg, header.width, header.height);
	if (img == EGG_INVALID_HANDLE)
		return EGG_INVALID_HANDLE;

	// we only process top level mipmap.
	pitch = header.width*fmt->depth/8;
	linebuffer = malloc(pitch);
	for (y = 0; y < header.height; y++)
	{
		readfn(io, linebuffer, pitch);
		eggImageSubData(img, linebuffer, pitch, 0, y, header.width, 1);
	}

	return img;
}
