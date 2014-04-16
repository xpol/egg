#ifndef __EGG_H__
#define __EGG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EGG_API			extern

typedef float          EGGfloat;
typedef signed char    EGGbyte;
typedef unsigned char  EGGubyte;
typedef signed short   EGGshort;
typedef signed int     EGGint;
typedef unsigned int   EGGuint;

#ifndef EGG_MAX_ENUM
#define EGG_MAX_ENUM 0x7FFFFFFF
#endif


typedef enum EGGboolean{
	EGG_FALSE               = 0,
	VG_TRUE                = 1,

	EGG_BOOLEAN_FORCE_SIZE  = EGG_MAX_ENUM
} EGGboolean;

typedef enum EGGErrorCode{
  EGG_OK                                 = 0,
  EGG_ILLEGAL_ARGUMENT_ERROR                   = 0x1001L,
  EGG_OUT_OF_MEMORY_ERROR                      = 0x1002L,
  EGG_UNSUPPORTED_IMAGE_FORMAT_ERROR           = 0x1004L,
  EGG_NO_CONTEXT_ERROR                         = 0x1007L,

  EGG_ERRORCODE_FORCE_SIZE  = EGG_MAX_ENUM
} EGGErrorCode;



typedef enum EGGParamType
{
	EGG_ALPHA = 0x1010,				// integer, [0..255]: global alapha
	EGG_COLORKEY = 0x1011,			// boolean, [0..1]: enable or disable colorkey
	EGG_COLORKEY_VALUE = 0x1012,	// integer, XRGB8888 format color information

	EGG_PARAMTYPE_FORCE_SIZE  = EGG_MAX_ENUM
} EGGParamType;

typedef enum EGGImageFormat
{
	EGG_RGB565		= 0x2001,
	EGG_RGBA5551	= 0x2002,
	EGG_RGBA4444	= 0x2003,
	
	EGG_IMAGEFORMAT_FORCE_SIZE  = EGG_MAX_ENUM
} EGGImageFormat;


// Init double buffer for drawing
EGG_API void eggInit(void* buffers[2], EGGImageFormat format, EGGuint width, EGGuint height, EGGint pitch);

// Cleanup when exit system...
EGG_API void eggExit();

// Get last error code.
EGG_API EGGErrorCode eggGetError();

// Set params
EGG_API void eggSeti(EGGParamType param, EGGint value);

// Get params
EGG_API EGGint eggGeti(EGGParamType param);

// Draw image pixels
EGG_API void eggWritePixels(const void * data, EGGImageFormat fmt, EGGint dx, EGGint dy, EGGint width,  EGGint height, EGGint pitch);

// Finish drawing by swap buffers
EGG_API void eggFlush();

// Get current buffer for display, that is the buffer contains data that ready to display to screen.
EGG_API void* eggDisplayBuffer();

#ifdef __cplusplus
}
#endif

#endif // __EGG_H__
