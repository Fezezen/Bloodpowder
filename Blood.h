#pragma once
#include "Actor.h"

class Blood : public Actor
{
public:
	Texture sprite;

	Blood(xyz);

	void Draw(SDL_Renderer* renderer, portalWnd& now, float pcos, float psin, float px, float py, float ph, float pitch);
	void Damage(int damage);

	void Initalize(b2World & world);
};

