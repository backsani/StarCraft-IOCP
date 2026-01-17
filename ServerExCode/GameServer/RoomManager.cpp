#include "pch.h"
#include "RoomManager.h"
#include "Room.h"

RoomManager::RoomManager()
{
	roomIdCount = 0;
}

void RoomManager::AddRoom(RoomRef room)
{
	WRITE_LOCK;
	rooms[roomIdCount] = room;
	roomIdCount++;
}

void RoomManager::RemoveRoom(RoomRef room)
{
	WRITE_LOCK;
	for (pair<int, GameObjectRef> object : room->objects)
	{
		room->ObjectRemove(object.second);
	}
	rooms.erase(room->GetRoomId());
}

RoomRef RoomManager::GetRoom(int id)
{
	READ_LOCK;

	auto it = rooms.find(id);
	if (it != rooms.end())
		return it->second;
	return nullptr;
}

void RoomManager::Update()
{
	for(auto room : rooms)
	{
		room.second->Update();
	}
}
