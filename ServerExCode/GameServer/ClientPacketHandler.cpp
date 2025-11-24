#include "pch.h"
#include "ClientPacketHandler.h"
#include "RoomManager.h"
#include "GameSession.h"
#include "Room.h"

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
	cout << pkt.logincode() << endl;

	Protocol::S_LOGIN data;
	data.set_gameid(pkt.logincode());

	if (RoomManager::GetInstance()->playerIdList.find(pkt.logincode()) == RoomManager::GetInstance()->playerIdList.end())
	{
		RoomManager::GetInstance()->playerIdList.insert(pkt.logincode());

		data.set_loginaccept(true);
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

	const auto& vec = pkt.direction();
	Vector3 dir(vec.x(), vec.y(), vec.z());

	JobRef job = MakeShared<Job>((int)pkt.objectid(), Vector3(), dir, (GameObjectState)pkt.state());

	{
		WriteLockGuard guard(room->_locks[0], __FUNCTION__);
		room->jobQueue.push(job);
	}
	

	//object->SetMove((GameObjectState)pkt.state(), dir);


	return true;
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

	Protocol::S_ROOM_RESPONSE packet;

	packet.set_roomaccept(true);

	packet.set_roomcode(room->GetRoomId());

	int playerId = room->Add(gameSession);

	packet.set_playerobjectid(playerId);


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

