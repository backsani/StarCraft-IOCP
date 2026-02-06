#include "pch.h"
#include "MapMaker.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <filesystem>


MapMaker::MapMaker()
{
    MapFileLoader();
}

void MapMaker::AppendSection(MapSection type, const vector<int32>& data)
{
    m_buffer.push_back(static_cast<int32>(type));

    m_buffer.push_back(static_cast<int32>(data.size()));

    m_buffer.insert(m_buffer.end(), data.begin(), data.end());

    SectionCount++;
}

void MapMaker::MapFileLoader()
{
    // SolutionDir/Map
    filesystem::path dir = filesystem::current_path() / "Map";

    // 해당 주소의 모든 파일들을 순회
    for (const auto& file : filesystem::directory_iterator(dir))
    {
        // text, 바이너리 등 파일인지 체크
        if (!file.is_regular_file())
            continue;

        // 확장자 명이 bin인 파일들만
        if (file.path().extension() == ".bin")
        {
            // 정상적으로 열렸는지 확인
            ifstream bin(file.path(), std::ios::binary);
            if (!bin.is_open())
            {
                cout << "file can't read on " << file.path() << endl;
                continue;
            }

            // bin의 파일 포인터를 맨 뒤로
            bin.seekg(0, std::ios::end);
            // 현재 포인터의 위치 값(맨뒤) == 파일의 크기
            size_t size = bin.tellg();
            // 파일 포인터를 다시 맵 앞으로
            bin.seekg(0, std::ios::beg);

            vector<uint8_t> data(size);
            // 파일을 data로 읽어오기
            bin.read(reinterpret_cast<char*>(data.data()), size);
            if (!bin)
                return;

            vector<uint8_t> mapHash;
            if (!GetHashData(data, mapHash))
            {
                cout << "Can't find map hash on " << file.path() << endl;
                continue;
            }

            if (!SetHashToMap(data))
            {
                cout << "find data Invalid on " << file.path() << endl;
                continue;
            }

            cout << "Map Loader Good" << endl;
        }
    }
}

bool MapMaker::GetHashData(vector<uint8_t>& data, vector<uint8_t>& mapHash)
{
    int index = 0;

    uint16 sectionCode;
    memcpy(&sectionCode, data.data() + index, sizeof(uint16));
    index += sizeof(uint16);

    if ((MapSection)sectionCode != MapSection::HASH)
    {
        return false;
    }

    int32 size;
    memcpy(&size, data.data() + index, sizeof(int32));
    index += sizeof(int32);

    mapHash.resize(size);
    memcpy(mapHash.data(), data.data() + index, size);

    return true;
}

bool MapMaker::SetHashToMap(vector<uint8_t>& data)
{
    int index = 0;

    string hash;
    // Hash 추출
    {
        uint16 sectionCode;
        memcpy(&sectionCode, data.data() + index, sizeof(uint16));
        index += sizeof(uint16);

        if ((MapSection)sectionCode != MapSection::HASH)
        {
            return false;
        }

        int32 size;
        memcpy(&size, data.data() + index, sizeof(int32));
        index += sizeof(int32);

        hash.assign(reinterpret_cast<const char*>(data.data() + index), size);
        index += size;
    }

    // hashToMap에서 MapSectionData가 const이기 때문에 
    // 미리 만들어서 값을 전부 변경 후 마지막에 map에 삽입
    auto mapData = MakeShared<MapSectionData>();

    // SectionData 추출
    while (index < data.size())
    {
        uint16 sectionCode;
        memcpy(&sectionCode, data.data() + index, sizeof(uint16));
        index += sizeof(uint16);

        int32 size;
        memcpy(&size, data.data() + index, sizeof(int32));
        index += sizeof(int32);

        if (size < 0)
            return false;

        vector<uint8_t> sectionData(size);
        memcpy(sectionData.data(), data.data() + index, size);
        index += size;

        // sectionData는 현재 스코프가 끝나면 파괴되고 사용이 끝나므로 move 시멘틱을 사용해 소유권을 이전시켜 복사 비용 줄이기
        mapData->sections[(MapSection)sectionCode] = move(sectionData);
    }

    hashToMap[hash] = mapData;

    return true;
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
        int32 numPlayers = 4;
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
        data.push_back(0);

        // 맵 크기 불러오는 중 오류 발생
        //data.reserve(width * height);
        //for (int32 y = 0; y < height; y++)
        //{
        //    for (int32 x = 0; x < width; x++)
        //    {
        //        // 평지
        //        data.push_back(0);
        //    }
        //}

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

   

    // (2,2)에 미네랄
    resources.push_back({ 0, 4, 2 });
    // (3,2)에 미네랄
    resources.push_back({ 0, 6, 2 });
    // (4,2)에 미네랄
    resources.push_back({ 0, 8, 2 });
    // (2,2)에 가스
    resources.push_back({ 1, 2, 4 });

    resources.push_back({ 0, 4, 30 });
    resources.push_back({ 0, 6, 30 });
    resources.push_back({ 0, 8, 30 });
    resources.push_back({ 1, 2, 28 });

    resources.push_back({ 0, 28, 30 });
    resources.push_back({ 0, 26, 30 });
    resources.push_back({ 0, 24, 30 });
    resources.push_back({ 1, 30, 28 });

    resources.push_back({ 0, 28, 2 });
    resources.push_back({ 0, 26, 2 });
    resources.push_back({ 0, 24, 2 });
    resources.push_back({ 1, 30, 4 });

    {
        vector<int32> data;
        int32 resCount = static_cast<int32>(resources.size());

        //data.push_back(resCount);

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

    {
        vector<int32> data;
        vector<int32> randomIndex = { 0, 1, 2, 3 };
        vector<pair<int32, int32>> spos = { {5.5, 4.5}, {5.5,27.5}, {26.5,27.5}, {26.5,4.5} };

        static std::mt19937 rng(std::random_device{}());

        std::shuffle(randomIndex.begin(), randomIndex.end(), rng);

        for (int i = 0; i < 4; i++)
        {
            data.push_back(randomIndex[i]);
            data.push_back(spos[i].first);
            data.push_back(spos[i].second);

            PlayerStartPos temp = { randomIndex[i], spos[i].first, spos[i].second };
            sposv.push_back(temp);
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