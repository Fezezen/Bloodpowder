#pragma once
#include <box2d/box2d.h>
#include <vector>
#include"MathHeader.h"
#include"TextureHandler.h"

class Actor;

struct Sector
{
	float floor, ceil;
	Vector2 *vertex; // Each vertex in a clockwise order
	int *neighbors;
	unsigned npoints; // number of vertexes
	float *wallLengths; // pre-calculated wall lengths

	std::vector<b2Body*> wallBodies;
	std::vector<b2Fixture*> wallFixtures;

	std::vector<Actor*> actors; // stores a list of all actors currently in this sector
	// I do this because sprite culling relies on portals

	bool noCeil;
	Texture* ceilingTexture;
	Texture* floorTexture;

	void UpdateActors(float deltaTime, float px, float py);
	void MoveActorToSector(Actor* actor);
	void RemoveActorToSector(Actor* actor);
};

Sector CreateSector(b2World& world, float flr, float cl, Vector2* vertices, int nVerts, int *neighboringSectors, unsigned char xFlags[4]);
void DestroySector(Sector &sect, bool end);
int GetWallTypeFromFixture(Sector& sect, b2Fixture * fixture);