#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "Grid.h"
#include "MapMaker.h"
#include "Unit.h"
#include "ProtossUnit.h"

// enum GameObjectCode 순서대로 맞춰주거나, 인덱스를 직접 매핑
static SpawnFunc g_spawners[] =
{
	nullptr, // NONE
	nullptr, // PLAYER
	nullptr, // ENEMY
	nullptr, // BULLET
	[](RoomRef room, int id, Vector3 pos) { return MakeShared<Mineral>(room, id, pos); },

	[](RoomRef room, int id, Vector3 pos) { return MakeShared<Gas>(room, id, pos); },

	[](RoomRef room, int id, Vector3 pos) { return MakeShared<ProtossProbe>(room, id, pos); },
	nullptr, // ZEALOT
	nullptr, // DARKTEMPLAR
	nullptr, // DRAGOON
	nullptr, // REAVER
	nullptr, // SHUTTLE
	nullptr, // SCOUT
	nullptr, // ARBITER
	nullptr, // ARCHON
	nullptr, // DARKARCHON
	nullptr, // OBSERVER
	nullptr, // CARRIER
	nullptr, // INTERCEPTOR
	nullptr, // CORSAIR
	nullptr, // HIGHTEMPLAR
	nullptr, // NEXUS
	nullptr, // PYLON
	nullptr, // ASSIMILATOR
	nullptr, // GATEWAY
	nullptr, // FORGE
	nullptr, // PHOTON_CANNON
	nullptr, // CYBERNETICS_CORE
	nullptr, // SHIELD_BATTERY
	nullptr, // ROBOTICS_FACILITY
	nullptr, // STARGATE
	nullptr, // CITADEL_OF_ADUN
	nullptr, // ROBOTICS_SUPPORT_BAY
	nullptr, // FLEET_BEACON
	nullptr, // TEMPLAR_ARCHIVES
	nullptr, // OBSERVATORY
	nullptr, // ARBITER_TRIBUNAL
};

Room::Room(int roomId, int hostId, vector<INT32> GameName, vector<INT32> GamePassWord, INT32 mapId) : roomId(roomId), hostId(hostId), GameName(GameName), GamePassWord(GamePassWord), mapId(mapId)
{
	idCount = 0;
	playerCount = 0;
	currentRoomTime = 0;

	grid = MakeShared<GridManager>(60, 60, 3, 3);
}

int Room::Add(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.insert(session);
	playerCount++;
	session->SetRoomId(roomId);
	//GameObjectRef player = AddPlayer();
	_sessionPlayers[session] = session->GetPlayerName();
	//SendBufferRef sendBuffer = player->spawn();

	/*for (GameSessionRef player : _sessions)
	{
		if (player == session)
			continue;
		player->Send(sendBuffer);
	}*/

	return session->GetPlayerName();
}

GameObjectRef Room::AddPlayer()
{
	GameObjectRef object;

	int objectId = idCount;
	object = MakeShared<Player>(shared_from_this(), objectId, Vector3(30.0f, 30.0f, 0.0f));
	objects[objectId] = object;
	idList.insert(objectId);

	object->UpdateBounds();

	while (idList.find(idCount) != idList.end())
	{
		idCount++;
	}

	grid->AddObject(object);

	return object;
}

void Room::Remove(GameSessionRef session)
{
	WRITE_LOCK;

	if (_sessions.erase(session) == 0)
		return;

	auto sessionPlayerIt = _sessionPlayers.find(session);
	if (sessionPlayerIt == _sessionPlayers.end())
		return;

	int objectId = sessionPlayerIt->second;
	_sessionPlayers.erase(sessionPlayerIt);

	/*auto objectIt = objects.find(objectId);
	if (objectIt == objects.end() || idList.find(objectId) == idList.end())
		return;

	GameObjectRef object = objectIt->second;

	grid->RemoveObject(object);
	ObjectRemove(object);*/

	playerCount--;

	//_sessions.erase(session);
	//playerCount;
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}


void Room::Update()
{
	WRITE_LOCK;
	long long startTime = GetNowMs();

	ProcessJob();

	for (auto object : objects)
	{
		if (object.first == -1)
			continue;

		SendBufferRef sendBuffer = nullptr;

		sendBuffer = object.second->Update();
		grid->MoveObject(object.second, object.second->bounds);

		if (sendBuffer != nullptr)
			Broadcast(sendBuffer);
	}

	HandleCollisions();

	currentRoomTime += GetNowMs() - startTime;
}

void Room::ProcessJob()
{
	while (!jobQueue.empty())
	{
		JobRef job = jobQueue.front();
		jobQueue.pop();

		GameObjectRef object = objects[job->GetObjectId()];

		if (job->GetObjectId() == -1)
			object = nullptr;

		switch (job->GetState())
		{
		case GameObjectState::IDLE:
		{
			object->SetMove(job->GetState(), job->GetDirection());
			break;
		}
		case GameObjectState::MOVE:
		{
			object->SetMove(job->GetState(), job->GetDirection());
			break;
		}
		case GameObjectState::ATTACK:
		{
			object->SetAttack(job->GetPosition());
			break;
		}
		case GameObjectState::DEAD:
		{
			grid->RemoveObject(object);
			ObjectRemove(object);
			break;
		}
		case GameObjectState::SPAWN:
		{
			object = ObjectAdd(job->GetObjectCode(), job->GetPosition(), job->GetDirection(), job->GetShooter());

			if (object != nullptr)
			{
				grid->AddObject(object);
			}

			break;
		}
		default:
			break;
		}
	}
}

void Room::StartGame(MapMakerRef map)
{
	for (ResourceEntry resource : map->resources)
	{
		GameObjectCode type = GameObjectCode::NONE;

		if (resource.type == 0)
			type = GameObjectCode::MINERAL;
		else if (resource.type == 1)
			type = GameObjectCode::GAS;

		GameObjectRef object = ObjectAdd(type, { resource.x * 0.32f, resource.y * 0.32f, 0 }, { 0,0,0 });
	}

	for (PlayerStartPos spos : map->sposv)
	{
		GameObjectRef object = ObjectAdd(GameObjectCode::PROBE, { spos.x * 0.32f, spos.y * 0.32f, 0 }, { 0,0,0 }, nullptr, spos.playerIndex);
	}
}

GameObjectRef Room::ObjectAdd(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectRef shooter, int owner)
{
	GameObjectRef object = nullptr;

	int idx = static_cast<int>(objectCode);

	int objectId = idCount;

	object = g_spawners[idx](shared_from_this(), idCount, position);
	object->ownerId = owner;
	//object->SetPosition(position);

	objects[objectId] = object;
	idList.insert(idCount);

	Broadcast(object->spawn());

	while(idList.find(idCount) != idList.end())
	{
		idCount++;
	}

	return object;
}

void Room::ObjectRemove(GameObjectRef object)
{
	int objectId = object->GetId();

	idList.erase(objectId);
	objects.erase(objectId);

	grid->RemoveObject(object);
}

void Room::HandleCollisions() 
{
	vector<pair<GameObjectRef, GameObjectRef>> collisions;
	grid->FindAllColliders(collisions);

	for (const auto& cell : collisions) 
	{
		if (cell.first->GetObjectCode() == GameObjectCode::BULLET && cell.second->GetObjectCode() == GameObjectCode::PLAYER)
		{ 
			auto bullet = static_pointer_cast<Bullet>(cell.first);
			auto player = static_pointer_cast<Player>(cell.second);
			
			// 본인이 쐈으면 충돌 무시
			if (bullet->IsShooter(player))
				continue;

			player->TakeDamage(bullet->GetDamage());
			bullet->SetDead();

			cout << player->GetId() << "player Hp : " << player->GetHp() << endl;
		}

		else if (cell.first->GetObjectCode() == GameObjectCode::PLAYER && cell.second->GetObjectCode() == GameObjectCode::BULLET)
		{
			auto bullet = static_pointer_cast<Bullet>(cell.second);
			auto player = static_pointer_cast<Player>(cell.first);

			if (bullet->IsShooter(player))
				continue;

			player->TakeDamage(bullet->GetDamage());
			bullet->SetDead();

			cout << player->GetId() << "player Hp : " << player->GetHp() << endl;
		}
	}
}


Job::Job(int objectId, Vector3 position, Vector3 direction, GameObjectState state) : objectId(objectId), position(position), direction(direction), state(state), objectCode(GameObjectCode::NONE)
{
}

Job::Job(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectState state) : objectId(-1), position(position), direction(direction), state(state), objectCode(objectCode)
{
}


Job::Job(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectRef shooter, GameObjectState state) : objectId(-1), position(position), direction(direction), state(state), objectCode(objectCode), shooter(shooter)
{
	
}

Job::~Job()
{
}

long long GetNowMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}
