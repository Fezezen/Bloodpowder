#include "Gloober.h"
#include <algorithm>
#include <string>
#include <sstream>
#include "main.h"
#include<iostream>
#include "player.h"
#include "Blood.h"
#include "CollisionTypes.h"

std::string int2str(int x) {
	std::stringstream ss;
	ss << x;
	return ss.str();
}

Gloober::Gloober(xyz p, Player* trg):Actor(p,trg)
{
	height = 9.0f;
	width = 4.0f;
	half_w = width / 2;
	health = 50;

	tag = "Gloober";

	SpritePack idlePack;
	for (int i = 0; i < 8; i++) {
		std::string str = "Textures/Monsters/Gloober/idle/idle" + int2str(i) + ".bmp";
		idlePack.a[i] = LoadTexture(str.c_str());
	}

	SpritePack death0Pack;
	for (int i = 0; i < 8; i++) {
		std::string str = "Textures/Monsters/Gloober/death/death0" + int2str(i) + ".bmp";
		death0Pack.a[i] = LoadTexture(str.c_str());
	}

	SpritePack death1Pack;
	for (int i = 0; i < 8; i++) {
		std::string str = "Textures/Monsters/Gloober/death/death1" + int2str(i) + ".bmp";
		death1Pack.a[i] = LoadTexture(str.c_str());
	}

	SpritePack death2Pack;
	for (int i = 0; i < 8; i++) {
		std::string str = "Textures/Monsters/Gloober/death/death2" + int2str(i) + ".bmp";
		death2Pack.a[i] = LoadTexture(str.c_str());
	}

	SpritePack walkPack[7];
	for (int j = 0; j < 7; j++) {
		for (int i = 0; i < 8; i++) {
			std::string str = "Textures/Monsters/Gloober/walk/walk" + int2str(j) + int2str(i) + ".bmp";
			walkPack[j].a[i] = LoadTexture(str.c_str());
		}
	}

	SpritePack attackPack[3];
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 8; i++) {
			std::string str = "Textures/Monsters/Gloober/attack/attack" + int2str(j) + int2str(i) + ".bmp";
			attackPack[j].a[i] = LoadTexture(str.c_str());
		}
	}

	sprites.push_back(idlePack);
	sprites.push_back(death0Pack);
	sprites.push_back(death1Pack);
	sprites.push_back(death2Pack);
	for (int j = 0; j < 7; j++) {
		sprites.push_back(walkPack[j]);
	}
	for (int j = 0; j < 3; j++) {
		sprites.push_back(attackPack[j]);
	}
	currentSprite = IDLE;
}

void Gloober::Damage(int dmg)
{
	health = std::max(health - dmg, 0);
	if (health == 0 && !dead) {
		dead = true;

		currentSprite = DEATH0;
		ChangeAnimationState(.2f, DEATH1, DEATH0);
		ChangeAnimationState(.3f, DEATH2, DEATH1);
		velocity.x = 0;
		velocity.z = 0;

		AddActor("Blood", cur_sector, pos.x, pos.y, pos.z);

		b2Filter filter = fixture->GetFilterData();
		filter.maskBits = 0;
		filter.categoryBits = DEAD;
		fixture->SetFilterData(filter);

		/*if (body != nullptr) {
			world->DestroyBody(body);
			body = nullptr;
		}*/
	}
}

void Gloober::Update(float deltaTime, float px, float py)
{
	if (target != nullptr && !dead && !attacking) {
		moveUpdate -= deltaTime;
		if (moveUpdate <= 0)
		{
			moveUpdate = 1;
			if (!target->dead)
				MoveTowardTarget();
			else
				velocity.x = 0, velocity.z = 0;
		}

		if (abs(velocity.x) > 0 || abs(velocity.z) > 0)
			UpdateWalkSprite(deltaTime);
	}

	Actor::Update(deltaTime, px, py);

	if (deadTimer > 20) {
		Destroy(false);
		RemoveActorFromSector(this, cur_sector);
	}

	if (attackCheck <= 0 && !dead) {
		if (!attacking) {
			attackCheck = 2.0f;
			// check if player is in range
			float dx = target->pos.x - pos.x, dy = target->pos.z - pos.z;
			float dist = sqrtf(dx*dx + dy * dy);
			if (dist < attackRange) {
				attacking = true;
				velocity.x = 0, velocity.z = 0;
				currentSprite = ATTACK0;
				ChangeAnimationState(.2f, ATTACK1, ATTACK0);
				ChangeAnimationState(.3f, ATTACK2, ATTACK1);
				ChangeAnimationState(.4f, IDLE, ATTACK2);
				CreateDelay_Main(.3f, [this]() {
					attacking = false;
					if (target == nullptr) return;
					float dx = target->pos.x - pos.x, dy = target->pos.z - pos.z;
					float dist = sqrtf(dx*dx + dy * dy);
					if (dist < attackRange) {
						DamagePlayer(4);
					}
				});
			}
		}
	}
	else {
		if (!attacking) {
			attackCheck -= deltaTime;
		}
	}
}

void Gloober::UpdateWalkSprite(float deltaTime)
{
	if (dead) return;
	if (currentSprite < WALK0)
		currentSprite = WALK0;

	t += deltaTime;
	if (t > 0.1f) {
		t = 0;
		currentSprite++;
		if (currentSprite > WALK6)
			currentSprite = WALK0;

	}
}

void Gloober::MoveTowardTarget()
{
	float dx = (target->pos.x - pos.x), dy = (target->pos.z - pos.z);
	float m = sqrtf(dx*dx + dy * dy);
	dx /= m;
	dy /= m;

	velocity.x = dx*speed;
	velocity.z = dy*speed;

	angle = -atan2f(dy, dx)+twoPi/2;
}
