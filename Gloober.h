#pragma once
#include "Actor.h"

class Gloober : public Actor
{
private:
	enum spriteTypes {
		IDLE = 0,
		DEATH0 = 1,
		DEATH1 = 2,
		DEATH2 = 3,
		WALK0 = 4,
		WALK1 = 5,
		WALK2 = 6,
		WALK3 = 7,
		WALK4 = 8,
		WALK5 = 9,
		WALK6 = 10,
		ATTACK0 = 11,
		ATTACK1 = 12,
		ATTACK2 = 13
	};

	float t,moveUpdate = 0;
	float speed = 5.0f;
	float attackRange = 3.0f;
	float attackCheck = 0;
	bool attacking = false;
public:
	Gloober(xyz, Player*trg=nullptr);

	void Damage(int dmg);
	void Update(float deltaTime, float px, float py);
	void UpdateWalkSprite(float deltaTime);

	void MoveTowardTarget();
};

