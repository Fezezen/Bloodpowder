#include "Blood.h"
#include <algorithm>
#include "TextureHandler.h"
#include "Sector.h"
#include "MathHeader.h"
#include "player.h"
#include "CollisionTypes.h"

Blood::Blood(xyz p):Actor(p)
{
	sprite = LoadTexture("Textures/corpse.bmp");

	height = 3.0f;
	width = 3.0f;
	half_w = width / 2;
	dead = true;

	tag = "Blood";
}

void Blood::Initalize(b2World & world)
{
	this->world = &world;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x, pos.z);

	body = world.CreateBody(&bodyDef);

	shape.SetAsBox(boundingBox.x, boundingBox.y);

	b2FixtureDef fixDef;
	fixDef.shape = &shape;
	fixDef.density = 500.0f;
	fixDef.friction = 1000;
	fixDef.filter.categoryBits = CollisionTypes::ACTOR;
	fixDef.filter.maskBits = CollisionTypes::WALL | CollisionTypes::ACTOR;

	fixture = body->CreateFixture(&fixDef);
}

void Blood::Draw(SDL_Renderer * renderer, portalWnd & now, float pcos, float psin, float px, float py, float ph, float pitch)
{
	float vx0 = pos.x - px + psin * half_w;
	float vy0 = pos.z - py - pcos * half_w;
	float vx1 = pos.x - px - psin * half_w;
	float vy1 = pos.z - py + pcos * half_w;

	float tx1 = vx0 * psin - vy0 * pcos, tz1 = vx0 * pcos + vy0 * psin;
	float tx2 = vx1 * psin - vy1 * pcos, tz2 = vx1 * pcos + vy1 * psin;

	if (tz1 <= 0 && tz2 <= 0) return;// make sure the wall isn't behind the player

	int u0 = 0, u1 = sprite.w;

	if (tz1 <= 0 || tz2 <= 0) {
		// perspective math
		const float nearz = 1e-4f, farz = 5, nearside = 1e-5f, farside = 20.f;

		Vector2 org1 = { tx1,tz1 }, org2 = { tx2,tz2 };

		float ix1, iz1;
		float ix2, iz2;
		Intersect(tx1, tz1, tx2, tz2, -nearside, nearz, -farside, farz, ix1, iz1);
		Intersect(tx1, tz1, tx2, tz2, nearside, nearz, farside, farz, ix2, iz2);
		if (tz1 < nearz) if (iz1 > 0)  tx1 = ix1, tz1 = iz1; else tx1 = ix2, tz1 = iz2;
		if (tz2 < nearz) if (iz1 > 0) tx2 = ix1, tz2 = iz1;	else tx2 = ix2, tz2 = iz2;
	}

	float xscale1 = (w_width*hfov) / tz1, yscale1 = (w_height*vfov) / tz1; int x1 = h_width + (int)(-tx1 * xscale1);
	float xscale2 = (w_width*hfov) / tz2, yscale2 = (w_height*vfov) / tz2; int x2 = h_width + (int)(-tx2 * xscale2);
	if (x1 >= x2 || x2 < now.wndX1 || x1 > now.wndX2) return;

	float top = pos.y + height - ph, bottom = pos.y - ph;

	int y1a = h_height - (int)((top + tz1 * pitch) * yscale1), y1b = h_height - (int)((bottom + tz1 * pitch) * yscale1); // top left point and bottom left
	int y2a = h_height - (int)((top + tz2 * pitch) * yscale2), y2b = h_height - (int)((bottom + tz1 * pitch) * yscale2);// top right point and bottom right

	int xbegin = std::max(x1, now.wndX1), xend = std::min(x2, now.wndX2);
	Scaler ya_int = Scaler_Init(x1, xbegin, x2, y1a, y2a);
	Scaler yb_int = Scaler_Init(x1, xbegin, x2, y1b, y2b);

	//vline_Solid(xbegin, y1a, y1b, 0xFF00FFFF);
	//vline_Solid(xend, y2a, y2b, 0xFF0000FF);

	for (int x = xbegin; x < xend; x++) {
		int txtx = (u0*((x2 - x)*tz2) + u1 * ((x - x1)*tz1)) / ((x2 - x)*tz2 + (x - x1)*tz1);

		int ya = Scaler_Next(&ya_int);
		int yb = Scaler_Next(&yb_int);

		int cya = clamp(ya, now.yTop[x], now.yBottom[x]);
		int cyb = clamp(yb, now.yTop[x], now.yBottom[x]);

		int y1 = clamp(cya, 0, w_height - 1);
		int y2 = clamp(cyb, 0, w_height - 1);

		vline_Texutred(x, cya, cyb, Scaler_Init(ya, cya, yb + 1, 0, sprite.h), txtx, 1, renderer, sprite);
	}
}

void Blood::Damage(int damage)
{
}
