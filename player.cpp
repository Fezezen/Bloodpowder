#include "Player.h"
#include <SDL.h>
#include <algorithm>
#include <math.h>
#include <iostream>

#include "MathHeader.h"
#include "sector.h"
#include "CollisionTypes.h"

float Player::Pitch(float y, float z) {
	return y + z * pitch;
}

void Player::HandleInput(SDL_Event & event, double deltaTime)
{
	switch (event.type)
	{
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym)
		{
		case SDLK_SPACE:
			if (grounded)
				velocity.y += jumpSpeed;
			break;
		default:
			break;
		}
	case SDL_KEYUP:
		switch (event.key.keysym.sym)
		{
		case SDLK_w: case SDLK_UP: input[0] = event.type == SDL_KEYDOWN; break;
		case SDLK_s: case SDLK_DOWN: input[1] = event.type == SDL_KEYDOWN; break;
		case SDLK_d: case SDLK_RIGHT: input[2] = event.type == SDL_KEYDOWN; break;
		case SDLK_a: case SDLK_LEFT: input[3] = event.type == SDL_KEYDOWN; break;
		default: break;
		}
		break;
	case SDL_MOUSEMOTION:
		int x, y;
		SDL_GetRelativeMouseState(&x, &y);

		angle += x*.002f; // I _think_ this is already independant of framerate and therefore cannot be multiplied by delta time
		pitch = clamp(pitch + y * 0.005f, -1.5f, 1.5f);

		scroll += x*.5;

		break;
	default:
		break;
	}
}

//void player::Move(float dx, float dy) {
void Player::Move(float dx, float dy) {
	const Sector*sect = &sectors[cur_sector];
	const Vector2* vert = sect->vertex;
	float px = pos.x, py = pos.z;

	for (unsigned s = 0; s < sect->npoints-1; ++s) {
		if (sect->neighbors[s] < 0) continue;

		float vx0 = vert[s + 0].x, vy0 = vert[s + 0].y;
		float vx1 = vert[s + 1].x, vy1 = vert[s + 1].y;

		if (IntersectBox(px, py, px + dx, py + dy, vx0, vy0, vx1, vy1)
			&& PointSide(px + dx, py + dy, vx0, vy0, vx1, vy1) < 0)
		{
			cur_sector = sect->neighbors[s];
			
			break;
		}
	}
	pos.x += dx;
	pos.z += dy;
}

void Player::Update(double deltaTime) {
	anglesin = sinf(angle);
	anglecos = cosf(angle);

	if (dead) {
		currentEyeHeight = currentEyeHeight + ((1.0f - currentEyeHeight) * deltaTime*5.0f);
	}

	float move_vec[2] = { 0.0f,0.0f };
	if (input[0]) { move_vec[0] += anglecos*speed; move_vec[1] += anglesin*speed; }
	if (input[1]) { move_vec[0] -= anglecos*speed; move_vec[1] -= anglesin*speed; }
	if (input[2]) { move_vec[0] -= anglesin*speed; move_vec[1] += anglecos*speed; }
	if (input[3]) { move_vec[0] += anglesin*speed; move_vec[1] -= anglecos*speed; }
	int pushing = (input[0] || input[1] || input[2] || input[3])&&!dead;
	float accel = (pushing ? 15.0f : 7.0f);
	moving = pushing;

	//player.velocity.x = move_vec[0] * deltaTime * speed;
	//player.velocity.z = move_vec[1] * deltaTime * speed;
	if (pushing) {
		velocity.x = velocity.x * (1 - accel * deltaTime) + move_vec[0] * accel * deltaTime;
		velocity.z = velocity.z * (1 - accel * deltaTime) + move_vec[1] * accel * deltaTime;
	}
	else {
		velocity.x = velocity.x * (1 - accel * deltaTime);
		velocity.z = velocity.z * (1 - accel * deltaTime);
	}
	//player.pos.x += player.velocity.x * deltaTime;
	//player.pos.z += player.velocity.z * deltaTime;

	unsigned int size = sectors.size();
	cur_sector = clamp(cur_sector, (unsigned)0, size-1);

	const Sector*sect = &sectors[cur_sector];
	const Vector2* vert = sect->vertex;
	float px = pos.x, py = pos.z;
	float dx = velocity.x, dy = velocity.z;

	for (unsigned s = 0; s < sect->npoints; ++s) {
		float vx0 = vert[s + 0].x, vy0 = vert[s + 0].y;
		float vx1 = vert[s + 1].x, vy1 = vert[s + 1].y;

		if (sect->neighbors[s] < 0 || sect->neighbors[s] > size) continue;

		if (IntersectBox(px, py, px + dx, py + dy,
			vx0, vy0,
			vx1, vy1)
			&& PointSide(px + dx, py + dy,
				vx0, vy0,
				vx1, vy1) < 0)
		{
			float hole_low = std::max(sect->floor, sectors[sect->neighbors[s]].floor);
			float hole_high =  std::min(sect->ceil, sectors[sect->neighbors[s]].ceil);
			// Check whether we're bumping into a wall.
			
			if (hole_high < pos.y + headMargin
				|| hole_low  > pos.y - currentEyeHeight + kneeHeight)
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

	b2Vec2 v = b2Vec2(velocity.x,velocity.z);
	body->SetLinearVelocity(v);

	b2Vec2 np = body->GetPosition();
	// Move Player
	Move(np.x - pos.x, np.y - pos.z);

	float yGoal = pos.y + velocity.y*deltaTime;
	//std::cout << cur_sector << std::endl;
	if (yGoal > sectors[cur_sector].ceil) {
		velocity.y = 0;
		pos.y = sectors[cur_sector].ceil;
	}
	else if (yGoal - currentEyeHeight <= sectors[cur_sector].floor) {
		velocity.y = 0;
		pos.y = sectors[cur_sector].floor + currentEyeHeight;
		grounded = true;
	}
	else {
		pos.y = yGoal;
		velocity.y -= deltaTime * gravity;
		grounded = false;
	}
}

void Player::Initalize(b2World & world)
{
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x, pos.z);

	body = world.CreateBody(&bodyDef);

	shape.m_radius = .5f;
	b2FixtureDef fixDef;
	fixDef.shape = &shape;
	fixDef.density = 1.0f;
	fixDef.filter.categoryBits = CollisionTypes::PLAYER;
	fixDef.filter.maskBits = CollisionTypes::ACTOR | CollisionTypes::WALL;

	fixture = body->CreateFixture(&fixDef);
}

