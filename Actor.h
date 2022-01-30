#pragma once
#include "MathHeader.h"
#include "main.h"
#include <SDL.h>
#include <vector>
#include <box2d/box2d.h>

//DESCRIPTION:
/*
This class is the base class for all sprite based things
It handles rendering the sprite and movement
And collisions
*/

struct Texture;
struct Sector;
struct Player;

struct SpritePack {
	Texture a[8];
};

class Actor
{
protected:
	// Physics stuff
	b2BodyDef bodyDef;
	b2PolygonShape shape;
	std::vector<SpritePack> sprites;
	
	b2World* world;
public:
	struct xyz { float x, y, z; } pos, velocity; // the bottom center of the sprite
	float height = 9.0f; // physical width and height, not sprite width and hieght
	float width = 2.0f, half_w;
	int health = 0;
	bool dead = false;
	float deadTimer = 0;

	b2Body* body;

	const char* tag = "Actor";

	float angle = 0.0f;

	Vector2 boundingBox = { 1,1 }; // used for collisions

	unsigned int currentSprite;
	unsigned int currentRot = 0;

	Player* target;

	b2Fixture* fixture;

	std::vector<Sector> sectors; // reference to the sectors vector defined in main
	unsigned int cur_sector; // index of current sector

	Actor();
	Actor(xyz, Player* t = nullptr);

	virtual void Initalize(b2World& world);

	virtual void Update(float deltaTime, float px, float py);
	virtual void Draw(SDL_Renderer* renderer, portalWnd& now, float pcos, float psin, float px, float py, float ph, float pitch);

	void MoveSectors(float dx, float dy);

	virtual void Damage(int damage) = 0;

	virtual void Destroy(bool end);

	void ChangeAnimationState(float delay, int state, int priorState);
};

