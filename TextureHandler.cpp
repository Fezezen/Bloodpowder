#include "TextureHandler.h"

Uint32 getPixel(SDL_Surface* surface, Sint16 x, Sint16 y)
{
	if (x >= 0 && y >= 0 && x < surface->w && y < surface->h)
	{
		int bpp = surface->format->BytesPerPixel;
		Uint8 *pixel = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

		switch (bpp) {
		case 1:
			return *pixel;
		case 2:
			return *(Uint16 *)pixel;
		case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			return pixel[0] << 16 | pixel[1] << 8 | pixel[2];
#else
			return pixel[0] | pixel[1] << 8 | pixel[2] << 16;
#endif
		case 4:
			return *(Uint32 *)pixel;
		}
	}
	return 0;
}

Texture LoadTexture(const char * dir)
{
	SDL_Surface* texture = SDL_ConvertSurfaceFormat(SDL_LoadBMP(dir), SDL_PIXELFORMAT_ABGR8888, 0);// This was necessary because apparently the bitmap had a different format
	Texture tex;
	tex.pixels = new Uint32*[texture->w];
	tex.w = texture->w;
	tex.h = texture->h;

	for (int x = 0; x < tex.w; x++) {
		tex.pixels[x] = new Uint32[tex.h];
		for (int y = 0; y < tex.h; y++) {
			tex.pixels[x][y] = getPixel(texture, x, y);
		}
	}
	SDL_FreeSurface(texture);
	return tex;
}

void DestroyTexture(Texture & tex)
{
	if (&tex != nullptr) {
		delete[] tex.pixels;
		tex.pixels = nullptr;
	}
}