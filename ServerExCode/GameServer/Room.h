#pragma once
#include "GameObject.h"
#include "MapMaker.h"
#include <functional>

class GameSession;
class Job;

using GameSessionRef = shared_ptr<GameSession>;
using GameObjectRef = shared_ptr<GameObject>;
using JobRef = shared_ptr<Job>;

class GridManager;
using GridManagerRef = shared_ptr<GridManager>;

using SpawnFunc = std::function<GameObjectRef(RoomRef, int, Vector3)>;
extern SpawnFunc g_spawners[];

struct IngamePlayerInfo
{
	PlayerStartPos spos;
	int playerIndex;

};

class Room : public std::enable_shared_from_this<Room>
{
	Set<GameSessionRef> _sessions;

	// roomId
	int roomId;

	// Object
	int idCount;
	Set<int> idList;
	int playerCount;

	long long currentRoomTime;

public:
	map<GameSessionRef, int> _sessionPlayers;

	// ingame에서 플레이어의 id
	map<GameSessionRef, int> ingamePlayerIndex;

	map<int, GameObjectRef> objects;
	queue<JobRef> jobQueue;
	GridManagerRef grid;

	int hostId;

	string GameName;
	string GamePassWord;

	string mapHash;
	// 현재 room이 진행중인 맵의 sectionData를 참조
	shared_ptr<const MapSectionData> mapSectionData;

	USE_LOCK;

	Room(int roomId, int hostId, std::string GameName, std::string GamePassWord, std::string mapHash);
	int Add(GameSessionRef session);
	GameObjectRef AddPlayer();
	void Remove(GameSessionRef session);
	void RemoveGameSession(GameSessionRef session);

	void Broadcast(SendBufferRef sendBuffer);
	void Update();
	void ProcessJob();

	void StartGame();

	GameObjectRef ObjectAdd(GameObjectCode objectId, Vector3 position, Vector3 direction, GameObjectRef shooter = nullptr, int owner = -1, bool broad = true);
	void ObjectRemove(GameObjectRef object);

	void HandleCollisions();

	int GetRoomId() const { return roomId; }
	int GetPlayerCount() const { return playerCount; }
	long long GetRoomTime() const { return currentRoomTime; }
	void GetRoomPlayerInfo(Vector<GameSessionRef>& out);
};

class Job
{
public:
	USE_LOCK;
	// 오브젝트 상태 변화 시 필요한 Job
	Job(int objectId, Vector3 position, Vector3 target, GameObjectState state);
	// 오브젝트 생성 작업 시 필요한 Job
	Job(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectState state);
	// 오브젝트 생성, shooter까지 포함한 Job
	Job(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectRef shooter, GameObjectState state);
	~Job();

	int GetObjectId() const { return objectId; }
	GameObjectCode GetObjectCode() const { return objectCode; }
	Vector3 GetPosition() const { return position; }
	Vector3 GetTarget() const { return target; }
	Vector3 GetDirection() const { return direction; }
	GameObjectState GetState() const { return state; }
	GameObjectRef GetShooter() { return shooter; }

private:
	int objectId;
	Vector3 position;
	Vector3 direction;
	Vector3 target;
	GameObjectState state;
	GameObjectCode objectCode;
	GameObjectRef shooter;
};

long long GetNowMs();