#pragma once
#include <SDL.h>

typedef Uint32** TextureData; // Type def to make this easier to read

struct Texture { // Structure to store width and height
	TextureData pixels;
	int w, h;
};

Texture LoadTexture(const char* dir);
void DestroyTexture(Texture& tex);