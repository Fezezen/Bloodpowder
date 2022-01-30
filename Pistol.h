#pragma once
#include "Gun.h"

struct SDL_Rect;

class Pistol : public Gun
{
private:
	int x = 204, y = 226;
	int w = 230, h = 319;

	enum spriteTypes {
		IDLE = 0,
		MUZZLE_FLASH = 1,
		RECOIL_0 = 2,
		RECOIL_1 = 3,
		RECOIL_2 = 4,
	};

public:
	Pistol(SDL_Renderer* renderer, Player* player);

	void Fire1();
};

