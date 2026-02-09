#pragma once
#include <cstdint>
#include <vector>
#include <fstream>
#include <string>

// 섹션 종류 (16비트 정수로 저장)
enum class MapSection : uint16
{
    HASH = 0,   // MapHash 데이터
    OWNR = 1,   // 플레이어 정보(인원)
    SIZE = 2,   // 맵 크기
    MTXM = 3,   // 지형 정보(타일맵)
    RESO = 4,   // 리소스 정보(0 : 미네랄, 1:가스)
    SPOS = 5,   // 플레이어의 시작 위치
};

// enum class인 MapSection을 unordered_map의 키로 사용하기 위해서 사용.
// enum class는 내부 Hash 함수가 구현이 안 되어있어서 map에서 기본적으로 사용 불가
//struct MapSectionHash {
//    size_t operator()(MapSection s) const noexcept {
//        using U = std::underlying_type_t<MapSection>;
//        return std::hash<U>{}(static_cast<U>(s));
//    }
//};

struct ResourceEntry
{
    uint16 type; // 0: 미네랄, 1: 가스
    int16 x;
    int16 y;
};

struct PlayerStartPos
{
    uint16 playerIndex;
    int16 x;
    int16 y;
};

struct MapSectionData
{
    unordered_map<MapSection, vector<uint8_t>> sections;

    uint16 playerCount;

    int32 mapWidth = 0;
    int32 mapHeight = 0;

    vector<uint16> tiles;
    vector<uint16> height;
    uint16 scale;

    vector<ResourceEntry> resources;
    vector<PlayerStartPos> playerPos;
};

class MapMaker
{
public:
    MapMaker();
    static MapMakerRef GetInstance() {
        static MapMakerRef instance = MakeShared<MapMaker>();
        return instance;
    }

    // MapHash와 MapData로 이루어진 map
    std::unordered_map<string, shared_ptr<const MapSectionData>> hashToMap;

    void ParsingMapData(MapSection section, vector<uint8_t> data, shared_ptr<MapSectionData> out);

private:
    void MapFileLoader();

    bool GetHashData(vector<uint8_t>& data, vector<uint8_t>& mapHash);
    bool SetHashToMap(vector<uint8_t>& data);
};