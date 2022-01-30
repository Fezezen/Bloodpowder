#include "Pistol.h"
#include <SDL.h>
#include <iostream>
#include <sstream>
#include "main.h"
#include <algorithm>
#include "rng.h"
#include "Actor.h"

std::string int_to_str(int x) {
	std::stringstream ss;
	ss << x;
	return ss.str();
}

Pistol::Pistol(SDL_Renderer * renderer, Player* player) : Gun(renderer, player)
{
	for (int i = 0; i < 5; i++) {
		std::string dir = "Textures/Guns/Pistol/Fire0" + int_to_str(i) + ".bmp";
		sprites.push_back(LoadTexture(dir.c_str()));
	}

	curSprite = IDLE;

	dstRect = new SDL_Rect();
	dstRect->x = x;
	dstRect->y = y;
	dstRect->w = w;
	dstRect->h = h;

	sfx.push_back(Mix_LoadWAV("Sounds/Guns/Pistol/pistol.wav"));
	sfx.push_back(Mix_LoadWAV("Sounds/Guns/Pistol/pistol1.wav"));
	sfx.push_back(Mix_LoadWAV("Sounds/Guns/Pistol/pistol2.wav"));

	ammo = 20;
	maxAmmo = 20;
	damage = 25;
}

void Pistol::Fire1() 
{
	if (!canFire || ammo <= 0) return;
	canFire = false;

	CreateDelay_Main(fireRate, [this]() {
		canFire = true;
	});

	curSprite = MUZZLE_FLASH;

	float t = .1;
	ChangeAnimationState(t, RECOIL_0);
	ChangeAnimationState(t+.1f, RECOIL_2);
	ChangeAnimationState(t+.15f, RECOIL_1);
	ChangeAnimationState(t+.2f, IDLE);

	ammo = std::max(ammo - 1, 0);

	int rng = (int)((Random() / 256.0f) * 3);
	Mix_VolumeChunk(sfx[rng], 20);
	Mix_PlayChannel(-1, sfx[rng], 0);

	float px = wielder->pos.x, pz = wielder->pos.z;
	/*std::pair<Vector2, Actor*> result = Hitscan(px,pz, px + wielder->anglecos * 100, pz + wielder->anglesin * 100, wielder->anglecos, wielder->anglesin, &wielder->sectors[player.cur_sector], wielder->sectors);
	if (result.second != nullptr) {
		result.second->Damage(damage);
	}*/

	std::pair<Vector2, Actor*> result = p_Hitscan(px, pz, px + wielder->anglecos * 100, pz + wielder->anglesin * 100, wielder->cur_sector);
	if (result.second != nullptr) {
		result.second->Damage(damage);
	}
}

