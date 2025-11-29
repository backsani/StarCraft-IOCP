#include "pch.h"
#include "MapMaker.h"
#include <algorithm>
#include <random>
#include <ctime>


MapMaker::MapMaker()
{
    BuildSampleMap();
}

void MapMaker::AppendSection(MapSection type, const vector<int32>& data)
{
    m_buffer.push_back(static_cast<int32>(type));

    m_buffer.push_back(static_cast<int32>(data.size()));

    m_buffer.insert(m_buffer.end(), data.begin(), data.end());

    SectionCount++;
}

void MapMaker::BuildSampleMap()
{
    Clear();
    SectionCount = 0;

    // ------------------------
    // 1) 플레이어 정보 (OWNR)
    // ------------------------
    // 플레이어 수 4명
    {
        vector<int32> data;
        uint32 numPlayers = 4;
        data.push_back(numPlayers);

        AppendSection(MapSection::OWNR, data);
    }

    // ------------------------
    // 2) 맵 크기 (SIZE)
    // ------------------------
    // 여기서는 32 * 32 * 2 를
    // width = 32, height = 32, scale = 2 로 저장한다고 가정
    
    int32 width = 32;
    int32 height = 32;
    int32 scale = 2;

    {
        vector<int32> data;
        data.push_back(width);
        data.push_back(height);
        data.push_back(scale);

        AppendSection(MapSection::SIZE, data);
    }

    // ------------------------
    // 3) 지형 정보 (MTXM) - 타일맵
    // ------------------------
    // 32 x 32 = 1024 타일, 전부 0
    
    {
        vector<int32> data;
        data.reserve(width * height);
        for (int32 y = 0; y < height; y++)
        {
            for (int32 x = 0; x < width; x++)
            {
                // 평지
                data.push_back(0);
            }
        }

        AppendSection(MapSection::MTXM, data);
    }

    // ------------------------
    // 4) 리소스 정보 (RESO)
    // ------------------------
    // 형식 예시:
    // [4바이트] resourceCount (uint32)
    // 이후 resourceCount 개의 엔트리:
    // [1바이트] type (0=미네랄, 1=가스)
    // [2바이트] x
    // [2바이트] y
    //
    // 총 1 + 2 + 2 = 5바이트 per entry

    struct ResourceEntry
    {
        int32 type; // 0: 미네랄, 1: 가스
        int32 x;
        int32 y;
    };

    vector<ResourceEntry> resources;

    // (2,2)에 미네랄
    resources.push_back({ 0, 2, 2 });
    // (3,2)에 미네랄
    resources.push_back({ 0, 3, 2 });
    // (4,2)에 미네랄
    resources.push_back({ 0, 4, 2 });
    // (2,2)에 가스
    resources.push_back({ 1, 2, 2 });

    {
        vector<int32> data;
        int32 resCount = static_cast<int32>(resources.size());

        data.push_back(resCount);

        for (ResourceEntry res : resources)
        {
            data.push_back(res.type);
            data.push_back(res.x);
            data.push_back(res.y);
        }

        AppendSection(MapSection::RESO, data);
    }

    // 플레이어 시작 위치
    // (3,5), (3,27), (29, 27), (29,5)

    struct PlayerStartPos
    {
        int32 playerIndex;
        int32 x;
        int32 y;
    };

    vector<PlayerStartPos> spos;

    {
        vector<int32> data;
        vector<int32> randomIndex = { 0, 1, 2, 3 };
        vector<pair<int32, int32>> spos = { {3,5}, {3,27}, {29,27}, {29,5} };

        static std::mt19937 rng(std::random_device{}());

        std::shuffle(randomIndex.begin(), randomIndex.end(), rng);

        for (int i = 0; i < 4; i++)
        {
            data.push_back(randomIndex[i]);
            data.push_back(spos[i].first);
            data.push_back(spos[i].second);
        }

        AppendSection(MapSection::SPOS, data);
    }
}

bool MapMaker::SaveToFile(const string& path) const
{
    ofstream ofs(path, ios::binary);
    if (!ofs.is_open())
        return false;

    ofs.write(reinterpret_cast<const char*>(m_buffer.data()),
        static_cast<streamsize>(m_buffer.size() * sizeof(int32)));

    return ofs.good();
}