#include "MapLoader.h"
#include <iostream>
#include <fstream>
#include "sector.h"

std::vector<Sector> MapLoader::Load(const char * dir, b2World& world)
{
	std::vector<Sector> sectors;
	std::vector<Vector2> vertexes;

	std::ifstream file;
	file.open(dir,std::ios::in);
	std::string line;

	if (file.is_open()) {
		while (std::getline(file, line)) {
			//std::cout << line << std::endl;
			if (line == "end")
				break;
			if (!line.empty())
				switch (line[0])
				{
				case 'v': // vertex
					vertexes.push_back(LoadVertex(line.substr(2))); // sub2 to get rid of the v and space
					break;
				case 's': // sector
					sectors.push_back(LoadSector(line.substr(2), vertexes, world));
					break;
				default:
					break;
				}
		}
		file.close();
	}

	//std::cout << sectors.size() << std::endl;

	return sectors;
}

Vector2 MapLoader::LoadVertex(std::string line)
{
	// Format: x y

	Vector2 v; // vertex
	std::string curNumber; // curent number string
	for (unsigned int i = 0; i < line.length(); i++) { // go through each character
		char c = line[i];
		switch (c)
		{
		case 32: // space means it's going to be the next number
			v.x = std::stof(curNumber);

			curNumber.clear(); // make sure to clear it so it doesn't act funny

			break;
		default:
			curNumber += c;
			break;
		}
	}
	
	
	v.y = std::stof(curNumber);

	//std::cout << v.x << ", " << v.y << std::endl;
	return v;
}

Sector MapLoader::LoadSector(std::string line, std::vector<Vector2>& v, b2World& world)
{
	// Format: floor, ceiling\t(vertex indexes)\t(neighbor indexes)
	float floorHeight, ceilingHeight;
	std::vector<Vector2> vertexes;
	std::vector<int> neighbors;

	bool noCeiling = false;

	std::string curNumber; // current data
	char mode = 'f';

	unsigned char xFlags[4] = {0,0,0,0}; // various flags that can be set
	unsigned int f = 0;

	for (unsigned int i = 0; i < line.length(); i++) { // go through each character
		char c = line[i];
		int index;
		switch (mode)
		{
		case 'f': // floor
			switch (c)
			{
			case 32: // space means it's going to be the next number
				floorHeight = std::stof(curNumber);
				curNumber.clear(); // make sure to clear it so it doesn't act funny
				mode = 'c';
				break;
			default:
				curNumber += c;
				break;
			}
			break;
		case 'c': // ceiling
			switch (c)
			{
			case '\t': // space means it's going to be the next number
				ceilingHeight = std::stof(curNumber);
				curNumber.clear(); // make sure to clear it so it doesn't act funny
				i++; // skip next tab
				mode = 'v';
				break;
			default:
				curNumber += c;
				break;
			}
			break;
		case 'v': // verts
			switch (c)
			{
			case '\t':
				i++;
				mode = 'n';
			case 32: // space means it's going to be the next number
				index = std::stoi(curNumber);
				curNumber.clear(); // make sure to clear it so it doesn't act funny
				//std::cout << index;
				vertexes.push_back(v[index]); // add vertex to list

				break;
			default:
				curNumber += c;
				break;
			}
			break;
		case 'n': // neis
			switch (c)
			{
			case 'x':
				curNumber.clear(); // make sure to clear it so it doesn't act funny
				neighbors.push_back(-1); // add index to list
				i++;
				break;
			case '\t':
				mode = 'k';
				i++;
				break;
			case 32: // space means it's going to be the next number
				if (isdigit(curNumber[0])) {
					index = std::stoi(curNumber);
					neighbors.push_back(index); // add index to list
				}

				curNumber.clear(); // make sure to clear it so it doesn't act funny

				break;
			default:
				if (isdigit(c)) {
					curNumber += c;
				}
				break;
			}
			break;
		case 'k': // flags
			if (isxdigit(c)) {
				xFlags[f] = c;
			}
			f++;
			break;
		default:
			break;
		}
	}

	if (!curNumber.empty() && isdigit(curNumber[0])) {
		int index = std::stoi(curNumber);
		curNumber.clear(); // make sure to clear it so it doesn't act funny
		neighbors.push_back(index); // add index to list
	}

	vertexes.push_back(vertexes.at(0));

	Vector2 *verArr = new Vector2[vertexes.size()];
	std::copy(vertexes.begin(), vertexes.end(), verArr);

	int *nArr = new int[neighbors.size()];
	std::copy(neighbors.begin(), neighbors.end(), nArr);

	//std::cout << floorHeight << ", " << ceilingHeight << std::endl;

	//return CreateSector(world, floorHeight, ceilingHeight, &vertexes[0], vertexes.size()-1, &neighbors[0]);
	return CreateSector(world, floorHeight, ceilingHeight, verArr, vertexes.size(), nArr, xFlags);
}