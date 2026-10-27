#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "graphics.h"

const SDL_PixelFormatDetails* pixFmt = NULL;

void setRect(SDL_Surface* surface, Uint32 x, Uint32 y, Uint32 w, Uint32 h, Uint32 color)
{
	for (Uint32 iy = 0; iy < h; iy++)
		setLine(surface, x, y + iy, w, color);
}

void clearScreen(SDL_Surface* surface, Uint32 color)
{
	setLine(surface, 0, 0, surface->w * surface->h, color);
}


void drawRect(SDL_Surface* surface, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint32 color)
{
	const Sint32 border = 0;
	if (w < 0)
	{
		x += w;
		w = -w;
	}
	if (h < 0)
	{
		y += h;
		h = -h;
	}
	if (x < border)
	{
		w += x - border;
		x = border;
	}
	if (y < border)
	{
		h += y - border;
		y = border;
	}
	if (x >= surface->w - border || y >= surface->h - border || w <= 0 || h <= 0)
		return;
	if (x + w >= surface->w - border)
		w = surface->w - border - x;
	if (y + h >= surface->h - border)
		h = surface->h - border - y;

	setRect(surface, x, y, w, h, color);
}

void drawRectA(SDL_Surface* surface, Sint32 x, Sint32 y, float alignX, float alignY, Sint32 w, Sint32 h, Uint32 color)
{
	drawRect(surface, x - w / 2.f - w * alignX / 2.f, y - h / 2.f - h * alignY / 2.f, w, h, color);
}

void drawRectOut(SDL_Surface* surface, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint32 size, Uint32 color)
{
	drawRect(surface, x, y, w, size, color);
	drawRect(surface, x, y, size, h, color);
	drawRect(surface, x, y + h - size, w, size, color);
	drawRect(surface, x + w - size, y, size, h, color);
}

void drawRectOutA(SDL_Surface* surface, Sint32 x, Sint32 y, float alignX, float alignY, Sint32 w, Sint32 h, Uint32 size, Uint32 color)
{
	drawRectOut(surface, x - w / 2.f - w * alignX / 2.f, y - h / 2.f - h * alignY / 2.f, w, h, size, color);
}


void drawCircle(SDL_Surface* surface, Sint32 x, Sint32 y, Uint32 r, Uint32 color)
{
	if (r == 0)
		return;

	Sint32 xx = x - r;
	Sint32 yy = y - r;
	Sint32 w = r * 2 + 1;
	Sint32 h = w;
	const Sint32 border = 0;
	if (xx < border)
	{
		w += xx - border;
		xx = border;
	}
	if (yy < border)
	{
		h += yy - border;
		yy = border;
	}
	if (xx >= surface->w - border || yy >= surface->h - border || w <= 0 || h <= 0)
		return;
	if (xx + w >= surface->w - border)
		w = surface->w - border - xx;
	if (yy + h >= surface->h - border)
		h = surface->h - border - yy;

	r *= r;

	for (Sint32 iy = 0; iy < h; iy++)
	{
		Sint32 yp = yy + iy;
		Sint32 ypow = y - yp;
		ypow *= ypow;

		// Find end points of circle outline, then rectangle fill
		Sint32 ixl = 0;
		for (; ixl < w; ixl++)
		{
			Sint32 xpow = x - (xx + ixl);
			xpow *= xpow;
			Sint32 dist = xpow + ypow;

			// Attempt at rounding after sqrt approx
			// Only needed at small sizes anyways
			// I have no idea how this actually helps
			dist &= ~0b11;
			if (dist <= r)
			//if (xpow + ypow <= r)
			//if (round(sqrtf(xpow + ypow)) <= r)
				break;
		}
		if (ixl == w)
			continue;

		Sint32 ixr = w - 1;
		for (; ixr >= 0; ixr--)
		{
			Sint32 xpow = x - (xx + ixr);
			xpow *= xpow;
			Sint32 dist = xpow + ypow;
			dist &= ~0b11;
			if (dist <= r)
			//if (xpow + ypow <= r)
			//if (round(sqrtf(xpow + ypow)) <= r)
				break;
		}
		if (ixr == -1)
			continue;

		Sint32 width = ixr + 1 - ixl;
		//printf("w:%3d l:%3d r:%3d\n", width, ixl, ixr);
		if (width > 0)
			setLine(surface, xx + ixl, yp, width, color);
	}
}

void drawCircleA(SDL_Surface* surface, Sint32 x, Sint32 y, float alignX, float alignY, Uint32 r, Uint32 color)
{
	drawCircle(surface, x - r * alignX, y - r * alignY, r, color);
}

#include "bulletinV1_font.h"

int letterSpacing = 1;
int lineSpacing = 1;

void getTextSize(const char* text, Uint32 size, Uint32* width, Uint32* height)
{
	Uint32 w = 0;
	Uint32 h = 0;
	
	if (text)
	{
		h = (size ? font_h : font_h / 2);

		Uint32 x = 0;
		Uint32 y = (size ? font_h : font_h / 2);

		for (Uint32 i = 0; text[i]; i++)
		{
			if (text[i] == '\n')
			{
				if (x > w)
					w = x;
				y += (size ? font_h : font_h / 2) + lineSpacing;
				x = 0;
			}
			else
			{
				x += (size ? font_w : font_w / 2) + letterSpacing;
				h = y;
			}
		}
		if (x > w)
			w = x;
	}

	if (width)
		*width = w;
	if (height)
		*height = h;
}

void getTextSizeF(Uint32 size, Uint32* width, Uint32* height, const char* fmt, ...)
{
	va_list prf0, prf1;
	va_start(prf0, fmt);

	va_copy(prf1, prf0);
	Sint32 len = vsnprintf(NULL, 0, fmt, prf0);
	va_end(prf0);

	if (len <= 100)
	{
		char smBuf[100];
		vsnprintf(smBuf, 100, fmt, prf1);
		getTextSize(smBuf, size, width, height);
	}
	else
	{
		char* buf = (char*)malloc(len);
		if (buf)
		{
			vsnprintf(buf, len, fmt, prf1);
			getTextSize(buf, size, width, height);
			free(buf);
		}
	}

	va_end(prf1);
}

static inline bool getArrayBit(const Uint32 array[], Uint32 i)
{
	Uint32 index = i >> 5;
	Uint32 shift = 31 - (i & 0b11111);
	return (array[index] >> shift) & 1;
}


void drawChar(SDL_Surface* surface, Sint32 x, Sint32 y, Uint32 size, Uint32 color, char ch)
{
	const int border = 0;

	if (size)
	{
		Sint32 index = (ch - ' ') * stride;

		// completely inside surface
		if (x >= border && y >= border &&
			x + size * font_w < surface->w - border &&
			y + size * font_h < surface->h - border)
		{
			for (Sint32 fy = 0; fy < font_h; fy++)
				for (Sint32 fx = 0; fx < font_w; fx++)
					if (getArrayBit(&font[index], fx + fy * font_w))
						for (Sint32 ix = 0; ix < size; ix++)
							for (Sint32 iy = 0; iy < size; iy++)
								setPixelUC(surface, x + fx * size + ix, y + fy * size + iy, color);
		}
		// partially outside surface
		else if (x + size * font_w >= border && y + size * font_h >= border &&
			x < surface->w - border && y < surface->h - border)
		{
			for (Sint32 fy = 0; fy < font_h; fy++)
				for (Sint32 fx = 0; fx < font_w; fx++)
					if (getArrayBit(&font[index], fx + fy * font_w))
						for (Sint32 ix = 0; ix < size; ix++)
						{
							Sint32 xp = x + fx * size + ix;
							if (xp >= 0 && xp < surface->w)
								for (Sint32 iy = 0; iy < size; iy++)
								{
									Sint32 yp = y + fy * size + iy;
									if (yp >= 0 && yp < surface->h)
										setPixelUC(surface, xp, yp, color);
								}
						}
		}
	}
	else // Hacky way of reducing text size, samples top left of 2x2 pixel section
	{
		Sint32 index = (ch - ' ') * stride;

		// completely inside surface
		if (x >= border && y >= border &&
			x + font_w / 2 < surface->w - border &&
			y + font_h / 2 < surface->h - border)
		{
			for (Sint32 fy = 0; fy < font_h; fy += 2)
				for (Sint32 fx = 0; fx < font_w; fx += 2)
					if (getArrayBit(&font[index], fx + fy * font_w))
						setPixelUC(surface, x + fx / 2, y + fy / 2, color);
		}		
		// partially outside surface
		else if (x + font_w / 2 >= border && y + font_h / 2 >= border &&
			x < surface->w - border && y < surface->h - border)
		{
			for (Sint32 fy = 0; fy < font_h; fy += 2)
			{
				Sint32 yp = y + fy / 2;
				if (yp >= 0 && yp < surface->h)
					for (Sint32 fx = 0; fx < font_w; fx += 2)
					{
						Sint32 xp = x + fx / 2;
						if (xp >= 0 && xp < surface->w)
							if (getArrayBit(&font[index], fx + fy * font_w))
								setPixelUC(surface, xp, yp, color);
					}
			}
		}
	}
}

void drawCharA(SDL_Surface* surface, Sint32 x, Sint32 y, float alignX, float alignY, Uint32 size, Uint32 color, char ch)
{
	drawChar(surface,
		x - (size ? size * font_w : font_w / 2) / 2.f - (size ? size * font_w : font_w / 2) / 2.f * alignX,
		y - (size ? size * font_h : font_h / 2) / 2.f - (size ? size * font_h : font_h / 2) / 2.f * alignY,
		size, color, ch);
}

void drawText(SDL_Surface* surface, Sint32 x, Sint32 y, Uint32 size, Uint32 color, const char* text)
{
	Sint32 xx = 0;
	Sint32 yy = 0;
	for (Sint32 i = 0; text[i]; i++)
	{
		if (text[i] == '\n')
		{
			xx = 0;
			yy += (size ? size * font_h : font_h / 2) + lineSpacing;
		}
		else if (text[i] < 127 && text[i] >= ' ')
		{
			drawChar(surface, x + xx, y + yy, size, color, text[i]);
			xx += (size ? size * font_w : font_w / 2) + letterSpacing;
		}
	}
}

void drawTextA(SDL_Surface* surface, Sint32 x, Sint32 y, float alignX, float alignY, Uint32 size, Uint32 color, const char* text)
{
	Uint32 width, height;
	getTextSize(text, size, &width, &height);

	drawText(surface,
		x - width / 2.f - width / 2.f * alignX,
		y - height / 2.f - height / 2.f * alignY,
		size, color, text);
}

void drawTextF(SDL_Surface* surface, Sint32 x, Sint32 y, Sint32 size, Uint32 color, const char* fmt, ...)
{
	va_list prf0, prf1;
	va_start(prf0, fmt);

	va_copy(prf1, prf0);
	Sint32 len = vsnprintf(NULL, 0, fmt, prf0);
	va_end(prf0);

	if (len <= 100)
	{
		char smBuf[100];
		vsnprintf(smBuf, 100, fmt, prf1);
		drawText(surface, x, y, size, color, smBuf);
	}
	else
	{
		char* buf = (char*)malloc(len);
		if (buf)
		{
			vsnprintf(buf, len, fmt, prf1);
			drawText(surface, x, y, size, color, buf);
			free(buf);
		}
	}

	va_end(prf1);
}

void drawTextFA(SDL_Surface* surface, Sint32 x, Sint32 y, float alignX, float alignY, Sint32 size, Uint32 color, const char* fmt, ...)
{
	va_list prf0, prf1;
	va_start(prf0, fmt);

	va_copy(prf1, prf0);
	Sint32 len = vsnprintf(NULL, 0, fmt, prf0);
	va_end(prf0);

	if (len <= 100)
	{
		char smBuf[100];
		vsnprintf(smBuf, 100, fmt, prf1);
		drawTextA(surface, x, y, alignX, alignY, size, color, smBuf);
	}
	else
	{
		char* buf = (char*)malloc(len);
		if (buf)
		{
			vsnprintf(buf, len, fmt, prf1);
			drawTextA(surface, x, y, alignX, alignY, size, color, buf);
			free(buf);
		}
	}

	va_end(prf1);
}


void drawTextFn(SDL_Surface* surface, Sint32 x, Sint32 y, Uint32 size, Uint32 color,
	void(before)(SDL_Surface*, Sint32, Sint32, Sint32, Sint32, char, void*), void* data, const char* text)
{
	Sint32 xx = 0;
	Sint32 yy = 0;
	for (Sint32 i = 0; text[i]; i++)
	{
		if (before)	before(surface, x + xx, y + yy, size * font_w, size * font_h, text[i], data);
		if (text[i] == '\n')
		{
			xx = 0;
			yy += (size ? size * font_h : font_h / 2);
		}
		else if (text[i] < 127 && text[i] >= ' ')
		{
			drawChar(surface, x + xx, y + yy, size, color, text[i]);
			xx += (size ? size * font_w : font_w / 2);
		}
	}
}

void drawTextAFn(SDL_Surface* surface, Sint32 x, Sint32 y, float alignX, float alignY, Uint32 size, Uint32 color,
	void(before)(SDL_Surface*, Sint32, Sint32, Sint32, Sint32, char, void*), void* data, const char* text)
{
	Uint32 width, height;
	getTextSize(text, size, &width, &height);

	drawTextFn(surface,
		x - width / 2.f - width / 2.f * alignX,
		y - height / 2.f - height / 2.f * alignY,
		size, color, before, data, text);
}

void drawTextFFn(SDL_Surface* surface, Sint32 x, Sint32 y, Sint32 size, Uint32 color,
	void(before)(SDL_Surface*, Sint32, Sint32, Sint32, Sint32, char, void*), void* data, const char* fmt, ...)
{
	va_list prf0, prf1;
	va_start(prf0, fmt);

	va_copy(prf1, prf0);
	Sint32 len = vsnprintf(NULL, 0, fmt, prf0);
	va_end(prf0);

	if (len <= 100)
	{
		char smBuf[100];
		vsnprintf(smBuf, 100, fmt, prf1);
		drawTextFn(surface, x, y, size, color, before, data, smBuf);
	}
	else
	{
		char* buf = (char*)malloc(len);
		if (buf)
		{
			vsnprintf(buf, len, fmt, prf1);
			drawTextFn(surface, x, y, size, color, before, data, buf);
			free(buf);
		}
	}

	va_end(prf1);
}

void drawTextFAFn(SDL_Surface* surface, Sint32 x, Sint32 y, float alignX, float alignY, Sint32 size, Uint32 color,
	void(before)(SDL_Surface*, Sint32, Sint32, Sint32, Sint32, char, void*), void* data, const char* fmt, ...)
{
	va_list prf0, prf1;
	va_start(prf0, fmt);

	va_copy(prf1, prf0);
	Sint32 len = vsnprintf(NULL, 0, fmt, prf0);
	va_end(prf0);

	if (len <= 100)
	{
		char smBuf[100];
		vsnprintf(smBuf, 100, fmt, prf1);
		drawTextAFn(surface, x, y, alignX, alignY, size, color, before, data, smBuf);
	}
	else
	{
		char* buf = (char*)malloc(len);
		if (buf)
		{
			vsnprintf(buf, len, fmt, prf1);
			drawTextAFn(surface, x, y, alignX, alignY, size, color, before, data, buf);
			free(buf);
		}
	}

	va_end(prf1);
}


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Bitmap loadImage(const char* filename)
{
	Bitmap bmp = { 0 };

	int width = 0, height = 0, trash = 4;
	bmp.pixels = (Uint32*)stbi_load(filename, &width, &height, &trash, 4);
	if (bmp.pixels)
	{
		for (Uint32 y = 0; y < height; y++)
			for (Uint32 x = 0; x < width; x++)
			{
				RGB color = *(RGB*)&bmp.pixels[x + y * width];
				bmp.pixels[x + y * width] = rgb(color.r, color.g, color.b);
			}
		bmp.width = width;
		bmp.height = height;
	}

	return bmp;
}

void freeImage(Bitmap* image)
{
	if (image && image->pixels)
	{
		stbi_image_free(image->pixels);
		image->pixels = NULL;
		image->width = 0;
		image->height = 0;
	}
}

void drawImage(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height)
{
	bool revW = false;
	if (width < 0)
	{
		revW = true;
		destX += width;
		width = -width;
	}
	bool revH = false;
	if (height < 0)
	{
		revH = true;
		destY += height;
		height = -height;
	}

	if (width > surface->w)
		width = surface->w;
	if (height > surface->h)
		height = surface->h;

	if (destX < 0)
	{
		srcX -= destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0)
	{
		srcY -= destY;
		height += destY;
		destY = 0;
	}

	if (destX >= surface->w || destY >= surface->h)
		return;

	if (destX + width >= surface->w)
		width += surface->w - (destX + width);
	if (destY + height >= surface->h)
		height += surface->h - (destY + height);

	if (width <= 0 || height <= 0)
		return;

	if (!revW)
	{
		if (!revH) // normal
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(x + srcX) + (y + srcY) * image->width];
					setPixelUC(surface, x + destX, y + destY, pixel);
				}
		}
		else
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(x + srcX) + (image->height - 1 - (y + srcY)) * image->width];
					setPixelUC(surface, x + destX, y + destY, pixel);
				}
		}
	}
	else
	{
		if (!revH)
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(image->width - 1 - (x + srcX)) + (y + srcY) * image->width];
					setPixelUC(surface, x + destX, y + destY, pixel);
				}
		}
		else
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(image->width - 1 - (x + srcX)) + (image->height - 1 - (y + srcY)) * image->width];
					setPixelUC(surface, x + destX, y + destY, pixel);
				}
		}
	}
}

void drawImageA(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height, float alignX, float alignY)
{
	drawImage(surface, image, destX - width / 2 - width / 2.f * alignX,
		destY - height / 2 - height / 2.f * alignY, srcX, srcY, width, height);
}

void drawImageFn(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height,
	void(pixelFn)(SDL_Surface* surface, Sint32 px, Sint32 py,
		Sint32 tx, Sint32 ty, Sint32 width, Sint32 height, Uint32 color, void* data), void* data)
{
	bool revW = false;
	if (width < 0)
	{
		revW = true;
		destX += width;
		width = -width;
	}
	bool revH = false;
	if (height < 0)
	{
		revH = true;
		destY += height;
		height = -height;
	}

	if (width > surface->w)
		width = surface->w;
	if (height > surface->h)
		height = surface->h;

	if (destX < 0)
	{
		srcX -= destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0)
	{
		srcY -= destY;
		height += destY;
		destY = 0;
	}

	if (destX >= surface->w || destY >= surface->h)
		return;

	if (destX + width >= surface->w)
		width += surface->w - (destX + width);
	if (destY + height >= surface->h)
		height += surface->h - (destY + height);

	if (width <= 0 || height <= 0)
		return;

	if (!revW)
	{
		if (!revH) // normal
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(x + srcX) + (y + srcY) * image->width];
					if (pixelFn) pixelFn(surface, x + destX, y + destY, x + srcX, y + srcY, 
						image->width, image->height, pixel, data);
				}
		}
		else
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(x + srcX) + (image->height - 1 - (y + srcY)) * image->width];
					if (pixelFn) pixelFn(surface, x + destX, y + destY, x + srcX, image->height - 1 - (y + srcY),
						image->width, image->height, pixel, data);
				}
		}
	}
	else
	{
		if (!revH)
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(image->width - 1 - (x + srcX)) + (y + srcY) * image->width];
					if (pixelFn) pixelFn(surface, x + destX, y + destY, image->width - 1 - (x + srcX),
						y + srcY, image->width, image->height, pixel, data);
				}
		}
		else
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(image->width - 1 - (x + srcX)) + (image->height - 1 - (y + srcY)) * image->width];
					if (pixelFn) pixelFn(surface, x + destX, y + destY,
						image->width - 1 - (x + srcX), image->height - 1 - (y + srcY), 
						image->width, image->height, pixel, data);
				}
		}
	}
}

void drawImageFnA(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height, float alignX, float alignY,
	void(pixelFn)(SDL_Surface* surface, Sint32 px, Sint32 py, 
		Sint32 tx, Sint32 ty, Sint32 width, Sint32 height, Uint32 color, void* data), void* data)
{
	drawImageFn(surface, image, destX - width / 2 - width / 2.f * alignX,
		destY - height / 2 - height / 2.f * alignY, srcX, srcY, width, height, pixelFn, data);
}

void drawImageFnNC(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height,
	void(pixelFn)(SDL_Surface* surface, Sint32 px, Sint32 py,
		Sint32 tx, Sint32 ty, Sint32 width, Sint32 height, Uint32 color, void* data), void* data)
{
	bool revW = false;
	if (width < 0)
	{
		revW = true;
		destX += width;
		width = -width;
	}
	bool revH = false;
	if (height < 0)
	{
		revH = true;
		destY += height;
		height = -height;
	}

	if (!revW)
	{
		if (!revH) // normal
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(x + srcX) + (y + srcY) * image->width];
					if (pixelFn) pixelFn(surface, x + destX, y + destY, x + srcX, y + srcY,
						image->width, image->height, pixel, data);
				}
		}
		else
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(x + srcX) + (image->height - 1 - (y + srcY)) * image->width];
					if (pixelFn) pixelFn(surface, x + destX, y + destY, x + srcX, image->height - 1 - (y + srcY),
						image->width, image->height, pixel, data);
				}
		}
	}
	else
	{
		if (!revH)
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(image->width - 1 - (x + srcX)) + (y + srcY) * image->width];
					if (pixelFn) pixelFn(surface, x + destX, y + destY, image->width - 1 - (x + srcX),
						y + srcY, image->width, image->height, pixel, data);
				}
		}
		else
		{
			for (Uint32 y = 0; y < height; y++)
				for (Uint32 x = 0; x < width; x++)
				{
					Uint32 pixel = image->pixels[(image->width - 1 - (x + srcX)) + (image->height - 1 - (y + srcY)) * image->width];
					if (pixelFn) pixelFn(surface, x + destX, y + destY,
						image->width - 1 - (x + srcX), image->height - 1 - (y + srcY),
						image->width, image->height, pixel, data);
				}
		}
	}
}

void drawImageFnANC(SDL_Surface* surface, Bitmap* image, Sint32 destX, Sint32 destY,
	Sint32 srcX, Sint32 srcY, Sint32 width, Sint32 height, float alignX, float alignY,
	void(pixelFn)(SDL_Surface* surface, Sint32 px, Sint32 py,
		Sint32 tx, Sint32 ty, Sint32 width, Sint32 height, Uint32 color, void* data), void* data)
{
	drawImageFnNC(surface, image, destX - width / 2 - width / 2.f * alignX,
		destY - height / 2 - height / 2.f * alignY, srcX, srcY, width, height, pixelFn, data);
}