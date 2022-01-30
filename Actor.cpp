#include "Actor.h"
#include <iostream>
#include "player.h"
#include "TextureHandler.h"
#include "Sector.h"
#include "CollisionTypes.h"

Actor::Actor()
{
	pos = { 0,0,0 };
	half_w = width / 2;
}

Actor::Actor(xyz p, Player* play):target(nullptr)
{
	target = play;
	pos = p;
	half_w = width / 2;
	cur_sector = 0;
};

void Actor::Initalize(b2World & world)
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
	fixDef.filter.maskBits = CollisionTypes::PLAYER | CollisionTypes::WALL;

	fixture = body->CreateFixture(&fixDef);
}

void Actor::Update(float deltaTime, float px, float py)
{
	b2Vec2 np = {pos.x,pos.z};

	if (body != nullptr) {
		body->SetLinearVelocity(b2Vec2(velocity.x, velocity.z));
		np = body->GetPosition();
	}

	float angle_p = angle- 0.1f+atan2f(py - pos.z, px - pos.x)*8/twoPi;
	currentRot = abs(fmodf(angle_p-4, 8));

	int size = sectors.size();
	const Sector*sect = &sectors[cur_sector];
	const Vector2* vert = sect->vertex;
	float px0 = pos.x, py0 = pos.z;
	float dx = velocity.x, dy = velocity.z;

	for (unsigned s = 0; s < sect->npoints-1; ++s) {
		float vx0 = vert[s + 0].x, vy0 = vert[s + 0].y;
		float vx1 = vert[s + 1].x, vy1 = vert[s + 1].y;

		if (sect->neighbors[s] < 0) continue;

		if (IntersectBox(px0, py0, px0 + dx, py0 + dy,
			vx0, vy0,
			vx1, vy1)
			&& PointSide(px0 + dx, py0 + dy,
				vx0, vy0,
				vx1, vy1) != 2)
		{
			float hole_low = std::max(sect->floor, sectors[sect->neighbors[s]].floor);
			float hole_high = std::min(sect->ceil, sectors[sect->neighbors[s]].ceil);
			// Check whether we're bumping into a wall.

			if (hole_high < pos.y + height
				|| hole_low  > pos.y + 3.0f)
			{
				b2Filter filter = fixture->GetFilterData();
				filter.groupIndex = 0;
				fixture->SetFilterData(filter);
			}
			else {
				b2Filter filter = fixture->GetFilterData();
				filter.groupIndex = -1;
				fixture->SetFilterData(filter);
			}
		}
	}

	MoveSectors(np.x - pos.x, np.y - pos.z);

	float yGoal = pos.y + velocity.y*deltaTime;
	if (yGoal + height > sectors[cur_sector].ceil) {
		velocity.y = 0;
		pos.y = sectors[cur_sector].ceil - height;
	}
	else if (yGoal <= sectors[cur_sector].floor) {
		velocity.y = 0;
		pos.y = sectors[cur_sector].floor;
	}
	else {
		pos.y = yGoal;
		velocity.y -= deltaTime * gravity;
	}

	if (dead)
		deadTimer += deltaTime;
}

void Actor::Draw(SDL_Renderer* renderer, portalWnd& now, float pcos, float psin, float px, float py, float ph, float pitch)
{
	float vx0 = pos.x - px + psin* half_w;
	float vy0 = pos.z - py - pcos * half_w;
	float vx1 = pos.x - px - psin * half_w;
	float vy1 = pos.z - py + pcos * half_w;

	float tx1 = vx0 * psin - vy0 * pcos, tz1 = vx0 * pcos + vy0 * psin;
	float tx2 = vx1 * psin - vy1 * pcos, tz2 = vx1 * pcos + vy1 * psin;

	if (tz1 <= 0 && tz2 <= 0) return;// make sure the wall isn't behind the player

	Texture sprite = sprites[currentSprite].a[currentRot];

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

	float top = pos.y+height- ph, bottom = pos.y- ph;

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

void Actor::MoveSectors(float dx, float dy)
{
	const Sector*sect = &sectors[cur_sector];
	const Vector2* vert = sect->vertex;
	float px = pos.x, py = pos.z;

	for (unsigned s = 0; s < sect->npoints - 1; ++s) {
		if (sect->neighbors[s] < 0) continue;

		float vx0 = vert[s + 0].x, vy0 = vert[s + 0].y;
		float vx1 = vert[s + 1].x, vy1 = vert[s + 1].y;

		if (IntersectLineSegments(px,py,px+dx,py+dy,vx0,vy0,vx1,vy1))
		{
			int last = cur_sector;
			cur_sector = sect->neighbors[s];
			MoveActorToSector(this, last, cur_sector);
			break;
		}
	}
	pos.x += dx;
	pos.z += dy;
}

void Actor::ChangeAnimationState(float delay, int state, int priorState = -1) // just so something like an attack wouldn't override the death animation
{
	CreateDelay_Main(delay, [this, state, priorState]() {
		if (priorState == -1 || priorState == currentSprite)
			currentSprite = state;
	});
}

void Actor::Destroy(bool end)
{
	for (int i = 0; i < sprites.size(); i++) {
		for (int b = 0; b < 8; b++) {
			DestroyTexture(sprites[i].a[b]);
		}
	}

	if (end){
		if (body != nullptr) {
			world->DestroyBody(body);
			body = nullptr;
		}
	}
	else {
		QueDestroyBody(body);
	}
}
