#pragma once
#include<math.h>
#include<algorithm>
#include<vector>
#include<utility>

struct Sector;
class Actor;

const float twoPi = 6.283185;

template<typename T>
T FNcross(T x0, T y0, T x1, T y1) {
	return x0 * y1 - y0 * x1;
}

template<typename T>
T clamp(T x, T min, T max) {
	if (x < min) return min;
	else if (x > max) return max;
	return x;
}

template<typename T>
T PointSide(T px, T py, T x0, T y0, T x1, T y1) {
	return FNcross(x1 - x0, y1 - y0, px - x0, py - y0);
}

template<typename T>
T Overlap(T a0, T a1, T b0, T b1) {
	return (std::min(a0, a1) <= std::max(b0, b1) && std::min(b0, b1) <= std::max(a0, a1));
}

template<typename T>
T IntersectBox(T x0, T y0, T x1, T y1, T x2, T y2, T x3, T y3) {
	return (Overlap(x0, x1, x2, x3) && Overlap(y0, y1, y2, y3));
}

template<typename T>
void Intersect(T x1, T y1, T x2, T y2, T x3, T y3, T x4, T y4, T &x, T &y) {
	/*x = FNcross(x1, y1, x2, y2);
	y = FNcross(x3, y3, x4, y4);
	T det = FNcross(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
	x = FNcross(x, x1 - x2, y, x3 - x4) / det;
	y = FNcross(x, y1 - y2, y, y3 - y4) / det;*/

	x = FNcross(FNcross(x1, y1, x2, y2), (x1)-(x2), FNcross(x3, y3, x4, y4), (x3)-(x4)) / FNcross((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4));
	y = FNcross(FNcross(x1, y1, x2, y2), (y1)-(y2), FNcross(x3, y3, x4, y4), (y3)-(y4)) / FNcross((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4));
}

template<typename T>
void IntersectPoint(T x1, T y1, T x2, T y2, T x3, T y3, T x4, T y4, T &x, T &y) {
	T dx = x2 - x1;
	T dy = y2 - y1;
	T m1 = dy / dx;
	T c1 = y1 - m1 * x1;

	dx = x4 - x3;
	dy = y4 - y3;
	T m2 = dy / dx;

	T c2 = y3 - m2 * x3;

	x = (c2 - c1) / (m1 - m2);
	y = m1 * x + c1;
}

static int IntersectLineSegments(float x0, float y0, float x1, float y1,
	float x2, float y2, float x3, float y3)
{
	return IntersectBox(x0, y0, x1, y1, x2, y2, x3, y3)
		&& ((int)abs(PointSide(x2, y2, x0, y0, x1, y1) + PointSide(x3, y3, x0, y0, x1, y1))) != 2
		&& ((int)abs(PointSide(x0, y0, x2, y2, x3, y3) + PointSide(x1, y1, x2, y2, x3, y3))) != 2;
}


struct Vector2
{
	float x, y;

	Vector2();
	Vector2(float, float);

	Vector2 operator+(const Vector2&);
	Vector2 operator-(const Vector2&);
	Vector2 operator*(const float&);
	Vector2 operator/(const float&);

	Vector2 Lerp(const Vector2&, float a);
};

struct Scaler { int result, bop, fd, ca, cache; };

static Scaler Scaler_Init(int a, int b, int c, int d, int f)
{
	return { // I actually have no clue what this does
		d + (b - 1 - a) * (f - d) / (c - a), ((f < d) ^ (c < a)) ? -1 : 1, \
		abs(f - d), abs(c - a), (int)((b - 1 - a) * abs(f - d)) % abs(c - a) 
	};
}

// Scaler_Next: Return (b++ - a) * (f-d) / (c-a) + d using the initial values passed to Scaler_Init().
static int Scaler_Next(Scaler* i)
{
	for (i->cache += i->fd; i->cache >= i->ca; i->cache -= i->ca) i->result += i->bop;
	return i->result;
}


std::pair<Vector2, Actor*> Hitscan(float x, float y, float px, float py, float pcos, float psin, Sector* sect, std::vector<Sector> sectors);