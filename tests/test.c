#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define strcasecmp _stricmp
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "egg.h"


static size_t FILEread(void* fp, void* buffer, size_t size)
{
	FILE* f = (FILE*)(fp);
	return fread(buffer, 1, size, f);
}


static EGGboolean FILEseek(void* fp, long int off, int origin)
{
	FILE* f = (FILE*)(fp);
	return fseek(f, off, origin) ==0;
}

EGGImage loadPVR(const char* filename)
{
	FILE* f = fopen(filename, "rb");
	EGGImage img = eggLoadPVRImage(f, FILEread, FILEseek);
	fclose(f);
	return img;
}




EGGboolean verifyImage(EGGImage image, char* expr)
{
	int w = -1;
	int h = -1;

	char* tok = strtok(expr, ":");
	if (!tok)
		return EGG_FALSE;
	// Verify format...
	switch (eggGetParameteri(image, EGG_IMAGE_FORMAT))
	{
	case EGG_RGB565:
		if (strcasecmp(tok, "RGB565") != 0)
			return EGG_FALSE;
		break;
	case EGG_RGBA4444:
		if (strcasecmp(tok, "RGBA4444") != 0)
			return EGG_FALSE;
		break;
	case EGG_RGBA5551:
		if (strcasecmp(tok, "RGBA5551") != 0)
			return EGG_FALSE;
		break;
	default:return EGG_FALSE;
	}
	
	tok = strtok(NULL, "x");
	if (!tok)
		return EGG_FALSE;
	w = atoi(tok);

	tok = strtok(NULL, "\n");
	if (!tok)
		return EGG_FALSE;
	h = atoi(tok);

	if (eggGetParameteri(image, EGG_IMAGE_WIDTH) != w)
		return EGG_FALSE;
	
	if (eggGetParameteri(image, EGG_IMAGE_HEIGHT) != h)
		return EGG_FALSE;

	return EGG_TRUE;
}

EGGuint pack(EGGuint argb, EGGImageFormat format)
{
	EGGuint packed = 0;

	switch(format)
	{
	case EGG_RGB565:
		packed  = (argb & 0x000000f8) >> 3; // b
		packed |= (argb & 0x0000fc00) >> 5; // g
		packed |= (argb & 0x00f80000) >> 8; // r
		return packed;
	case EGG_RGBA4444:
		packed  = (argb & 0x000000f0);      // b
		packed |= (argb & 0x0000f000) >> 4; // g
		packed |= (argb & 0x00f00000) >> 8; // r
		packed |=  argb >> 28;              // a
		return packed;
	case EGG_RGBA5551:
		packed  = (argb & 0x000000f8) >> 2;
		packed |= (argb & 0x0000f800) >> 5;
		packed |= (argb & 0x00f80000) >> 8;
		packed |= (argb & 0x80000000) >>31;
		return packed;
	default:
		return 0;
	}
}

EGGboolean verifyColor(EGGImage image, char* expr)
{
	int x = -1;
	int y = -1;
	EGGuint color, packed;
	unsigned short actually;
	char* endptr = NULL;
	char* tok = strtok(expr, "[, ");
	if (!tok)
		return EGG_FALSE;
	x = strtol(tok, &endptr, 10);
	if (*endptr != '\0')
		return EGG_FALSE;

	tok = strtok(NULL, ", ");
	if (!tok)
		return EGG_FALSE;
	y = strtol(tok, &endptr, 10);
	if (*endptr != '\0')
		return EGG_FALSE;

	tok = strtok(NULL, "] ");
	if (!tok)
		return EGG_FALSE;
	color = strtoul(tok, &endptr, 16);
	if (*endptr != '\0')
		return EGG_FALSE;


	if (   x < 0 
		|| y < 0
		|| x >= eggGetParameteri(image, EGG_IMAGE_WIDTH)
		|| y >= eggGetParameteri(image, EGG_IMAGE_HEIGHT))
		return EGG_FALSE;
	packed = pack(color, eggGetParameteri(image, EGG_IMAGE_FORMAT));
	actually = (unsigned short)~packed; // make sure different....
	eggGetImageSubData(image, &actually, 2, x, y, 1, 1);

	return packed == (unsigned)actually;
}

// xxx.pvr RGB565:wxh (x,y,0xRRGGBBAA)
int main(int argc, char *argv[])
{
	EGGImage image;
	int i;
	if (argc < 2)
		return EXIT_FAILURE;

	image = loadPVR(argv[1]);
	if (image == EGG_INVALID_HANDLE)
		return EXIT_FAILURE;

	if (argc > 2 && !verifyImage(image, argv[2]))
		return EXIT_FAILURE;
	
	for (i = 3; i < argc; i++)
	{
		if (!verifyColor(image, argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
