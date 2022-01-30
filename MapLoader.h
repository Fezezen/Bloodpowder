#pragma once
#include <vector>
#include <string>
#include "MathHeader.h"
#include <box2d/box2d.h>

struct Sector;

namespace MapLoader {
	std::vector<Sector> Load(const char* dir, b2World& world);
	
	Vector2 LoadVertex(std::string line); // constructs a vertex from string
	Sector LoadSector(std::string line, std::vector<Vector2>& v, b2World& world); // constructs a sector from string data
}