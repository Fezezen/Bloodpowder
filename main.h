#pragma once
#include <vector>
#include <functional>
#include "Sector.h"
#include <box2d/box2d.h>

#define w_width 640
#define w_height 480

#define h_width (w_width/2)
#define h_height (w_height/2)

static double tick = 0;

const float hfov = (0.73f*w_height / w_width);
const float vfov = (0.2f);

struct portalWnd {//defines a polygon "portal" from which bounds rendering of a sector
	int wndX1 = 0, wndX2 = w_width;// min and max left and right
	int yTop[w_width];// min window y per column
	int yBottom[w_width];// max window y per column
};

void vline_Solid(int x, int y1, int y2, Uint32 color); // draws a vertical line of a solid colour

void vline_Texutred(int x, int y1, int y2, struct Scaler ty, unsigned txtx, float lightingScalar, SDL_Renderer* renderer, Texture& texture);

void CreateDelay_Main(float delay, std::function<void()> callback);

float GetTick();

void MoveActorToSector(Actor* actor, int sect0, int sect1);
void RemoveActorFromSector(Actor* actor, int sect);

void AddActor(const char*,int sect, float x, float y, float z);

static std::vector<b2Body*> quedBodiesToDestroy;
void QueDestroyBody(b2Body* body);

void DamagePlayer(int amount);

std::pair<Vector2, Actor*> p_Hitscan(float x, float y, float px, float py, int s);