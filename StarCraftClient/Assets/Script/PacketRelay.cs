using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public enum MapSection : ushort
{
    HASH = 0,   // MapHash 데이터
    OWNR = 1,   // 플레이어 정보(인원)
    SIZE = 2,   // 맵 크기
    MTXM = 3,   // 지형 정보(타일맵)
    RESO = 4,   // 리소스 정보(0 : 미네랄, 1:가스)
    SPOS = 5,   // 플레이어의 시작 위치
}

[Serializable]
public struct ResourceData
{
    public enum ResourceCode
    {
        Mineral,
        Gas,
    }
    public ResourceCode ResourceType;
    public int x;
    public int y;
}

public struct MapData
{
    public int PlayerNumber;
    public int MapSize;
    public int width;
    public int height;
    public int scale;
    public int[] map;

}

public struct PlayerStartPosition
{
    public int index;
    public float x;
    public float y;
}

public class PacketRelay : MonoBehaviour
{
    private static PacketRelay instance;


    public static PacketRelay Instance
    {
        get
        {
            // 인스턴스가 없으면 생성
            if (instance == null)
            {
                instance = FindObjectOfType<PacketRelay>();

                // 인스턴스가 씬에 없다면 새로 생성
                if (instance == null)
                {
                    GameObject go = new GameObject("PacketRelay");
                    instance = go.AddComponent<PacketRelay>();
                }
            }

            return instance;
        }
    }

    public PlayerStartPosition playerStartPositions;
    public MapData currentMapData;
    public ResourceData[] currentResourceData;

    // 생성자와 초기화
    private void Awake()
    {
        // 싱글톤 인스턴스가 다른 인스턴스와 충돌하면 현재 객체를 파괴
        if (instance != null && instance != this)
        {
            Destroy(gameObject);
        }
        else
        {
            instance = this;
            // 씬을 전환해도 싱글톤을 유지하려면 DontDestroyOnLoad 사용
            DontDestroyOnLoad(gameObject);
        }
    }

    public void GameStart(Vector3 spos)
    {
        playerStartPositions.x = spos.x;
        playerStartPositions.y = spos.y;
    }

    //public void MapLeader(int count, IEnumerable<int> data)
    //{
    //    int[] mapData = data.ToArray();
    //    int index = 1;

    //    for(int i = 0; i < count; i++)
    //    {
    //        MapSection sectionType = (MapSection)mapData[index++];
    //        int size = mapData[index++];
    //        int[] temp = new int[size];

    //        Array.Copy(mapData, index, temp, 0, size);

    //        index += size;

    //        switch (sectionType)
    //        {
    //            case MapSection.OWNR:
    //                playerStartPositions = new PlayerStartPosition[temp[0]];
    //                currentMapData.PlayerNumber = temp[0];
    //                break;

    //            case MapSection.SIZE:
    //                currentMapData.width = temp[0];
    //                currentMapData.height = temp[1];
    //                currentMapData.scale = temp[2];
    //                currentMapData.map = new int[currentMapData.width * currentMapData.height];
    //                break;

    //            case MapSection.MTXM:
    //                if (currentMapData.map == null)
    //                    currentMapData.map = new int[currentMapData.width * currentMapData.height];

    //                for (int j = 0; j < currentMapData.map.Length; j++)
    //                {
    //                    currentMapData.map[i] = temp[0];
    //                }

    //                break;
    //            //currentMapData.MapSize = temp.Length;
    //            //currentMapData.map = temp;
    //            //for(int y = 0; y < currentMapData.height; y++)
    //            //{
    //            //    for(int x = 0; x < currentMapData.width; x++)
    //            //    {
    //            //        currentMapData.map[y * currentMapData.height + x] = temp[0];
    //            //    }
    //            //}
    //            //break;

    //            case MapSection.RESO:
    //                currentResourceData = new ResourceData[temp.Length / 3];

    //                for(int j = 0; j < temp.Length / 3; j++)
    //                {
    //                    currentResourceData[j].ResourceType = (ResourceData.ResourceCode)temp[j * 3];
    //                    currentResourceData[j].x = temp[j * 3 + 1];
    //                    currentResourceData[j].y = temp[j * 3 + 2];
    //                }
    //                break;

    //            case MapSection.SPOS:
    //                for(int j = 0; j < temp.Length / 3; j++)
    //                {
    //                    playerStartPositions[j].index = temp[j * 3];
    //                    playerStartPositions[j].x = temp[j * 3 + 1];
    //                    playerStartPositions[j].y = temp[j * 3 + 2];
    //                }
    //                break;
    //            default:
    //                break;
    //        }
    //    }
    //}
}
