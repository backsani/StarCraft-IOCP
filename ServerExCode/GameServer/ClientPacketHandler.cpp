#include "pch.h"
#include "ClientPacketHandler.h"
#include "RoomManager.h"
#include "GameSession.h"
#include "Room.h"
#include "MapMaker.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

using GameSessionRef = std::shared_ptr<class GameSession>;

// 직접 컨텐츠 작업자

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	cout << "Packet Recv Bug" << endl;
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	cout << pkt.logincode() << endl;

	Protocol::S_LOGIN data;
	data.set_gameid(pkt.logincode());

	if (RoomManager::GetInstance()->playerIdList.find(pkt.logincode()) == RoomManager::GetInstance()->playerIdList.end())
	{
		RoomManager::GetInstance()->playerIdList.insert(pkt.logincode());

		data.set_loginaccept(true);

		gameSession->SetPlayerName(pkt.logincode());

		cout << pkt.test() << endl;
	}
	else
	{
		data.set_loginaccept(false);
	}

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(data);
	session->Send(sendBuffer);

	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	cout << "MovePlayer" << endl;

	RoomManagerRef roomManager = RoomManager::GetInstance();

	RoomRef room = roomManager->GetRoom(pkt.roomcode());

	if (room == nullptr)
		cout << "room Null" << endl;

	for (uint32 id : pkt.objectid())
	{
		GameObjectRef object = room->objects[id];
		Vector3 vec = object->GetPosition();
		Vector3 target(pkt.position().x(), pkt.position().y(), 0);

		cout << id << " : " << vec.x << ", " << vec.y << "target" << target.x << ", " << target.y << endl;

		JobRef job = MakeShared<Job>(id, vec, target, GameObjectState::MOVE);
		{
			WriteLockGuard guard(room->_locks[0], __FUNCTION__);
			room->jobQueue.push(job);
		}
	}

	//object->SetMove((GameObjectState)pkt.state(), dir);


	return true;
}

bool Handle_C_ROOM_CREATE(PacketSessionRef& session, Protocol::C_ROOM_CREATE& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	RoomManagerRef roomManager = RoomManager::GetInstance();

	int hostId = pkt.gameid();
	string gameName = pkt.gamename();
	string gamePassWord = pkt.gamepassword();

	RoomRef room = MakeShared<Room>(roomManager->roomIdCount, hostId, gameName, gamePassWord, pkt.maphash());
	cout << (pkt.maphash()) << endl;

	roomManager->AddRoom(room);

	room->Add(gameSession);

	Protocol::S_ROOM_LOBBY packet;

	packet.set_hostid(hostId);
	packet.set_gamename(gameName);
	packet.set_gamepassword(gamePassWord);
	packet.set_roomcode(room->GetRoomId());
	packet.set_maphash(room->mapHash);

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);
	session->Send(sendBuffer);

	return false;
}


bool Handle_C_ROOM_DATA(PacketSessionRef& session, Protocol::C_ROOM_DATA& pkt)
{
	RoomManagerRef roomManager = RoomManager::GetInstance();

	Protocol::S_ROOM_DATA packet;

	for (auto room : roomManager->rooms)
	{
		Protocol::RoomData* data = packet.add_roomdata();
		data->set_roomcode(room.first);
		data->set_playercount(room.second->GetPlayerCount());

		string name = room.second->GameName;
		string passWord = room.second->GamePassWord;

		if (passWord.size() == 0)
			data->set_ispassword(false);
		else
			data->set_ispassword(true);

		data->set_roomname(name);
		data->set_maphash(room.second->mapHash);
	}

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);
	session->Send(sendBuffer);

	return false;
}

bool Handle_C_ROOM_REQUEST(PacketSessionRef& session, Protocol::C_ROOM_REQUEST& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	RoomManagerRef roomManager = RoomManager::GetInstance();
	RoomRef room = roomManager->rooms[pkt.roomcode()];
	string passWord = pkt.gamepassword();

	if (room->GamePassWord.size() == 0 || room->GamePassWord == passWord)
	{
		room->Add(gameSession);

		Protocol::S_ROOM_LOBBY packet;
		packet.set_hostid(room->hostId);
		packet.set_gamename(room->GameName);
		packet.set_gamepassword(room->GamePassWord);
		packet.set_maphash(room->mapHash);
		packet.set_roomcode(pkt.roomcode());

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);
		session->Send(sendBuffer);
	}
	else
	{
		Protocol::S_ROOM_RESPONSE packet;
		packet.set_roomaccept(false);
		packet.set_roomcode(room->GetRoomId());

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);
		session->Send(sendBuffer);
	}


	/*
	{
		room->_locks;
		for (auto object : room->objects)
		{
			GameObjectRef gameObject = object.second;
			Vector3 pos = gameObject->GetPosition();
			Vector3 dir = gameObject->GetDirection();

			Protocol::ObjectData* data = packet.add_objectdata();
			data->set_type(static_cast<Protocol::ObjectType>(gameObject->GetObjectCode()));
			data->set_objectid(gameObject->GetId());

			Protocol::Vector3* position = data->mutable_position();
			position->set_x(pos.x);
			position->set_y(pos.y);
			position->set_z(pos.z);

			Protocol::Vector3* direction = data->mutable_direction();
			direction->set_x(dir.x);
			direction->set_y(dir.y);
			direction->set_z(dir.z);
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);
		session->Send(sendBuffer);
	}
	
	cout << "Object 정보 전송 : " << playerId << endl;
	*/

	return false;
}

bool Handle_C_ROOM_PLAYER_LIST_REQUEST(PacketSessionRef& session, Protocol::C_ROOM_PLAYER_LIST_REQUEST& pkt)
{
	RoomManagerRef roomManager = RoomManager::GetInstance();

	RoomRef room = roomManager->GetRoom(pkt.roomcode());
	Protocol::S_LOBBY_PLAYER_INFO packet;

	for (pair<GameSessionRef, int> player : room->_sessionPlayers)
	{
		Protocol::PlayerInfo* data = packet.add_playerdata();
		
		data->set_playerid(player.first->GetPlayerName());
		//data->set_playername()
	}

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);
	room->Broadcast(sendBuffer);

	return false;
}

bool Handle_C_START_GAME(PacketSessionRef& session, Protocol::C_START_GAME& pkt)
{
	RoomManagerRef roomManager = RoomManager::GetInstance();

	RoomRef room = roomManager->GetRoom(pkt.roomcode());

	Protocol::S_GAME_START packet;

	MapMakerRef map = MapMaker::GetInstance();

	room->mapSectionData = map->hashToMap[room->mapHash];

	//packet.set_mapsectioncount(map->GetSectionCount());

	//for (int32 data : map->GetBuffer())
	//{
	//	packet.add_mapdata(data);
	//}

	room->StartGame();

	return false;
}

bool Handle_C_EXIT_GAME(PacketSessionRef& session, Protocol::C_EXIT_GAME& pkt)
{
	RoomManagerRef roomManager = RoomManager::GetInstance();

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	int roomId = gameSession->GetRoomId();

	if (roomId == INT_MAX)
		return false;

	// 나가기 요청을 한 플레이어를 방에서 제거
	RoomRef room = roomManager->GetRoom(roomId);

	room->Remove(gameSession);
	{
		Protocol::S_ROOM_EXIT packet;

		packet.set_diconnectcode(Protocol::DisconnectCode::EXIT);

		gameSession->SetRoomId(INT_MAX);

		// 플레이어에게 나갔다는 패킷 전송
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);

		session->Send(sendBuffer);
	}

	return false;
}


bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt)
{
	RoomManagerRef roomManager = RoomManager::GetInstance();

	RoomRef room = roomManager->GetRoom(pkt.roomcode());

	if (room == nullptr)
		cout << "room Null" << endl;

	const auto& vec = pkt.direction();
	Vector3 dir(vec.x(), vec.y(), vec.z());

	{
		WriteLockGuard guard(room->_locks[0], __FUNCTION__);

		GameObjectRef shooter = room->objects[pkt.objectid()];

		Vector3 pos = shooter->GetPosition();

		JobRef job = MakeShared<Job>((GameObjectCode)pkt.objecttype(), pos, dir, shooter, GameObjectState::SPAWN);

		room->jobQueue.push(job);
	}

	pkt.roomcode();
	pkt.objectid();

	return false;
}

bool Handle_C_RTT_PING(PacketSessionRef& session, Protocol::C_RTT_PING& pkt)
{
	Protocol::S_RTT_PONG packet;

	packet.set_client_time(pkt.client_time());

	{
		using namespace std::chrono;
		int64_t serverTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

		packet.set_server_time(serverTime);
	}
	 
	SendBufferRef data = ClientPacketHandler::MakeSendBuffer(packet);

	session->Send(data);

	return true;
}

