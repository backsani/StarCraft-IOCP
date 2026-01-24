#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Room.h"
#include "RoomManager.h"

void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
    RoomManagerRef roomManager = RoomManager::GetInstance();

    if (currentRoom != INT_MAX)
    {
        RoomRef room = roomManager->GetRoom(currentRoom);
        if (room != nullptr) // 호스트라면 지우고 아니면 룸에서만 제거
        {
            room->Remove(static_pointer_cast<GameSession>(shared_from_this()));
        }

        currentRoom = INT_MAX;
    }

    roomManager->playerIdList.erase(playerName);

    GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO : packetId 대역 체크
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
}