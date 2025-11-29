#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "Grid.h"

Room::Room(int roomId, int hostId, vector<INT32> GameName, vector<INT32> GamePassWord, INT32 mapId) : roomId(roomId), hostId(hostId), GameName(GameName), GamePassWord(GamePassWord), mapId(mapId)
{
	idCount = 0;
	playerCount = 0;

	grid = MakeShared<GridManager>(60, 60, 3, 3);
}

int Room::Add(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.insert(session);
	playerCount++;
	session->SetRoomId(roomId);
	GameObjectRef player = AddPlayer();
	_sessionPlayers[session] = player->GetId();
	SendBufferRef sendBuffer = player->spawn();

	for (GameSessionRef player : _sessions)
	{
		if (player == session)
			continue;
		player->Send(sendBuffer);
	}

	return player->GetId();
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

	auto objectIt = objects.find(objectId);
	if (objectIt == objects.end() || idList.find(objectId) == idList.end())
		return;

	GameObjectRef object = objectIt->second;

	grid->RemoveObject(object);
	ObjectRemove(object);

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

GameObjectRef Room::ObjectAdd(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectRef shooter)
{
	GameObjectRef object = nullptr;

	switch (objectCode)
	{
	case GameObjectCode::NONE:
	{
		break;
	}
	case GameObjectCode::PLAYER:
	{
		break;
	}
	case GameObjectCode::ENEMY:
	{
		// 더 보완해야함
		idList.insert(idCount);
		break;
	}
	case GameObjectCode::BULLET:
	{
		object = MakeShared<Bullet>(shared_from_this(), idCount, position, direction, shooter);
		objects[idCount] = object;
		idList.insert(idCount);

		object->UpdateBounds();
		break;
	}

	default:
		break;
	}

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
