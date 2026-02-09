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


void MapMaker::ParsingMapData(MapSection section, vector<uint8_t> data, shared_ptr<MapSectionData> out)
{
    switch (section)
    {
    case MapSection::HASH:
        break;

    case MapSection::OWNR:
        memcpy(&(out->playerCount), data.data(), sizeof(uint16));
        break;

    case MapSection::SIZE:
        memcpy(&(out->mapWidth), data.data(), sizeof(int32));
        memcpy(&(out->mapHeight), data.data() + sizeof(int32), sizeof(int32));
        memcpy(&(out->scale), data.data() + (sizeof(int32) * 2), sizeof(int32));
        break;

    case MapSection::MTXM:
    {
        size_t count = (data.size() / sizeof(uint16_t))/2;
        out->tiles.resize(count);
        out->height.resize(count);

        int index = 0;
        for (size_t i = 0; i < count; i++)
        {
            uint16 tileId;
            uint16 height;

            memcpy(&tileId, data.data() + index, sizeof(uint16));
            index += sizeof(uint16);

            memcpy(&height, data.data() + index, sizeof(uint16));
            index += sizeof(uint16);

            out->tiles[i] = tileId;
            out->height[i] = height;
        }

        break;
    }

    case MapSection::RESO:
    {
        int index = 0;

        while (index < data.size())
        {
            if (index + sizeof(uint16) + (sizeof(int16) * 2) > data.size())
                break;

            ResourceEntry entry;

            memcpy(&entry.type, data.data() + index, sizeof(uint16));
            index += sizeof(uint16);

            memcpy(&entry.x, data.data() + index, sizeof(uint16));
            index += sizeof(int16);
            memcpy(&entry.y, data.data() + index, sizeof(uint16));
            index += sizeof(int16);

            out->resources.emplace_back(entry);
        }

        break;
    }

    case MapSection::SPOS: 
    {
        int index = 0;
        uint16 playerCount = 0;

        while (index < data.size())
        {
            if (index + (sizeof(int16) * 2) > data.size())
                break;

            PlayerStartPos spos;

            spos.playerIndex = playerCount++;

            memcpy(&spos.x, data.data() + index, sizeof(uint16));
            index += sizeof(int16);
            memcpy(&spos.y, data.data() + index, sizeof(uint16));
            index += sizeof(int16);

            out->playerPos.emplace_back(spos);
        }

        break;
    }

    default:
        break;
    }
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

        ParsingMapData((MapSection)sectionCode, sectionData, mapData);

        mapData->sections[(MapSection)sectionCode] = move(sectionData);
    }

    hashToMap[hash] = mapData;

    return true;
}