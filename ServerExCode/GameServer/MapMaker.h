#include <cstdint>
#include <vector>
#include <fstream>
#include <string>

// 섹션 종류 (32비트 정수로 저장)
enum class MapSection : int32
{
    OWNR = 0,   // 플레이어 정보(인원)
    SIZE = 1,   // 맵 크기
    MTXM = 2,   // 지형 정보(타일맵)
    RESO = 3,   // 리소스 정보(0 : 미네랄, 1:가스)
    SPOS = 4,   // 플레이어의 시작 위치
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

private:
    vector<int32> m_buffer;
    int32 SectionCount;

    void Clear() { m_buffer.clear(); }

    // 섹션 하나를 추가: [type(4)] [size(4)] [data(size)]
    void AppendSection(MapSection type, const vector<int32>& data);
};