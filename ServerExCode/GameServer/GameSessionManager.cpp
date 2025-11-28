#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include "RoomManager.h"
#include "Room.h"

GameSessionManager GSessionManager;

void GameSessionManager::Add(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.insert(session);
}

void GameSessionManager::Remove(GameSessionRef session)
{
	WRITE_LOCK;

	RoomManagerRef roomManager = RoomManager::GetInstance();
	roomManager->rooms[session->GetRoomId()]->Remove(session);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}