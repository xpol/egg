#include "egg.h"

typedef struct _EGGImage {
	char sig[4];
	EGGint format;
	EGGuint depth;
	EGGuint w;
	EGGuint h;
	EGGint pitch;
	EGGubyte* pixels;
} _EGGImage;

union EGGImageCast {
	_EGGImage* image;
	EGGImage handle;
};