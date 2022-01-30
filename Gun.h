#pragma once
#include<vector>
#include <SDL.h>
#include "MathHeader.h"
#include <SDL_mixer.h>
#include "player.h"

// This is an abstract class that handles most of the things guns do

class Gun
{
protected:
	SDL_Rect* dstRect;
	SDL_Renderer* renderer;

	int curSprite = 0;
	std::vector<SDL_Texture*> sprites;

	SDL_Texture* LoadTexture(const char*);

	float fireRate = .3f;
	int ammo = 0,maxAmmo = 0;
	int damage = 0;
	bool canFire = true; // determines if it can currently fire (fire rate)

	std::vector<Mix_Chunk*> sfx;

	Player* wielder;
public:
	Vector2 offset; // for sway

	Gun(SDL_Renderer* renderer, Player* player);

	virtual void Fire1() {};

	void Draw();
	void Destroy();

	int GetAmmo();
	void AddAmmo(int amount);

	void ChangeAnimationState(float, int);
};