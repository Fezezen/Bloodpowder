#include "RaycastCallback.h"
#include "CollisionTypes.h"

RaycastCallback::RaycastCallback()
{
	bHit = false;
}

float RaycastCallback::ReportFixture(b2Fixture * fixture, const b2Vec2 & point, const b2Vec2 & normal, float fraction)
{
	bHit = true;
	out_fixture = fixture;
	out_point = point;
	out_normal = normal;
	out_fraction = fraction;

	b2Filter filter = fixture->GetFilterData();
	if (filter.categoryBits == CollisionTypes::PORTAL || filter.categoryBits == CollisionTypes::PLAYER || filter.categoryBits == CollisionTypes::DEAD)
		return -1;
	else if (filter.categoryBits == CollisionTypes::WALL)
		return 0;
	else if (filter.categoryBits == CollisionTypes::ACTOR) {
		return 0;
	}
	else
		return 1;
}
