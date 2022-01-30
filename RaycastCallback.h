#pragma once
#include <box2d/box2d.h>

class RaycastCallback : public b2RayCastCallback
{
public:
	RaycastCallback();
	virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction);

	bool bHit;
	b2Fixture* out_fixture = nullptr;
	b2Vec2 out_point;
	b2Vec2 out_normal;
	float out_fraction;
};

