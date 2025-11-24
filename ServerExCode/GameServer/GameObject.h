#pragma once
#include "Grid.h"

class GameObject;
using GameObjectRef = shared_ptr<GameObject>;
using namespace std::chrono;

class Room;

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
	DEAD,
	SPAWN
};

enum class GameObjectCode
{
	NONE,
	PLAYER,
	ENEMY,
	BULLET
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

public:
	//충돌관련
	AABB bounds;
	GridManagerRef currentNode = nullptr;
	float width;
	float height;

public:
	GameObject(RoomRef room, int id, float hp, GameObjectCode objectCode, Vector3 position = Vector3(0, 0, 0), Vector3 direction = Vector3(0, 0, 1), float speed = 0.1f);

	SendBufferRef Update();
	virtual void SetMove(GameObjectState state, Vector3 direction);
	virtual void SetAttack(Vector3 targetPos);
	virtual void SetDead() { currentState = GameObjectState::DEAD; }

	int GetId() { return id;  }
	float GetHp() { return hp; }
	Vector3 GetPosition() { return position; }
	Vector3 GetDirection() { return direction; }
	GameObjectCode GetObjectCode() { return objectCode; }
	float GetDamage() { return damage; }

public:
	virtual SendBufferRef move();
	virtual SendBufferRef spawn();
	virtual SendBufferRef dead();
	virtual void attack();
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