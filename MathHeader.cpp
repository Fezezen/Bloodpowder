#include "MathHeader.h"
#include <iostream>
#include "sector.h"
#include "Actor.h"

Vector2::Vector2()
{
	x = 0, y = 0;
}

Vector2::Vector2(float x_, float y_)
{
	x = x_, y = y_;
}

Vector2 Vector2::operator+(const Vector2 &v)
{
	return {x+v.x, y+v.y};
}

Vector2 Vector2::operator-(const Vector2 &v)
{
	return { x - v.x, y - v.y };
}

Vector2 Vector2::operator*(const float &s)
{
	return { x * s, y * s};
}

Vector2 Vector2::operator/(const float &s)
{
	return { x / s,y / s };
}

Vector2 Vector2::Lerp(const Vector2 &b, float a)
{
	return Vector2(
		x + (b.x - x) * a,
		y + (b.y - y) * a
	);
}

std::pair<Vector2, Actor*> Hitscan(float x, float y, float px, float py, float pcos, float psin, Sector* sect, std::vector<Sector> sectors)
{
	const Vector2* vert = sect->vertex;
	float dx = (px - x), dy = (py - y);

	// check actors
	std::vector<Actor*> actors = sect->actors;
	
	for (unsigned a = 0; a < actors.size(); a++) {
		Actor* actor = actors[a];
		if (actor->dead) continue;
		float vx0 = actor->pos.x + psin * actor->half_w;
		float vy0 = actor->pos.z - pcos * actor->half_w;
		float vx1 = actor->pos.x - psin * actor->half_w;
		float vy1 = actor->pos.z + pcos * actor->half_w;

		if (IntersectLineSegments(x,y,px,py,vx0,vy0,vx1,vy1))
		{
			float nx, ny;
			Intersect(x, y, px, py, vx0, vy0, vx1, vy1, nx, ny);
			float dist = sqrtf((actor->pos.x - nx)*(actor->pos.x - nx) + (actor->pos.z - ny)*(actor->pos.z - ny)); // for whatever teason this is needed
			// otherwise it thinks it's hitting when you're shooting well left
			if (dist < actor->boundingBox.x*.7) {
				return std::pair<Vector2, Actor*>(Vector2(nx,ny),actor);
			}
		}
	}
	
	// check walls
	for (unsigned s = 0; s < sect->npoints-1; ++s) {
		float vx0 = vert[s + 0].x, vy0 = vert[s + 0].y;
		float vx1 = vert[s + 1].x, vy1 = vert[s + 1].y;

		if (IntersectBox(x, y, px, py, vx0, vy0, vx1, vy1)
			&& PointSide(px, py, vx0, vy0, vx1, vy1) < 0)
		{
			float nx, ny;
			Intersect(x, y, px, py, vx0, vy0, vx1, vy1, nx, ny);

			if (sect->neighbors[s] >= 0) { // portal, can pass
				return Hitscan(nx+dx/100, ny+dy/100, nx + dx, ny + dy, pcos, psin, &sectors[sect->neighbors[s]], sectors);
				break;
			}
			else { // hit wall
				return std::pair<Vector2, Actor*>(Vector2(nx, ny), nullptr);
			}
		}

	}

	return std::pair<Vector2, Actor*>(Vector2(px, py), nullptr);
}
