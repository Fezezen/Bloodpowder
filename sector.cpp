#include"Sector.h"
#include<math.h>
#include<iostream>
#include"CollisionTypes.h"
#include "Actor.h"

Sector CreateSector(b2World& world, float flr, float cl, Vector2* vertices, int nVerts, int * neighboringSectors, unsigned char xFlags[4])
{
	Sector s = { flr,cl,vertices,neighboringSectors, nVerts};
	
	for (unsigned int i = 0; i < 4; i++) { // run through flags
		switch (xFlags[i]) {
		case '1': // no ceiling
			s.noCeil = true;
			break;
		default:
			break;
		}
	}

	s.wallLengths = new float[s.npoints-1];
	for (int i = 1; i < s.npoints; i++) {
		float xd = s.vertex[i].x - s.vertex[i-1].x;
		float yd = s.vertex[i].y - s.vertex[i-1].y;
		float length = sqrtf(xd*xd+yd*yd);
		s.wallLengths[i - 1] = length;
		
		// We create it for every wall because some times portals are too high for the player
		b2BodyDef wallDef;
		wallDef.position.Set( // set its possition to the midpoint of the two vertexes
			(s.vertex[i - 1].x + s.vertex[i].x) / 2.0f,
			(s.vertex[i - 1].y + s.vertex[i].y) / 2.0f
		);
		
		b2Body* wallBody = world.CreateBody(&wallDef);
		b2PolygonShape wallPoly;
		wallPoly.SetAsBox(length/2, 0.2f, b2Vec2_zero, atan2f(yd, xd));//For some reason box2d incorrectly gets the width, so I had to divide the length by 2
		// the angle is calculated with atan2 and the vector between v1 - v0
		b2FixtureDef fixDef;
		fixDef.shape = &wallPoly;
		fixDef.density = 0.0f;// No density for static body
		if (s.neighbors[i - 1] < 0) {
			fixDef.filter.categoryBits = CollisionTypes::WALL;
			fixDef.filter.maskBits = CollisionTypes::PLAYER | CollisionTypes::ACTOR;
		}
		else {
			fixDef.filter.categoryBits = CollisionTypes::PORTAL;
			fixDef.filter.groupIndex = -1; // set to not collide with
		}

		s.wallFixtures.push_back(wallBody->CreateFixture(&fixDef));
		s.wallBodies.push_back(wallBody); // add to vector
	}
	return s;
}

void DestroySector(Sector &sect, bool end) {
	for (int i = 0; i < sect.actors.size(); i++) {
		sect.actors[i]->Destroy(end);
	}
	sect.actors.clear();

	delete[] sect.wallLengths;
}

int GetWallTypeFromFixture(Sector& sect, b2Fixture * fixture)
{
	auto begin = sect.wallFixtures.begin();
	for (auto it = begin; it != sect.wallFixtures.end(); it++) {
		int index = std::distance(begin, it);
		if (*it == fixture) {
			return sect.neighbors[index];
		}
	}

	return -1;
}

void Sector::MoveActorToSector(Actor * actor)
{
	actors.push_back(actor);
}

void Sector::RemoveActorToSector(Actor * actor)
{
	auto it = std::find(actors.begin(), actors.end(), actor);
	if (it != actors.end()) {
		actors.erase(it);
	}
}

void Sector::UpdateActors(float deltaTime, float px, float py)
{
	for (int i = 0; i < actors.size(); i++) {
		actors[i]->Update(deltaTime, px, py);
	}
}
