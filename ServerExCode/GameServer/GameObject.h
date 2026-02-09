#pragma once
#include "Grid.h"
#include <functional>

class GameObject;
using GameObjectRef = shared_ptr<GameObject>;
using namespace std::chrono;

class Room;

float Clamp(float v, float minV, float maxV);


struct Vector3
{
	float x;
	float y;
	float z;

	Vector3() : x(0), y(0), z(0) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vector3 operator+(const Vector3& other) const
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	Vector3& operator+=(const Vector3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vector3 operator-(const Vector3& other) const
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	Vector3& operator-=(const Vector3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vector3& operator=(const Vector3& other)
	{
		if (this == &other)
			return *this;

		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	Vector3 operator*(float amount) const
	{
		return Vector3(x * amount, y * amount, z * amount);
	}
}; 

enum class GameObjectState
{
	IDLE,
	MOVE,
	ATTACK,
	MINING,
	DEAD,
	SPAWN
};

enum class GameObjectCode : INT32
{
	NONE,
	PLAYER,
	ENEMY,
	BULLET,
	MINERAL,
	GAS,
	PROBE,
	ZEALOT,
	DARKTEMPLAR,
	DRAGOON,
	REAVER,
	SHUTTLE,
	SCOUT,
	ARBITER,
	ARCHON,
	DARKARCHON,
	OBSERVER,
	CARRIER,
	INTERCEPTOR,
	CORSAIR,
	HIGHTEMPLAR,
	NEXUS,
	PYLON,
	ASSIMILATOR,
	GATEWAY,
	FORGE,
	PHOTON_CANNON,
	CYBERNETICS_CORE,
	SHIELD_BATTERY,
	ROBOTICS_FACILITY,
	STARGATE,
	CITADEL_OF_ADUN,
	ROBOTICS_SUPPORT_BAY,
	FLEET_BEACON,
	TEMPLAR_ARCHIVES,
	OBSERVATORY,
	ARBITER_TRIBUNAL,
};

using SpawnFunc = std::function<GameObjectRef(RoomRef, int, Vector3)>;

extern SpawnFunc g_spawners[];

enum class Race
{
	NONE,
	PROTOSS,
	TERRAN,
	ZERG,
};

class GameObject
{
protected:
	RoomRef room;
	Vector3 position;
	Vector3 direction;
	int id;
	float hp;
	float speed;
	float damage;
	GameObjectState currentState;
	GameObjectCode objectCode;
	int64_t spawnTime;
	Vector3 moveTarget;

	Race race;

	std::vector<std::pair<int, int>> path;

public:
	//충돌관련
	AABB bounds;
	GridManagerRef currentNode = nullptr;
	float width;
	float height;
	int ownerId;

public:
	GameObject(RoomRef room, int id, float hp, GameObjectCode objectCode, Vector3 position = Vector3(0, 0, 0), Vector3 direction = Vector3(0, 0, 1), float speed = 0.1f, Race race = Race::NONE);

	SendBufferRef Update();
	virtual void SetMove(GameObjectState state, Vector3 position, Vector3 target);
	virtual void SetAttack(Vector3 targetPos);
	virtual void SetDead() { currentState = GameObjectState::DEAD; }

	int GetId() const { return id;  }
	float GetHp() const { return hp; }
	Vector3 GetPosition() const { return position; }
	void SetPosition(Vector3 pos) { position = pos; }
	Vector3 GetDirection() const { return direction; }
	GameObjectCode GetObjectCode() const { return objectCode; }
	float GetDamage() { return damage; }
	Vector3 GetMoveTarget() const { return moveTarget; }

	virtual void attack();

	Race GetRace() const { return race; }

public:
	virtual SendBufferRef move();
	virtual SendBufferRef spawn();
	virtual SendBufferRef dead();
	void UpdateBounds();

	void TakeDamage(float damage);
};

/*------------------------------------------
******************Player********************
------------------------------------------*/

class Player : public GameObject
{
public:
	Player(RoomRef room, int id, Vector3 position);
	virtual ~Player();

};

/*------------------------------------------
******************Bullet********************
------------------------------------------*/

class Bullet : public GameObject
{
	GameObjectRef shooter;

	// 1초에 40틱
	int lifeTick;
public:
	Bullet(RoomRef room, int id, Vector3 position, Vector3 direction, GameObjectRef shooter);
	
	SendBufferRef spawn() override;
	SendBufferRef move() override;
	virtual ~Bullet();

	bool IsShooter(GameObjectRef other) { return other == shooter; }
};