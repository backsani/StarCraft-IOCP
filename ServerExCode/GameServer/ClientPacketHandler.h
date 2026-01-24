#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_C_MOVE = 1001,
	PKT_C_ROOM_CREATE = 1002,
	PKT_C_ROOM_DATA = 1003,
	PKT_C_ROOM_REQUEST = 1004,
	PKT_C_ROOM_PLAYER_LIST_REQUEST = 1005,
	PKT_C_START_GAME = 1006,
	PKT_C_EXIT_GAME = 1007,
	PKT_C_ATTACK = 1008,
	PKT_C_RTT_PING = 1009,
	PKT_S_RTT_PONG = 1010,
	PKT_S_LOGIN = 1011,
	PKT_S_ROOM_LOBBY = 1012,
	PKT_S_LOBBY_PLAYER_INFO = 1013,
	PKT_S_GAME_START = 1014,
	PKT_S_ROOM_EXIT = 1015,
	PKT_S_ROOM_DATA = 1016,
	PKT_S_ROOM_RESPONSE = 1017,
	PKT_S_MOVE = 1018,
	PKT_S_OBJECT_SPAWN = 1019,
	PKT_S_OBJECT_DEAD = 1020,
	PKT_S_OBJECT_DAMAGE = 1021,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt);
bool Handle_C_ROOM_CREATE(PacketSessionRef& session, Protocol::C_ROOM_CREATE& pkt);
bool Handle_C_ROOM_DATA(PacketSessionRef& session, Protocol::C_ROOM_DATA& pkt);
bool Handle_C_ROOM_REQUEST(PacketSessionRef& session, Protocol::C_ROOM_REQUEST& pkt);
bool Handle_C_ROOM_PLAYER_LIST_REQUEST(PacketSessionRef& session, Protocol::C_ROOM_PLAYER_LIST_REQUEST& pkt);
bool Handle_C_START_GAME(PacketSessionRef& session, Protocol::C_START_GAME& pkt);
bool Handle_C_EXIT_GAME(PacketSessionRef& session, Protocol::C_EXIT_GAME& pkt);
bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt);
bool Handle_C_RTT_PING(PacketSessionRef& session, Protocol::C_RTT_PING& pkt);

class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_LOGIN>(Handle_C_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_MOVE>(Handle_C_MOVE, session, buffer, len); };
		GPacketHandler[PKT_C_ROOM_CREATE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ROOM_CREATE>(Handle_C_ROOM_CREATE, session, buffer, len); };
		GPacketHandler[PKT_C_ROOM_DATA] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ROOM_DATA>(Handle_C_ROOM_DATA, session, buffer, len); };
		GPacketHandler[PKT_C_ROOM_REQUEST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ROOM_REQUEST>(Handle_C_ROOM_REQUEST, session, buffer, len); };
		GPacketHandler[PKT_C_ROOM_PLAYER_LIST_REQUEST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ROOM_PLAYER_LIST_REQUEST>(Handle_C_ROOM_PLAYER_LIST_REQUEST, session, buffer, len); };
		GPacketHandler[PKT_C_START_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_START_GAME>(Handle_C_START_GAME, session, buffer, len); };
		GPacketHandler[PKT_C_EXIT_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_EXIT_GAME>(Handle_C_EXIT_GAME, session, buffer, len); };
		GPacketHandler[PKT_C_ATTACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ATTACK>(Handle_C_ATTACK, session, buffer, len); };
		GPacketHandler[PKT_C_RTT_PING] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_RTT_PING>(Handle_C_RTT_PING, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_RTT_PONG& pkt) { return MakeSendBuffer(pkt, PKT_S_RTT_PONG); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ROOM_LOBBY& pkt) { return MakeSendBuffer(pkt, PKT_S_ROOM_LOBBY); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LOBBY_PLAYER_INFO& pkt) { return MakeSendBuffer(pkt, PKT_S_LOBBY_PLAYER_INFO); }
	static SendBufferRef MakeSendBuffer(Protocol::S_GAME_START& pkt) { return MakeSendBuffer(pkt, PKT_S_GAME_START); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ROOM_EXIT& pkt) { return MakeSendBuffer(pkt, PKT_S_ROOM_EXIT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ROOM_DATA& pkt) { return MakeSendBuffer(pkt, PKT_S_ROOM_DATA); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ROOM_RESPONSE& pkt) { return MakeSendBuffer(pkt, PKT_S_ROOM_RESPONSE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_OBJECT_SPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_OBJECT_SPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_OBJECT_DEAD& pkt) { return MakeSendBuffer(pkt, PKT_S_OBJECT_DEAD); }
	static SendBufferRef MakeSendBuffer(Protocol::S_OBJECT_DAMAGE& pkt) { return MakeSendBuffer(pkt, PKT_S_OBJECT_DAMAGE); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};