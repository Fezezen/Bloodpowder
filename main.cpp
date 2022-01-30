#include "main.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <functional>
#include <box2d/box2d.h>
#include <string>

#include "MathHeader.h"
#include "Player.h"
#include "Sector.h"
#include "TextureHandler.h"
#include "MapLoader.h"
#include "Actor.h"
#include "Gloober.h"
#include "Blood.h"
#include "Gun.h"
#include "Pistol.h"
#include "rng.h"

#include "RaycastCallback.h"

#define twoPi 6.28318530718

const int maxSectorDraw = 32; // the maximum recursion a sector can render another sector

int renderType = 0;

std::vector < std::pair<float, std::function<void()>>> delays;

std::vector<Sector> sectors;

SDL_Texture* renderTexture;
Uint32 *pixels;

b2World* worldPtr;

Texture brickWall;
Texture cobbleStone;
Texture blackStone;
Texture sky;

SDL_Texture* hpIcon;

Gun* pistol;

float nextSpawn = 5.0f;
float spawnRate = 5.0f;

void CreateDelay_Main(float delay, std::function<void()> callback)
{
	delays.push_back(
		std::pair<float, std::function<void()>>(tick +delay, callback)
	);
}

float GetTick()
{
	return (float)tick;
}

void MoveActorToSector(Actor * actor, int sect0, int sect1)
{
	// basically swaps an actor between parent sectors
	sectors[sect0].RemoveActorToSector(actor);
	sectors[sect1].MoveActorToSector(actor);
}

void RemoveActorFromSector(Actor * actor, int sect)
{
	sectors[sect].RemoveActorToSector(actor);
}

void AddActor(const char* name, int sect, float x, float y, float z)
{
	if (name == "Blood") 
	{
		Blood *m = new Blood({x,y,z});
		m->cur_sector = sect;
		m->sectors = sectors;
		m->Initalize(*worldPtr);
		sectors[sect].actors.push_back(m);
	}
	else if (name == "Gloober") {
		Gloober *m = new Gloober({ x,y,z }, &player);
		m->cur_sector = sect;
		m->sectors = sectors;
		m->Initalize(*worldPtr);
		sectors[sect].actors.push_back(m);
	}
}

void QueDestroyBody(b2Body * body)
{
	quedBodiesToDestroy.push_back(body);
}

void DamagePlayer(int amount)
{
	pistol->AddAmmo(-amount);
}

/*std::pair<Vector2, Actor*> p_Hitscan(float x, float y, float px, float py, int s)
{
	const Sector* sect = &sectors[s];
	float psin = player.anglesin, pcos = player.anglecos;
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

		if (IntersectLineSegments(x, y, px, py, vx0, vy0, vx1, vy1))
		{
			float nx, ny;
			Intersect(x, y, px, py, vx0, vy0, vx1, vy1, nx, ny);
			float dist = sqrtf((actor->pos.x - nx)*(actor->pos.x - nx) + (actor->pos.z - ny)*(actor->pos.z - ny)); // for whatever teason this is needed
			// otherwise it thinks it's hitting when you're shooting well left
			//if (dist < actor->boundingBox.x*.7) {
				return std::pair<Vector2, Actor*>(Vector2(nx, ny), actor);
			//}
		}
	}

	// check walls
	for (unsigned s = 0; s < sect->npoints; ++s) {
		float vx0 = vert[s + 0].x, vy0 = vert[s + 0].y;
		float vx1 = vert[s + 1].x, vy1 = vert[s + 1].y;

		if (IntersectLineSegments(x, y, px, py, vx0, vy0, vx1, vy1))
		{
			float nx, ny;
			Intersect(x, y, px, py, vx0, vy0, vx1, vy1, nx, ny);

			if (sect->neighbors[s] >= 0) { // portal, can pass
				return p_Hitscan(nx+dx/100, ny+dy/100, nx + dx, ny + dy, sect->neighbors[s]);
			}
			else { // hit wall
				return std::pair<Vector2, Actor*>(Vector2(nx, ny), nullptr);
			}
		}

	}

	return std::pair<Vector2, Actor*>(Vector2(px, py), nullptr);
}*/

std::pair<Vector2, Actor*> p_Hitscan(float x, float y, float px, float py, int s) {
	RaycastCallback callback;
	worldPtr->RayCast(&callback, b2Vec2(x, y), b2Vec2(px, py));

	if (callback.bHit) {
		for (int i = 0; i < sectors.size(); i++) {
			for (int j = 0; j < sectors[i].actors.size(); j++) {
				if (sectors[i].actors[j]->fixture == callback.out_fixture) {
					return std::pair<Vector2, Actor*>(Vector2(callback.out_point.x,callback.out_point.y), sectors[i].actors[j]);
					break;
				}
			}
		}
	}

	return std::pair<Vector2, Actor*>(Vector2(), nullptr);
}

void RelativeMapCoordinatesToAbsoluteOnes(float &X, float &Z) {
	float rtx = (Z)* player.anglecos + (X)* player.anglesin;
	float rtz = (Z)* player.anglesin - (X)* player.anglecos;
	X = rtx + player.pos.x; Z = rtz + player.pos.z;
}

void CeilingFloorScreenCoordinatesToMapCoordinates(float mapY, int screenX, int screenY, float &X, float &Z) 
{
	Z = (mapY)*w_height*vfov / ((h_height - (screenY)) - player.pitch*w_height*vfov);
	X = (Z) * (h_width - (screenX)) / (w_width*hfov);
	RelativeMapCoordinatesToAbsoluteOnes(X, Z);
}

static int ApplyLight(int texture, float light)
{
	int tr = (texture >> 16) & 0xFF;
	int tg = (texture >> 8) & 0xFF;
	int tb = (texture >> 0) & 0xFF;

	tr *= light;
	tg *= light;
	tb *= light;

	return clamp(tr, 0, 255) * 65536
		+ clamp(tg, 0, 255) * 256
		+ clamp(tb, 0, 255);
}

void vline_Texutred(int x, int y1, int y2, struct Scaler ty, unsigned txtx, float lightingScalar, SDL_Renderer* renderer, Texture& texture) { // draws a texture mapped vertical line
	y1 = clamp(y1, 0, w_height - 1);
	y2 = clamp(y2, 0, w_height - 1);

	TextureData t_pixels = texture.pixels;

	for (int y = y1; y < y2; y++)
	{
		unsigned txty = Scaler_Next(&ty);
		//pixels[y*width + x] = texturePixels[txtx % 64][txty % 64];
		//pixels[y*width + x] = ApplyLight(texturePixels[txtx % 64][txty % 64],lightingScalar);
		if (t_pixels[txtx % texture.w][txty % texture.h] == 0x00FFFFFF) continue;
		pixels[y*w_width + x] = ApplyLight(t_pixels[txtx % texture.w][txty % texture.h],lightingScalar);
	}
}

void vline_Solid(int x, int y1, int y2, Uint32 color) // draws a vertical line of a solid colour
{
	y1 = clamp(y1, 0, w_height - 1);
	y2 = clamp(y2, 0, w_height - 1);

	for (int y = y1; y < y2; y++)
	{
		pixels[y*w_width + x] = color;
	}
}

bool DrawWall(float vx0, float vy0, float vx1, float vy1, SDL_Renderer* renderer, int neighbour, float len, portalWnd& now, portalWnd& next, Sector *sect, Texture *wallTexture) { // this function handles drawing walls given their world coordinates 
	 // transform around player
	float pcos = player.anglecos, psin = player.anglesin;
	float px = player.pos.x - pcos*.2f, py = player.pos.z - psin*.2f;

	vx0 -= px;
	vx1 -= px;
	vy0 -= py;
	vy1 -= py;
	float tx1 = vx0 * psin - vy0 * pcos, tz1 = vx0 * pcos + vy0 * psin;
	float tx2 = vx1 * psin - vy1 * pcos, tz2 = vx1 * pcos + vy1 * psin;

	if (tz1 <= 0 && tz2 <= 0) return false;// make sure the wall isn't behind the player

	//float xd = vx1 - vx0, yd = vy1 - vy0;
	float dis = len*14;

	int u0 = 0, u1 = dis;
	if (tz1 <= 0 || tz2 <= 0) {
		// perspective math
		const float nearz = 1e-4f, farz = 5, nearside = 1e-5f, farside = 20.f;

		Vector2 org1 = { tx1,tz1 }, org2 = { tx2,tz2 };

		float ix1, iz1;
		float ix2, iz2;
		Intersect(tx1, tz1, tx2, tz2, -nearside, nearz, -farside, farz, ix1, iz1);
		Intersect(tx1, tz1, tx2, tz2, nearside, nearz, farside, farz, ix2, iz2);
		if (tz1 < nearz) if (iz1 > 0)  tx1 = ix1, tz1 = iz1; else tx1 = ix2, tz1 = iz2;
		if (tz2 < nearz) if (iz1 > 0) tx2 = ix1, tz2 = iz1;	else tx2 = ix2, tz2 = iz2;

		if (abs(tx2 - tx1) > abs(tz2 - tz1)) {
			u0 = (tx1 - org1.x) * dis / (org2.x - org1.x), u1 = (tx2 - org1.x) * dis / (org2.x - org1.x);
		}
		else {
			u0 = (tz1 - org1.y) * dis / (org2.y - org1.y), u1 = (tz2 - org1.y) * dis / (org2.y - org1.y);
		}
	}

	//float xscale1 = hfov / tz1, yscale1 = vfov / tz1; int x1 = h_width - (int)(tx1 * xscale1);
	//float xscale2 = hfov / tz2, yscale2 = vfov / tz2; int x2 = h_width - (int)(tx2 * xscale2);
	float xscale1 = (w_width*hfov) / tz1, yscale1 = (w_height*vfov) / tz1; int x1 = h_width + (int)(-tx1 * xscale1);
	float xscale2 = (w_width*hfov) / tz2, yscale2 = (w_height*vfov) / tz2; int x2 = h_width + (int)(-tx2 * xscale2);
	if (x1 >= x2 || x2 < now.wndX1 || x1 > now.wndX2) return false;

	float yCeil = (sect->ceil - player.pos.y), yFloor = (sect->floor - player.pos.y);
	float ceilDis = yCeil - yFloor;

	int y1a = h_height - (int)(player.Pitch(yCeil, tz1) * yscale1), y1b = h_height - (int)(player.Pitch(yFloor, tz1) * yscale1); // top left point and bottom left
	int y2a = h_height - (int)(player.Pitch(yCeil, tz2) * yscale2), y2b = h_height - (int)(player.Pitch(yFloor, tz2) * yscale2);// top right point and bottom right 

	float nCeil = 5, nFloor = 0;
	if (neighbour >= 0 && neighbour < sectors.size()) {
		nCeil = sectors[neighbour].ceil - player.pos.y;
		nFloor = sectors[neighbour].floor - player.pos.y;
	}
	int ny1a = h_height - (int)(player.Pitch(nCeil,tz1)* yscale1), ny1b = h_height - (int)(player.Pitch(nFloor, tz1) * yscale1);
	int ny2a = h_height - (int)(player.Pitch(nCeil, tz2) * yscale2), ny2b = h_height - (int)(player.Pitch(nFloor, tz2) * yscale2);

	portalWnd nextWnd;

	int xbegin = std::max(x1, now.wndX1), xend = std::min(x2, now.wndX2);

	Scaler ya_int = Scaler_Init(x1, xbegin, x2, y1a, y2a);
	Scaler yb_int = Scaler_Init(x1, xbegin, x2, y1b, y2b);
	Scaler nya_int = Scaler_Init(x1, xbegin, x2, ny1a, ny2a);
	Scaler nyb_int = Scaler_Init(x1, xbegin, x2, ny1b, ny2b);

	int skyX = player.scroll;
	int skyY = (player.pitch * 45);

	for (int x = xbegin; x < xend; x++) {
		int z = ((x - x1) * (float)(tz2 - tz1) / (float)(x2 - x1) + tz1) * 8;
		float d = clamp((100.0f / z),0.0f,1.0f);

		int txtx = (u0*((x2 - x)*tz2) + u1 * ((x - x1)*tz1)) / ((x2 - x)*tz2 + (x - x1)*tz1);

		//int ya = ((float)(y2a - y1a) / (float)(x2 - x1)) * (float)(x - x1) + y1a, cya = clamp(ya,now.yTop[x], now.yBottom[x]);
		//int yb = ((float)(y2b - y1b) / (float)(x2 - x1)) * (float)(x - x1) + y1b, cyb = clamp(yb, now.yTop[x], now.yBottom[x]); 
		int ya = Scaler_Next(&ya_int);
		int yb = Scaler_Next(&yb_int);

		int cya = clamp(ya, now.yTop[x], now.yBottom[x]);
		int cyb = clamp(yb, now.yTop[x], now.yBottom[x]);

		// Draw ceiling
		if (!sect->noCeil)
		{
			Texture *c_tex = sect->ceilingTexture;
			TextureData c_pixels = c_tex->pixels;
			for (int y = now.yTop[x]; y < cya; ++y)
			{
				float hei = y < cya ? yCeil : yFloor;
				float mapx, mapz;
				CeilingFloorScreenCoordinatesToMapCoordinates(hei, x, y, mapx, mapz);
				unsigned txtx = (mapx * c_tex->w)*.5, txtz = (mapz * c_tex->h)*.5;
				int pel = c_pixels[txtz % c_tex->w][txtx % c_tex->h];
				pixels[y*w_width + x] = pel;
			}
		}
		else {
			for (int y = now.yTop[x]; y < cya; ++y)
			{
				int i = abs((x + skyX) % sky.w);
				int iy = abs(y + skyY) % sky.h;
				pixels[y*w_width + x] = sky.pixels[i][iy];
			}
		}

		// Draw floor
		for (int y = cyb; y < now.yBottom[x]; ++y)
		{
			Texture *f_tex = sect->floorTexture;
			TextureData f_pixels = f_tex->pixels;
			float hei = y < cya ? yCeil : yFloor;
			float mapx, mapz;
			CeilingFloorScreenCoordinatesToMapCoordinates(hei, x, y, mapx, mapz);
			unsigned txtx = (mapx * f_tex->w)*.5, txtz = (mapz * f_tex->h)*.5;
			int pel = f_pixels[txtz % f_tex->w][txtx % f_tex->h];
			pixels[y*w_width + x] = pel;
		}

		// Wall
		if (neighbour == -1) { // if not a portal, than render the wall completely
			vline_Texutred(x, cya, cyb, Scaler_Init(ya, cya, yb + 1, 0, ceilDis*5), txtx,d, renderer, *wallTexture); // I added 1 to yb just incase ya==yb
		}
		else {				
			//int nya = (int)((float)(x - x1) * (float)(ny2a - ny1a) / (float)(x2 - x1) + (float)ny1a), cnya = clamp(nya, now.yTop[x], now.yBottom[x]); // do the same as for ya for neighbour
			//int nyb = (int)((float)(x - x1) * (float)(ny2b - ny1b) / (float)(x2 - x1) + (float)ny1b), cnyb = clamp(nyb, now.yTop[x], now.yBottom[x]);
			int nya = Scaler_Next(&nya_int);
			int nyb = Scaler_Next(&nyb_int);
			/* Clamp ya2 & yb2 */
			int cnya = clamp(nya, now.yTop[x], now.yBottom[x]);
			int cnyb = clamp(nyb, now.yTop[x], now.yBottom[x]);

			if (cya < cnya) { // if their ceiling is lower than our ceiling, draw the wall from our ceiling to theirs 
				vline_Texutred(x, cya, cnya, Scaler_Init(ya, cya, yb + 1, 0, ceilDis * 5), txtx,d, renderer, *wallTexture);
			}
			if (cyb > cnyb) { // only draw if their floor is higher than our floor
				vline_Texutred(x, cnyb, cyb, Scaler_Init(ya, cnyb+1, yb + 1, 0, ceilDis * 5), txtx,d, renderer, *wallTexture);
			}

			nextWnd.yTop[x] = clamp(std::max(cya, cnya), now.yTop[x], w_height - 1);
			nextWnd.yBottom[x] = clamp(std::min(cyb, cnyb), 0, now.yBottom[x]);
		}
	}
	nextWnd.wndX1 = xbegin;
	nextWnd.wndX2 = xend;

	if (neighbour >= 0 && xend > xbegin) {
		next = nextWnd; // pass window if there is another sector to be rendered
		return true;
	}
	
	return false;
}

bool CompareActors(Actor* actor0, Actor* actor1)
{
	float sqr0 = (player.pos.x - actor0->pos.x)*(player.pos.x - actor0->pos.x) + (player.pos.z - actor0->pos.z)*(player.pos.z - actor0->pos.z);
	float sqr1 = (player.pos.x - actor1->pos.x)*(player.pos.x - actor1->pos.x) + (player.pos.z - actor1->pos.z)*(player.pos.z - actor1->pos.z);

	return (sqr0 > sqr1);
}

int timesCalled = 0;
void DrawSector(Sector* sect, SDL_Renderer* renderer, portalWnd& now) { // handles drawing sectors
	
	for (unsigned int i = 1; i < sect->npoints; i++) {
		portalWnd next;
		if (DrawWall(sect->vertex[i - 1].x, sect->vertex[i - 1].y, sect->vertex[i].x, sect->vertex[i].y, renderer, sect->neighbors[i - 1], sect->wallLengths[i-1], now, next, sect, &brickWall) && timesCalled < maxSectorDraw) {
			timesCalled++;
			if (sect->neighbors[i - 1] < sectors.size())
				DrawSector(&sectors[sect->neighbors[i - 1]], renderer, next);
		}
	}

	std::sort(sect->actors.begin(), sect->actors.end(), CompareActors);
	float pc = player.anglecos, ps = player.anglesin, px = player.pos.x, py = player.pos.z, ph = player.pos.y, pp = player.pitch;
	for (unsigned int i = 0; i < sect->actors.size(); i++) {
		sect->actors[i]->Draw(renderer, now, pc, ps, px, py, ph, pp);
	}
}

void DrawSectorTopDown(int cur, SDL_Renderer* renderer, std::vector<int>& drawnSectors) { // draws sectors 
	Sector* sect = &sectors[cur];

	float pcos = player.anglecos, psin = player.anglesin;
	float px = player.pos.x, py = player.pos.z;

	drawnSectors.push_back(cur);

	for (unsigned i = 0; i < sect->npoints-1; i++) {
		float vx0 = sect->vertex[i + 0].x - px, vx1 = sect->vertex[i + 1].x - px;
		float vy0 = sect->vertex[i + 0].y - py, vy1 = sect->vertex[i + 1].y - py;
		float tx1 = -10 * (vx0 * psin - vy0 * pcos) + h_width, tz1 = -10 * (vx0 * pcos + vy0 * psin) + h_height;
		float tx2 = -10 * (vx1 * psin - vy1 * pcos) + h_width, tz2 = -10 * (vx1 * pcos + vy1 * psin) + h_height;

		if ((tx1 < 0 || tx1 > w_width) && (tx2 < 0 || tx2 > w_width) && (tz1 < 0 || tz1 > w_height) && (tz2 < 0 || tz2 > w_height))
			continue;

		int neighbor = sect->neighbors[i];
		if (neighbor >= 0) {
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			std::vector<int>::iterator it = std::find(drawnSectors.begin(), drawnSectors.end(), neighbor);
			if (it == drawnSectors.end())
				DrawSectorTopDown(neighbor, renderer, drawnSectors);
			else
				SDL_RenderDrawLine(renderer, tx1, tz1, tx2, tz2);
		}
		else {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

			SDL_RenderDrawLine(renderer, tx1, tz1, tx2, tz2);
		}
	}
}

// Basically just reset the player's health and get rid of the other actors
float playerDeadFor = 0;
bool deathSound = false;
void Restart() {
	player.dead = false;
	deathSound = false;
	
	for (int s = 0; s < sectors.size(); s++) {
		for (int i = 0; i < sectors[i].actors.size(); i++) {
			sectors[i].actors[i]->Destroy(false);
		}
		sectors[s].actors.clear();
	}

	player.angle = 0;
	player.pos.x = 0;
	player.pos.z = 0;
	player.currentEyeHeight = player.eyeHeight;

	pistol->AddAmmo(20);

	spawnRate = 5.0f;
}

int main(int argc, char *argv[]) {
	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	window = SDL_CreateWindow("Bloodpowder", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 720, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, w_width, w_height);

	pixels = new Uint32[w_width*w_height];

	SDL_Event event;
	SDL_SetRelativeMouseMode(SDL_TRUE);

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cout << "Could not open audio" << std::endl;
		return 1;
	}

	brickWall = LoadTexture("Textures/wall.bmp");
	cobbleStone = LoadTexture("Textures/floor.bmp");
	blackStone = LoadTexture("Textures/ceil.bmp");
	sky = LoadTexture("Textures/sky.bmp");

	Mix_Chunk* deathsfx = Mix_LoadWAV("Sounds/scream.wav");

	Pistol p = Pistol(renderer, &player);
	pistol = &p;

	// Load hp icon
	SDL_Surface* s = SDL_ConvertSurfaceFormat(SDL_LoadBMP("Textures/UI/hp.bmp"), SDL_PIXELFORMAT_ABGR8888, 0);
	hpIcon = SDL_CreateTextureFromSurface(renderer, s);
	SDL_FreeSurface(s);

	/*float aspectRatio = 1.33333f;
	float viewScale = 1.0f / tanf(0.5f * 1.22f);
	float magicConstant = h_width + h_width * viewScale / std::min(1.0f, aspectRatio);
	float magicConstant2 = h_height + h_height * viewScale / std::max(1.0f, aspectRatio);*/

	b2World world(b2Vec2_zero);
	worldPtr = &world;

	sectors = MapLoader::Load("Maps/map.txt", world);

	for (unsigned i = 0; i < sectors.size(); i++) {
		//std::cout << "Sector " << i << ": " << sectors[i].vertex[0].x << std::endl;
		for (unsigned j = 0; j < sectors[i].npoints-1; j++) {
			//std::cout << sectors[i].neighbors[j] << ", ";
		}
		sectors[i].ceilingTexture = &blackStone, sectors[i].floorTexture = &cobbleStone; 
	}

	player.cur_sector = 0;
	player.sectors = sectors;
	player.Initalize(world);

	player.pos.x = 0;
	player.pos.z = 0;

	player.angle = 0.0f;

	double lastTime = SDL_GetTicks();
	double deltaTime = 0;

	bool quit = false;
	while (!quit) {
		memset(pixels, 0, w_width * w_height * sizeof(Uint32));

		if (!renderType) {
			portalWnd wnd;
			for (unsigned i = 0; i < w_width; i++) wnd.yTop[i] = 0;
			for (unsigned i = 0; i < w_width; i++) wnd.yBottom[i] = w_height;
			timesCalled = 0;
			DrawSector(&sectors[player.cur_sector], renderer, wnd);

			//actor_t.Draw(renderer, wnd, player.anglecos, player.anglesin, player.pos.x, player.pos.z, player.pos.y, player.pitch);

			SDL_UpdateTexture(renderTexture, NULL, pixels, w_width * sizeof(Uint32));

			// Draw gun
			SDL_SetRenderTarget(renderer, renderTexture);
			
			pistol->Draw();

			// Draw UI
			SDL_Rect r;
			r.x = w_width-230;
			r.y = w_height-40;
			r.w = 200;
			r.h = 20;
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			SDL_RenderDrawRect(renderer, &r);

			r.w = 200*(((float)pistol->GetAmmo())/20.0f);
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			SDL_RenderFillRect(renderer, &r);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			r.x = w_width - 25;
			r.w = 15;
			r.h = 15;
			SDL_RenderCopy(renderer, hpIcon, NULL, &r);

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_SetRenderTarget(renderer, NULL);
			SDL_RenderCopy(renderer, renderTexture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		else {
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // sky
			SDL_RenderClear(renderer);
			std::vector<int> drawnSectors;
			DrawSectorTopDown(player.cur_sector, renderer, drawnSectors);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderDrawLine(renderer, h_width, h_height-10, h_width, h_height+10);

			SDL_RenderPresent(renderer);
		}

		// handle events and input
		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT) {
					pistol->Fire1();
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_RETURN:
					renderType = !renderType;
					break;
				case SDLK_ESCAPE:
					quit = true;
					break;
				default:
					break;
				}
			default:
				player.HandleInput(event, deltaTime);
				break;
			}
		}	

		double startTime = SDL_GetTicks();
		deltaTime = (startTime - lastTime)*0.001;
		lastTime = startTime;
		tick += deltaTime;

		if (!player.dead) {
			if (player.moving) {
				float t = tick * 3;
				pistol->offset = pistol->offset.Lerp(
					Vector2(
						sinf(t) * 50,
						sinf(t)*cosf(t) * 30),
					deltaTime*10.0f
				);
			}
			else {
				pistol->offset = pistol->offset.Lerp(
					Vector2(),
					deltaTime*5.0f
				);
			}

			std::vector<Actor*> s = sectors[player.cur_sector].actors;
			for (int i = 0; i < s.size(); i++) {
				if (s[i]->tag == "Blood") {
					float d = (player.pos.x - s[i]->pos.x)*(player.pos.x - s[i]->pos.x) + (player.pos.z - s[i]->pos.z)*(player.pos.z - s[i]->pos.z);
					if (d < 2.0f) {
						pistol->AddAmmo(3);
						s[i]->Destroy(false);
						sectors[player.cur_sector].RemoveActorToSector(s[i]);
						break;
					}
				}
			}

			// Enemy spawn loob
			nextSpawn -= deltaTime;
			if (nextSpawn <= 0.0f) {
				nextSpawn = spawnRate;
				spawnRate = std::max(spawnRate - .2f, 2.0f);
				
				int r = (int)((Random() / 256.0f) * 3);
				// Rnadomly spawn in one of the 3 sectors 
				switch (r)
				{
				case 0:
					AddActor("Gloober", 8, -1.0f, 26.0f, -25.0f);
					break;
				case 1:
					AddActor("Gloober", 6, -17.0f, 26.0f, -2.75f);
					break;
				case 2:
					AddActor("Gloober", 10, 21.5f, 26.0f, 4.50f);
					break;
				default:
					break;
				}
			}
		}
		else {
			pistol->offset = pistol->offset.Lerp(
				Vector2(0,300),
				deltaTime*5.0f
			);

			playerDeadFor += deltaTime;
		}

		// Update delays
		for (unsigned i = delays.size(); i-- > 0; )
		{
			if (delays[i].first < tick) {
				delays[i].second();
				delays.erase(delays.begin()+i);
			}
		}
		

		if (pistol->GetAmmo() <= 0) { // Ammo in this game is your health
			player.dead = true;
			if (!deathSound) {
				deathSound = true;
				Mix_VolumeChunk(deathsfx, 10);
				Mix_PlayChannel(-1, deathsfx, 0);
			}
		}

		player.Update(deltaTime);
		for (int i = 0; i < sectors.size(); i++) 
		{
			sectors[i].UpdateActors(deltaTime,player.pos.x,player.pos.z);
		}

		world.Step(deltaTime, 6, 2);
		//player.pos.y = sectors[player.sector].floor + eyeHeight;		

		//std::cout << 1.0 / deltaTime << std::endl;

		std::vector<b2Body*>::iterator it = quedBodiesToDestroy.begin();
		std::vector<b2Body*>::iterator end = quedBodiesToDestroy.end();
		for (; it != end; ++it) {
			b2Body* b = *it;

			world.DestroyBody(b);
			b = nullptr;
		}
		quedBodiesToDestroy.clear();

		if (playerDeadFor > 5) {
			playerDeadFor = 0;
			Restart();
		}
	}

	//End stuff
	Mix_FreeChunk(deathsfx);

	for (int i = 0; i < sizeof(sectors) / sizeof(Sector); i++)
		DestroySector(sectors[i],true);
	//sectors = NULL;
	//delete sectors;

	delete[] pixels;

	pistol->Destroy();

	DestroyTexture(brickWall);
	DestroyTexture(cobbleStone);
	DestroyTexture(blackStone);
	DestroyTexture(sky);

	SDL_DestroyTexture(hpIcon);

	SDL_DestroyTexture(renderTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	Mix_Quit();
	SDL_Quit();

	return 0;
}