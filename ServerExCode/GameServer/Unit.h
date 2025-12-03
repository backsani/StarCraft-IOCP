#pragma once
#include "GameObject.h"
#include "Building.h"

class Unit : public GameObject
{
protected:
	float attackRange;
	float attackCooldown;
	long long lastAttackTime;

	float ad;
	float df;
	bool fly;

	float cost;
	float population;
	float produceTime;

	bool Ranged;

	UnitRef directTarget;

public:
	Unit(RoomRef room, int objectId, float hp, Vector3 pos,  GameObjectCode objectCode, float speed, Race race, float cost, float population, float produceTime);
	
	virtual void SetAttack(UnitRef target);

	void attack() override;
};


class Mineral : public GameObject
{
protected:
	float miningTime;
	GameObjectRef occpied;
	map<GameObjectRef, int> WaitingWorker;
	float startMiningTime;

public:
	Mineral(RoomRef room, int objectId, Vector3 pos);

	bool TryMining(int workerId);
	void RemoveWorker(int workerId);
};

class Gas : public GameObject
{
protected:
	float miningTime;
	BuildingRef build;
	GameObjectRef occpied;

public:
	Gas(RoomRef room, int objectId, Vector3 pos);


};