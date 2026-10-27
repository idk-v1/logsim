#pragma once

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


#ifdef __cplusplus
#define PIXEL_FN extern "C" 
#else
#define PIXEL_FN
#endif

#ifdef __cplusplus
extern "C" { extern const SDL_PixelFormatDetails* pixFmt; }
struct RGB { Uint8 r, g, b, x; };
#else
extern const SDL_PixelFormatDetails* pixFmt;
typedef struct RGB { Uint8 r, g, b, x; } RGB;
#endif

// Call this first, just once, this saves it globally
static inline void setPixelFormat(SDL_Surface* surface)
{
	pixFmt = SDL_GetPixelFormatDetails(surface->format);
}

// Converts from rgb to the system pixel format
static inline Uint32 rgb(Uint8 r, Uint8 g, Uint8 b)
{
	return SDL_MapRGB(pixFmt, NULL, r, g, b);
}

// Converts from the system pixel format to rgb
static inline RGB unrgb(Uint32 color)
{
	RGB rgb = { 0 };
	SDL_GetRGB(color, pixFmt, NULL, &rgb.r, &rgb.g, &rgb.b);
	return rgb;
}

// Not recommended for use, sets pixel without bounds checking
inline void setPixelUC(SDL_Surface* surface, Uint32 x, Uint32 y, Uint32 color)
{
	((Uint32*)surface->pixels)[x + y * surface->w] = color;
}

// Not recomented for use, used as backup for setline speedup if no SSE
inline void setPixel2UC(SDL_Surface* surface, Uint32 x, Uint32 y, Uint64 color)
{
	*((Uint64*)&((Uint32*)surface->pixels)[x + y * surface->w]) = color;
}

// Sets pixel with bounds checking
// Use this first for testing, then if all out of bounds cases are covered, use unchecked
inline void setPixel(SDL_Surface* surface, Uint32 x, Uint32 y, Uint32 color)
{
	if (x < (Uint32)surface->w && y < (Uint32)surface->h)
		setPixelUC(surface, x, y, color);
	//else
		//printf("Pixel out of bounds (%d %d)\n", x - (Sint32)surface->w, y - (Sint32)surface->h);
}

inline Uint32 getPixelUC(SDL_Surface* surface, Uint32 x, Uint32 y)
{
	return ((Uint32*)surface->pixels)[x + y * surface->w];
}

inline Uint32 getPixel(SDL_Surface* surface, Uint32 x, Uint32 y)
{
	if (x < (Uint32)surface->w && y < (Uint32)surface->h)
		return getPixelUC(surface, x, y);
	//else
		//printf("Pixel out of bounds (%d %d)\n", x - (Sint32)surface->w, y - (Sint32)surface->h);
}

#ifdef __GNUC__
#define DECLARE_ALIGNED(t, v, a) t __attribute__((aligned(a))) v
#elif defined(_MSC_VER)
#define DECLARE_ALIGNED(t, v, a) __declspec(align(a)) t v
#else
#define DECLARE_ALIGNED(t, v, a) t v
#endif

#ifdef USE_SSE
#include <immintrin.h>

#if defined(_MSC_VER) && !defined(__clang__)
#define SSE_BEGIN \
    __m128 c128; \
    c128.m128_u32[0] = color; \
    c128.m128_u32[1] = color; \
    c128.m128_u32[2] = color; \
    c128.m128_u32[3] = color;
#else
#define SSE_BEGIN \
    __m128 c128; \
    DECLARE_ALIGNED(Uint32, cccc[4], 16); \
    cccc[0] = color; \
    cccc[1] = color; \
    cccc[2] = color; \
    cccc[3] = color; \
    c128 = *(__m128 *)cccc;
#endif
#else
#define SSE_BEGIN
#endif

// Unrolled loops, I'm sorry
#ifdef USE_SSE
inline static void setHLine64(SDL_Surface* surface, Uint32 x, Uint32 y, __m128 color)
{
	// extreme alignment needed to not crash (16 bytes)
	// https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm_stream_ps&ig_expand=6562
	// make sure caller passes aligned values
	Uint32* pixels = ((Uint32*)surface->pixels + (x + y * surface->w));
	_mm_stream_ps((float*)(pixels + 0), color);
	_mm_stream_ps((float*)(pixels + 4), color);
	_mm_stream_ps((float*)(pixels + 8), color);
	_mm_stream_ps((float*)(pixels + 12), color);
	_mm_stream_ps((float*)(pixels + 16), color);
	_mm_stream_ps((float*)(pixels + 20), color);
	_mm_stream_ps((float*)(pixels + 24), color);
	_mm_stream_ps((float*)(pixels + 28), color);
	_mm_stream_ps((float*)(pixels + 32), color);
	_mm_stream_ps((float*)(pixels + 36), color);
	_mm_stream_ps((float*)(pixels + 40), color);
	_mm_stream_ps((float*)(pixels + 44), color);
	_mm_stream_ps((float*)(pixels + 48), color);
	_mm_stream_ps((float*)(pixels + 52), color);
	_mm_stream_ps((float*)(pixels + 56), color);
	_mm_stream_ps((float*)(pixels + 60), color);
}

inline static void setHLine32(SDL_Surface* surface, Uint32 x, Uint32 y, __m128 color)
{
	Uint32* pixels = &(((Uint32*)surface->pixels)[x + y * surface->w]);
	_mm_stream_ps((float*)(pixels + 0), color);
	_mm_stream_ps((float*)(pixels + 4), color);
	_mm_stream_ps((float*)(pixels + 8), color);
	_mm_stream_ps((float*)(pixels + 12), color);
	_mm_stream_ps((float*)(pixels + 16), color);
	_mm_stream_ps((float*)(pixels + 20), color);
	_mm_stream_ps((float*)(pixels + 24), color);
	_mm_stream_ps((float*)(pixels + 28), color);
}

inline static void setHLine16(SDL_Surface* surface, Uint32 x, Uint32 y, __m128 color)
{
	Uint32* pixels = &(((Uint32*)surface->pixels)[x + y * surface->w]);
	_mm_stream_ps((float*)(pixels + 0), color);
	_mm_stream_ps((float*)(pixels + 4), color);
	_mm_stream_ps((float*)(pixels + 8), color);
	_mm_stream_ps((float*)(pixels + 12), color);
}

inline static void setHLine8(SDL_Surface* surface, Uint32 x, Uint32 y, __m128 color)
{
	Uint32* pixels = &(((Uint32*)surface->pixels)[x + y * surface->w]);
	_mm_stream_ps((float*)(pixels + 0), color);
	_mm_stream_ps((float*)(pixels + 4), color);
}
#else
inline static void setHLine64(SDL_Surface* surface, Uint32 x, Uint32 y, Uint64 color)
{
	setPixel2UC(surface, x + 0, y, color);
	setPixel2UC(surface, x + 2, y, color);
	setPixel2UC(surface, x + 4, y, color);
	setPixel2UC(surface, x + 6, y, color);
	setPixel2UC(surface, x + 8, y, color);
	setPixel2UC(surface, x + 10, y, color);
	setPixel2UC(surface, x + 12, y, color);
	setPixel2UC(surface, x + 14, y, color);
	setPixel2UC(surface, x + 16, y, color);
	setPixel2UC(surface, x + 18, y, color);
	setPixel2UC(surface, x + 20, y, color);
	setPixel2UC(surface, x + 22, y, color);
	setPixel2UC(surface, x + 24, y, color);
	setPixel2UC(surface, x + 26, y, color);
	setPixel2UC(surface, x + 28, y, color);
	setPixel2UC(surface, x + 30, y, color);
	setPixel2UC(surface, x + 32, y, color);
	setPixel2UC(surface, x + 34, y, color);
	setPixel2UC(surface, x + 36, y, color);
	setPixel2UC(surface, x + 38, y, color);
	setPixel2UC(surface, x + 40, y, color);
	setPixel2UC(surface, x + 42, y, color);
	setPixel2UC(surface, x + 44, y, color);
	setPixel2UC(surface, x + 46, y, color);
	setPixel2UC(surface, x + 48, y, color);
	setPixel2UC(surface, x + 50, y, color);
	setPixel2UC(surface, x + 52, y, color);
	setPixel2UC(surface, x + 54, y, color);
	setPixel2UC(surface, x + 56, y, color);
	setPixel2UC(surface, x + 58, y, color);
	setPixel2UC(surface, x + 60, y, color);
	setPixel2UC(surface, x + 62, y, color);
}

inline static void setHLine32(SDL_Surface* surface, Uint32 x, Uint32 y, Uint64 color)
{
	setPixel2UC(surface, x + 0, y, color);
	setPixel2UC(surface, x + 2, y, color);
	setPixel2UC(surface, x + 4, y, color);
	setPixel2UC(surface, x + 6, y, color);
	setPixel2UC(surface, x + 8, y, color);
	setPixel2UC(surface, x + 10, y, color);
	setPixel2UC(surface, x + 12, y, color);
	setPixel2UC(surface, x + 14, y, color);
	setPixel2UC(surface, x + 16, y, color);
	setPixel2UC(surface, x + 18, y, color);
	setPixel2UC(surface, x + 20, y, color);
	setPixel2UC(surface, x + 22, y, color);
	setPixel2UC(surface, x + 24, y, color);
	setPixel2UC(surface, x + 26, y, color);
	setPixel2UC(surface, x + 28, y, color);
	setPixel2UC(surface, x + 30, y, color);
}

inline static void setHLine16(SDL_Surface* surface, Uint32 x, Uint32 y, Uint64 color)
{
	setPixel2UC(surface, x + 0, y, color);
	setPixel2UC(surface, x + 2, y, color);
	setPixel2UC(surface, x + 4, y, color);
	setPixel2UC(surface, x + 6, y, color);
	setPixel2UC(surface, x + 8, y, color);
	setPixel2UC(surface, x + 10, y, color);
	setPixel2UC(surface, x + 12, y, color);
	setPixel2UC(surface, x + 14, y, color);
}

inline static void setHLine8(SDL_Surface* surface, Uint32 x, Uint32 y, Uint64 color)
{
	setPixel2UC(surface, x + 0, y, color);
	setPixel2UC(surface, x + 2, y, color);
	setPixel2UC(surface, x + 4, y, color);
	setPixel2UC(surface, x + 6, y, color);
}
#endif

// Not recommended for use, used internally to fill a row at a time
// Not bounds checked
inline static void setLine(SDL_Surface* surface, Uint32 x, Uint32 y, Uint32 w, Uint32 color)
{
	Uint64 color64 = color | (Uint64)color << 32;
#ifdef USE_SSE
	SSE_BEGIN;
#endif

	Sint32 align = (4 - ((x + y * surface->w) & 0b11)) & 0b11;
	for (Sint32 i = 0; i < align && i < w; i++)
		setPixelUC(surface, x + i, y, color);

	for (Uint32 ix = align; ix < w;)
	{
		Sint32 width = w - ix;
		if (width >= 64)
		{
#ifdef USE_SSE
			setHLine64(surface, ix + x, y, c128);
#else
			setHLine64(surface, ix + x, y, color64);
#endif
			ix += 64;
		}
		else if (width >= 32)
		{
#ifdef USE_SSE
			setHLine32(surface, ix + x, y, c128);
#else
			setHLine32(surface, ix + x, y, color64);
#endif
			ix += 32;
		}
		else if (width >= 16)
		{
#ifdef USE_SSE
			setHLine16(surface, ix + x, y, c128);
#else
			setHLine16(surface, ix + x, y, color64);
#endif
			ix += 16;
		}
		else if (width >= 8)
		{
#ifdef USE_SSE
			setHLine8(surface, ix + x, y, c128);
#else
			setHLine8(surface, ix + x, y, color64);
#endif
			ix += 8;
		}
		else if (width >= 4)
		{
			setPixel2UC(surface, x + 0 + ix, y, color64);
			setPixel2UC(surface, x + 2 + ix, y, color64);
			ix += 4;
		}
		else if (width >= 2)
		{
			setPixel2UC(surface, x + ix, y, color64);
			ix += 2;
		}
		else
		{
			setPixelUC(surface, x + ix, y, color);
			ix++;
		}
	}
}


// Sets rect of pixels, NOT bounds checked
// Only use if you are sure this will always be in bounds
PIXEL_FN void setRect(SDL_Surface* surface, Uint32 x, Uint32 y,
	Uint32 w, Uint32 h, Uint32 color);

// Sets the entire screen to a color
PIXEL_FN void clearScreen(SDL_Surface* surface, Uint32 color);

// Sets a rect of pixels, will flip if needed and clips
// Negative width or height work as expected
PIXEL_FN void drawRect(SDL_Surface* surface, Sint32 x, Sint32 y,
	Sint32 w, Sint32 h, Uint32 color);

// Sets a rect of pixels, will flip if needed and clips
// Negative width or height work as expected
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawRectA(SDL_Surface* surface, Sint32 x, Sint32 y,
	float alignX, float alignY, Sint32 w, Sint32 h, Uint32 color);

// Draws 4 rects around a rect, not filling in
// Size is thickness of outline, moving inward
PIXEL_FN void drawRectOut(SDL_Surface* surface, Sint32 x, Sint32 y,
	Sint32 w, Sint32 h, Uint32 size, Uint32 color);

// Draws 4 rects around a rect, not filling in
// Size is thickness of outline, moving inward
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawRectOutA(SDL_Surface* surface, Sint32 x, Sint32 y,
	float alignX, float alignY, Sint32 w, Sint32 h, Uint32 size, Uint32 color);


// Draws circle, clipped
// R is radius, due to being ints, circle can't have odd center, r*2 is always even
// Having diameter felt weird
PIXEL_FN void drawCircle(SDL_Surface* surface, Sint32 x, Sint32 y, Uint32 r, Uint32 color);

// Draws circle, clipped
// R is radius, due to being ints, circle can't have odd center, r*2 is always even
// Having diameter felt weird
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawCircleA(SDL_Surface* surface, Sint32 x, Sint32 y,
	float alignX, float alignY, Uint32 r, Uint32 color);


// It's fine to change these
// Default: 1 pixel
extern int letterSpacing; // Spacing between characters
extern int lineSpacing; // Spacing between lines

// Gets the bounds of text if it were to be drawn
// Pass a pointer to as width and height to get the data
PIXEL_FN void getTextSize(const char* text, Uint32 size, Uint32* width, Uint32* height);

// Gets the bounds of text if it were to be drawn
// Pass a pointer to as width and height to get the data
// Printf formatted text
PIXEL_FN void getTextSizeF(Uint32 size, Uint32* width, Uint32* height, const char* fmt, ...);

// Draws a single character
// Used internally, but it's fine to use
// Clipped
PIXEL_FN void drawChar(SDL_Surface* surface, Sint32 x, Sint32 y, Uint32 size,
	Uint32 color, char ch);

// Draws a single character
// Used internally, but it's fine to use
// Clipped
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawCharA(SDL_Surface* surface, Sint32 x, Sint32 y,
	float alignX, float alignY, Uint32 size, Uint32 color, char ch);

// Draws a string
// Clipped
PIXEL_FN void drawText(SDL_Surface* surface, Sint32 x, Sint32 y, Uint32 size,
	Uint32 color, const char* text);

// Draws a string
// Clipped
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawTextA(SDL_Surface* surface, Sint32 x, Sint32 y,
	float alignX, float alignY, Uint32 size, Uint32 color, const char* text);

// Draws a string
// Clipped
// Uses printf format for text
PIXEL_FN void drawTextF(SDL_Surface* surface, Sint32 x, Sint32 y, Sint32 size,
	Uint32 color, const char* fmt, ...);

// Draws a string
// Clipped
// Uses printf format for text
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawTextFA(SDL_Surface* surface, Sint32 x, Sint32 y,
	float alignX, float alignY, Sint32 size, Uint32 color, const char* fmt, ...);

// Draws a string
// Clipped
// Pass a function to be called before the char is drawn
// Pass a void pointer as data to access in the function
// Use: drawing a more accurate background behind text
PIXEL_FN void drawTextFn(SDL_Surface* surface, Sint32 x, Sint32 y, Uint32 size, Uint32 color,
	void(before)(SDL_Surface*, Sint32, Sint32, Sint32, Sint32, char, void*), 
	void* data, const char* text);

// Draws a string
// Clipped
// Pass a function to be called before the char is drawn
// Pass a void pointer as data to access in the function
// Use: drawing a more accurate background behind text
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawTextAFn(SDL_Surface* surface, Sint32 x, Sint32 y,
	float alignX, float alignY, Uint32 size, Uint32 color,
	void(before)(SDL_Surface*, Sint32, Sint32, Sint32, Sint32, char, void*),
	void* data, const char* text);


// Draws a string
// Clipped
// Pass a function to be called before the char is drawn
// Pass a void pointer as data to access in the function
// Use: drawing a more accurate background behind text
// Uses printf format for text
PIXEL_FN void drawTextFFn(SDL_Surface* surface, Sint32 x, Sint32 y, Sint32 size, Uint32 color,
	void(before)(SDL_Surface*, Sint32, Sint32, Sint32, Sint32, char, void*),
	void* data, const char* fmt, ...);

// Draws a string
// Clipped
// Pass a function to be called before the char is drawn
// Pass a void pointer as data to access in the function
// Use: drawing a more accurate background behind text
// Uses printf format for text
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawTextFAFn(SDL_Surface* surface, Sint32 x, Sint32 y,
	float alignX, float alignY, Sint32 size, Uint32 color,
	void(before)(SDL_Surface*, Sint32, Sint32, Sint32, Sint32, char, void*),
	void* data, const char* fmt, ...);


#ifdef __cplusplus
struct Bitmap
{
	Uint32 width, height;
	Uint32* pixels;
};
#else
typedef struct Bitmap
{
	Uint32 width, height;
	Uint32* pixels;
} Bitmap;
#endif

// Loads an image
// Use freeImage() when done
PIXEL_FN Bitmap loadImage(const char* filename);

// Frees image and sets everything to NULL
PIXEL_FN void freeImage(Bitmap* image);

// Draws an image
// dest is where on the screen it will be drawn
// src is the image position to copy from, spritesheet coordinates stuff
// width and height are the max size to be drawn, bounds checking will be done
// Negative width or height will reverse the image in that direction
PIXEL_FN void drawImage(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height);

// Draws an image
// dest is where on the screen it will be drawn
// src is the image position to copy from, spritesheet coordinates stuff
// width and height are the max size to be drawn, bounds checking will be done
// Negative width or height will reverse the image in that direction
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawImageA(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height, float alignX, float alignY);

// Draws an image
// dest is where on the screen it will be drawn
// src is the image position to copy from, spritesheet coordinates stuff
// width and height are the max size to be drawn, bounds checking will be done
// Negative width or height will reverse the image in that direction
// Pass a function to call at every pixel, kind of like a shader
// This will clip the image, so if you need to offset pixels, use drawImageFnNC()
PIXEL_FN void drawImageFn(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height,
	void(pixelFn)(SDL_Surface* surface, Sint32 px, Sint32 py,
		Sint32 tx, Sint32 ty, Sint32 width, Sint32 height, Uint32 color, void* data), void* data);

// Draws an image
// dest is where on the screen it will be drawn
// src is the image position to copy from, spritesheet coordinates stuff
// width and height are the max size to be drawn, bounds checking will be done
// Negative width or height will reverse the image in that direction
// Pass a function to call at every pixel, kind of like a shader
// This will clip the image, so if you need to offset pixels, use drawImageFnNC()
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawImageFnA(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height, float alignX, float alignY,
	void(pixelFn)(SDL_Surface* surface, Sint32 px, Sint32 py,
		Sint32 tx, Sint32 ty, Sint32 width, Sint32 height, Uint32 color, void* data), void* data);

// Draws an image
// dest is where on the screen it will be drawn
// src is the image position to copy from, spritesheet coordinates stuff
// width and height are the max size to be drawn, bounds checking will be done
// Negative width or height will reverse the image in that direction
// Pass a function to call at every pixel, kind of like a shader
// This is NOT bounds checked, make sure 0 <= x < surface->w and 0 <= y surface->h
PIXEL_FN void drawImageFnNC(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height,
	void(pixelFn)(SDL_Surface* surface, Sint32 px, Sint32 py,
		Sint32 tx, Sint32 ty, Sint32 width, Sint32 height, Uint32 color, void* data), void* data);

// Draws an image
// dest is where on the screen it will be drawn
// src is the image position to copy from, spritesheet coordinates stuff
// width and height are the max size to be drawn, bounds checking will be done
// Negative width or height will reverse the image in that direction
// Pass a function to call at every pixel, kind of like a shader
// This is NOT bounds checked, make sure 0 <= x < surface->w and 0 <= y surface->h
// Aligned: 0 0 means centered, -1 -1 means origin is top left, 1 1 is bottom right
PIXEL_FN void drawImageFnANC(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height, float alignX, float alignY,
	void(pixelFn)(SDL_Surface* surface, Sint32 px, Sint32 py,
		Sint32 tx, Sint32 ty, Sint32 width, Sint32 height, Uint32 color, void* data), void* data);