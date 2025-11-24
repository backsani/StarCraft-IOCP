#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_C_MOVE = 1001,
	PKT_C_ROOM_DATA = 1002,
	PKT_C_ROOM_REQUEST = 1003,
	PKT_C_ATTACK = 1004,
	PKT_S_LOGIN = 1005,
	PKT_S_ROOM_DATA = 1006,
	PKT_S_ROOM_RESPONSE = 1007,
	PKT_S_MOVE = 1008,
	PKT_S_OBJECT_SPAWN = 1009,
	PKT_S_OBJECT_DEAD = 1010,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt);
bool Handle_S_ROOM_DATA(PacketSessionRef& session, Protocol::S_ROOM_DATA& pkt);
bool Handle_S_ROOM_RESPONSE(PacketSessionRef& session, Protocol::S_ROOM_RESPONSE& pkt);
bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt);
bool Handle_S_OBJECT_SPAWN(PacketSessionRef& session, Protocol::S_OBJECT_SPAWN& pkt);
bool Handle_S_OBJECT_DEAD(PacketSessionRef& session, Protocol::S_OBJECT_DEAD& pkt);

class ServerPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LOGIN>(Handle_S_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_S_ROOM_DATA] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ROOM_DATA>(Handle_S_ROOM_DATA, session, buffer, len); };
		GPacketHandler[PKT_S_ROOM_RESPONSE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ROOM_RESPONSE>(Handle_S_ROOM_RESPONSE, session, buffer, len); };
		GPacketHandler[PKT_S_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_MOVE>(Handle_S_MOVE, session, buffer, len); };
		GPacketHandler[PKT_S_OBJECT_SPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_OBJECT_SPAWN>(Handle_S_OBJECT_SPAWN, session, buffer, len); };
		GPacketHandler[PKT_S_OBJECT_DEAD] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_OBJECT_DEAD>(Handle_S_OBJECT_DEAD, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_C_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ROOM_DATA& pkt) { return MakeSendBuffer(pkt, PKT_C_ROOM_DATA); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ROOM_REQUEST& pkt) { return MakeSendBuffer(pkt, PKT_C_ROOM_REQUEST); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ATTACK& pkt) { return MakeSendBuffer(pkt, PKT_C_ATTACK); }

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