#include <cstdint>
#include <vector>
#include <fstream>
#include <string>

// 섹션 종류 (32비트 정수로 저장)
enum class MapSection : int16
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
struct MapSectionHash {
    size_t operator()(MapSection s) const noexcept {
        using U = std::underlying_type_t<MapSection>;
        return std::hash<U>{}(static_cast<U>(s));
    }
};

struct ResourceEntry
{
    int32 type; // 0: 미네랄, 1: 가스
    int32 x;
    int32 y;
};

struct PlayerStartPos
{
    int32 playerIndex;
    int32 x;
    int32 y;
};

struct MapSectionData
{
    unordered_map<MapSection, vector<uint8_t>, MapSectionHash> sections;
};

class MapMaker
{
public:
    MapMaker();
    static MapMakerRef GetInstance() {
        static MapMakerRef instance = MakeShared<MapMaker>();
        return instance;
    }

    // 샘플 데이터(질문에서 준 정보)로 버퍼를 구성
    void BuildSampleMap();

    // 현재 버퍼를 파일로 저장
    bool SaveToFile(const string& path) const;

    const vector<int32>& GetBuffer() const { return m_buffer; }
    const int32 GetSectionCount() { return SectionCount; }

    vector<ResourceEntry> resources;
    vector<PlayerStartPos> sposv;

    // MapHash와 MapData로 이루어진 map
    unordered_map<string, shared_ptr<const MapSectionData>> hashToMap;

private:
    vector<int32> m_buffer;
    int32 SectionCount;

    void Clear() { m_buffer.clear(); }

    // 섹션 하나를 추가: [type(4)] [size(4)] [data(size)]
    void AppendSection(MapSection type, const vector<int32>& data);

    void MapFileLoader();

    bool GetHashData(vector<uint8_t>& data, vector<uint8_t>& mapHash);
    bool SetHashToMap(vector<uint8_t>& data);
};