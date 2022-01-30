#pragma once
#include <box2d/box2d.h>
#include <vector>
#include <SDL_mixer.h>

const float gravity = 140.0f;

union SDL_Event; // forward delcaration
struct Sector;

static class Player
{
private:
	const float jumpSpeed = 40.0f;
	const float speed = 10.0f;
	const float turnSpeed = 2.0f;

	b2BodyDef bodyDef;
	b2CircleShape shape;
	b2Body* body;

	int input[4];

	void Move(float dx, float dy);

	bool grounded;
public:
	struct xyz { float x, y, z; } pos, velocity; // position and velocity
	float angle, anglesin, anglecos, pitch; // looking angle, with sin and cos
	int scroll = 0;
	unsigned int cur_sector;// current sector
	std::vector<Sector> sectors; // sector data

	const float eyeHeight = 6.0f;
	const float headMargin = 1.0f;
	const float kneeHeight = 3.0f;
	float currentEyeHeight = eyeHeight;

	b2Fixture* fixture;

	bool moving = false;
	bool dead = false;

	float Pitch(float y, float z);

	void HandleInput(SDL_Event &event, double deltaTime);

	void Update(double deltaTime);

	void Initalize(b2World& world);
} player;