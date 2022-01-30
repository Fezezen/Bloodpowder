#include "Gun.h"
#include <iostream>
#include "main.h"

SDL_Texture * Gun::LoadTexture(const char * dir)
{
	SDL_Surface* s = SDL_ConvertSurfaceFormat(SDL_LoadBMP(dir), SDL_PIXELFORMAT_ABGR8888, 0);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, s);
	SDL_FreeSurface(s);
	return tex;
}

Gun::Gun(SDL_Renderer * renderer, Player* player)
{
	this->renderer = renderer;
	wielder = player;
}

void Gun::Draw()
{
	SDL_Rect r;
	r.x = dstRect->x + offset.x, r.y = dstRect->y + offset.y;
	r.w = dstRect->w, r.h = dstRect->h;

	SDL_RenderCopy(renderer, sprites[curSprite], NULL, &r);
}

void Gun::Destroy()
{
	for (std::vector<SDL_Texture*>::iterator it = sprites.begin(); it != sprites.end(); ++it) 
	{
		SDL_DestroyTexture(*it);
	}


	for (int i = 0; i < sfx.size(); i++) {
		Mix_FreeChunk(sfx[i]);
		sfx[i] = nullptr;
	}

	delete dstRect;
}

int Gun::GetAmmo()
{
	return ammo;
}

void Gun::AddAmmo(int amount)
{
	ammo = clamp(ammo+amount, 0, maxAmmo);
}

void Gun::ChangeAnimationState(float delay, int state)
{
	CreateDelay_Main(delay, [this,state]() {
		curSprite = state;
	});
}
