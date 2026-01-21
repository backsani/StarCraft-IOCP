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
	// 기존의 오브젝트를 바로 지우는 방식은 object의 erase가 작동되면서
	// 컨테이너의 크기가 변경되어 이터레이터가 잘못된 위치를 가르키게됨.
	// object의 주소값을 복사해놓고 해당 주소값만 접근해 해결.
	vector<GameObjectRef> objectsToRemove;
	objectsToRemove.reserve(room->objects.size());

	for (const auto& object : room->objects)
	{
		objectsToRemove.push_back(object.second);
	}

	for (const auto& object : objectsToRemove)
	{
		room->ObjectRemove(object);
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
