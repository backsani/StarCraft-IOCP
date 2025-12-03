#include "pch.h"
#include "Unit.h"
#include "Room.h"

Unit::Unit(RoomRef room, int objectId, float hp, Vector3 pos, GameObjectCode objectCode, float speed, Race race, float cost, float population, float produceTime) : GameObject(room, objectId, hp, objectCode, pos, { 0,0,0 }, speed, race)
{
	this->cost = cost;
	this->population = population;
	this->produceTime = produceTime;
	lastAttackTime = 0;
}

void Unit::SetAttack(UnitRef target)
{
	currentState = GameObjectState::ATTACK;
	directTarget = target;
}

void Unit::attack()
{
	if (lastAttackTime + attackCooldown >= room->GetRoomTime())
		return;

	if (Ranged)
	{
		float closestX = Clamp(position.x, directTarget->bounds.minX, directTarget->bounds.maxX);
		float closestY = Clamp(position.y, directTarget->bounds.minY, directTarget->bounds.maxY);

		float dx = position.x - closestX;
		float dy = position.y - closestY;
		float distSq = dx * dx + dy * dy;

		if (distSq <= (attackRange * attackRange))
		{
			// 공격 오브젝트 소환
		}
	}
	else
	{
		if (bounds.Collider(directTarget->bounds))
		{
			lastAttackTime = room->GetRoomTime();
			directTarget->TakeDamage(ad);
		}
	}
}

Mineral::Mineral(RoomRef room, int objectId, Vector3 pos) : GameObject(room, objectId, hp, objectCode, pos, {0,0,0}, 0, Race::NONE)
{
	hp = 1500.f;
	objectCode = GameObjectCode::MINERAL;
	width = 0.64f;
	height = 0.54f;
}

bool Mineral::TryMining(int workerId)
{
	GameObjectRef worker = room->objects[workerId];
	if (occpied != nullptr)
	{
		WaitingWorker[worker] = workerId;
		return false;
	}

	occpied = worker;

	return true;
}

void Mineral::RemoveWorker(int workerId)
{
	GameObjectRef worker = room->objects[workerId];

	if (occpied == worker)
		occpied = nullptr;
	else
	{
		auto It = WaitingWorker.find(worker);
		WaitingWorker.erase(It);
	}
}

Gas::Gas(RoomRef room, int objectId, Vector3 pos) : GameObject(room, objectId, hp, objectCode, pos, { 0,0,0 }, 0, Race::NONE)
{
	hp = 1500.f;
	objectCode = GameObjectCode::GAS;
	width = 0.64f;
	height = 0.32f;
}
