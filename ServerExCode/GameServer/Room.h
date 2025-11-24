#pragma once
#include "GameObject.h"

class GameSession;
class Job;

using GameSessionRef = shared_ptr<GameSession>;
using GameObjectRef = shared_ptr<GameObject>;
using JobRef = shared_ptr<Job>;

class GridManager;
using GridManagerRef = shared_ptr<GridManager>;

class Room : public std::enable_shared_from_this<Room>
{
	Set<GameSessionRef> _sessions;

	// roomId
	int roomId;

	// Object
	int idCount;
	Set<int> idList;
	int playerCount;

public:
	map<int, GameObjectRef> objects;
	queue<JobRef> jobQueue;
	GridManagerRef grid;
	USE_LOCK;

	Room(int roomId);
	int Add(GameSessionRef session);
	GameObjectRef AddPlayer();
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);
	void Update();
	void ProcessJob();

	GameObjectRef ObjectAdd(GameObjectCode objectId, Vector3 position, Vector3 direction, GameObjectRef shooter = nullptr);
	void ObjectRemove(GameObjectRef object);

	void HandleCollisions();

	int GetRoomId() { return roomId; }
	int GetPlayerCount() { return playerCount; }
};

class Job
{
public:
	USE_LOCK;
	// 오브젝트 상태 변화 시 필요한 Job
	Job(int objectId, Vector3 position, Vector3 direction, GameObjectState state);
	// 오브젝트 생성 작업 시 필요한 Job
	Job(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectState state);
	// 오브젝트 생성, shooter까지 포함한 Job
	Job(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectRef shooter, GameObjectState state);
	~Job();

	int GetObjectId() const { return objectId; }
	GameObjectCode GetObjectCode() const { return objectCode; }
	Vector3 GetPosition() const { return position; }
	Vector3 GetDirection() const { return direction; }
	GameObjectState GetState() const { return state; }
	GameObjectRef GetShooter() { return shooter; }

private:
	int objectId;
	Vector3 position;
	Vector3 direction;
	GameObjectState state;
	GameObjectCode objectCode;
	GameObjectRef shooter;
};
