#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "Grid.h"
#include "MapMaker.h"
#include "Unit.h"
#include "ProtossUnit.h"
#include "RoomManager.h"
#include "Protocol.pb.h"
#include <random>

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

Room::Room(int roomId, int hostId, std::string GameName, std::string GamePassWord, std::string mapHash) : roomId(roomId), hostId(hostId), GameName(GameName), GamePassWord(GamePassWord), mapHash(mapHash)
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
	Vector<GameSessionRef> playersInfo;

	bool deleteRoom = false;
	{
		WRITE_LOCK;

		RemoveGameSession(session);

		GetRoomPlayerInfo(playersInfo);

		// 플레이어가 룸에 없거나 방장이 나가면 해당 방 삭제
		deleteRoom = (playerCount == 0 || session->GetPlayerName() == hostId);
		if (deleteRoom)
		{
			_sessions.clear(); 
			_sessionPlayers.clear();
			playerCount = 0;
		}
	}

	// 플레이어가 0이면 for문이 작동되지 않을 것.
	if (deleteRoom)
	{
		Protocol::S_ROOM_EXIT packet;

		packet.set_diconnectcode(Protocol::DisconnectCode::ADMIN_EXIT);
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);

		for (auto player : playersInfo)
		{
			player->SetRoomId(INT_MAX);
			player->Send(sendBuffer);
		}
	}
	// 방이 사라지지 않는다면, 방에 남아있는 플레이어들에게 현재 방 플레이어 정보를 보냄
	else
	{
		Protocol::S_LOBBY_PLAYER_INFO packet;

		for (GameSessionRef player : playersInfo)
		{
			Protocol::PlayerInfo* data = packet.add_playerdata();

			data->set_playerid(player->GetPlayerName());
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);
		
		for (GameSessionRef recver : playersInfo)
		{
			recver->Send(sendBuffer);
		}
	}
	

	if(deleteRoom)
	{
		RoomManagerRef roomManager = RoomManager::GetInstance();
		roomManager->RemoveRoom(shared_from_this());
	}
}

void Room::RemoveGameSession(GameSessionRef session)
{
	if (_sessions.erase(session) == 0)
		return;

	auto sessionPlayerIt = _sessionPlayers.find(session);
	if (sessionPlayerIt == _sessionPlayers.end())
		return;

	_sessionPlayers.erase(sessionPlayerIt);

	playerCount--;
	return;
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
			object->SetMove(job->GetState(), job->GetPosition(), job->GetTarget());
			break;
		}
		case GameObjectState::MOVE:
		{
			object->SetMove(job->GetState(), job->GetPosition(), job->GetTarget());
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

void Room::StartGame()
{
	// 플레이어 index 설정
	{
		int index = 0;
		for (pair<GameSessionRef, int> p : _sessionPlayers)
		{
			ingamePlayerIndex[p.first] = index;
			index++;
		}
	}

	// 플레이어 별 시작위치 선정
	{
		vector<int> v = { 0,1,2,3 };
		random_device rd;
		mt19937 gen(rd());
		shuffle(v.begin(), v.end(), gen);

		for (pair<GameSessionRef, int> p : ingamePlayerIndex)
		{
			PlayerStartPos spos = mapSectionData->playerPos[v[p.second]];

			Protocol::S_GAME_START packet;
			IngamePlayerInfo info;

			info.playerIndex = p.second;
			packet.set_playerid(p.second);

			cout << p.first << " " << p.second << endl;

			// 시작시 넥서스, 프로브 데이터 넘기기.
			for (int i = -1; i < 3; i++)
			{
				GameObjectRef object = ObjectAdd(GameObjectCode::PROBE, { (float)spos.x + i, (float)spos.y - 2, 0 }, { 0,0,0 }, nullptr, info.playerIndex);
			}
			


			// 자원 오브젝트들 생성하기.


			info.spos = spos;
			Protocol::Vector3* data = packet.mutable_startpos();

			data->set_x(spos.x);
			data->set_y(spos.y);
			data->set_z(0);

			cout << spos.x << " " << spos.y << " " << info.playerIndex << endl;

			SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);
			p.first->Send(sendBuffer);
		}
	}
	/*for (PlayerStartPos spos : map->sposv)
	{
		GameObjectRef object = ObjectAdd(GameObjectCode::PROBE, { spos.x * 0.32f, spos.y * 0.32f, 0 }, { 0,0,0 }, nullptr, spos.playerIndex);
	}*/
}

GameObjectRef Room::ObjectAdd(GameObjectCode objectCode, Vector3 position, Vector3 direction, GameObjectRef shooter, int owner, bool broad)
{
	GameObjectRef object = nullptr;

	int idx = static_cast<int>(objectCode);

	int objectId = idCount;

	object = g_spawners[idx](shared_from_this(), idCount, position);
	object->ownerId = owner;
	//object->SetPosition(position);

	objects[objectId] = object;
	idList.insert(idCount);

	if(broad)
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

// 현재 room에 있는 플레이어들의 정보를 복사해주는 스냅샷 함수
void Room::GetRoomPlayerInfo(Vector<GameSessionRef>& out)
{
	for (GameSessionRef player : _sessions)
	{
		out.push_back(player);
	}
}


Job::Job(int objectId, Vector3 position, Vector3 target, GameObjectState state) : objectId(objectId), position(position), target(target), state(state), objectCode(GameObjectCode::NONE)
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
