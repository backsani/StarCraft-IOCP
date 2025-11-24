#include "pch.h"
#include "RoomManager.h"
#include "Room.h"

RoomManager::RoomManager()
{
	roomIdCount = 0;
	RoomRef room1 = MakeShared<Room>(roomIdCount);
	AddRoom(room1);
	RoomRef room2 = MakeShared<Room>(roomIdCount);
	AddRoom(room2);
}

void RoomManager::AddRoom(RoomRef room)
{
	rooms[roomIdCount] = room;
	roomIdCount++;
}

RoomRef RoomManager::GetRoom(int id)
{
	if (rooms.find(id) != rooms.end())
		return rooms[id];
	return nullptr;
}

void RoomManager::Update()
{
	for(auto room : rooms)
	{
		room.second->Update();
	}
}
