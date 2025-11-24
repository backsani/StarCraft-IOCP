#pragma once

class Room;
using RoomRef = std::shared_ptr<Room>;

class RoomManager {
public:
    std::unordered_map<int, RoomRef> rooms;

    Set<int> playerIdList;
    int roomIdCount;

public:
    RoomManager();

    static RoomManagerRef GetInstance() {
        static RoomManagerRef instance = MakeShared<RoomManager>();
        return instance;
    }

    /// <summary>
    /// 룸을 만들어주는 함수
    /// </summary>
    /// <param name="room"> room 객체 </param>
    void AddRoom(RoomRef room);

    /// <summary>
    /// 특정 룸에 대한 정보를 가져오는 함수
    /// </summary>
    /// <param name="id"> 원하는 roomId </param>
    /// <returns> RoomRef를 반환. 해당 룸이 없으면 nullptr </returns>
    RoomRef GetRoom(int id);

    /// <summary>
    /// 존재하는 모든 room을 update
    /// </summary>
    void Update();
};

