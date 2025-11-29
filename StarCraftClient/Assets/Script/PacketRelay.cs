using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public enum MapSection : int
{
    OWNR = 0,   // 플레이어 정보(인원)
    SIZE = 1,   // 맵 크기
    MTXM = 2,   // 지형 정보(타일맵)
    RESO = 3,   // 리소스 정보(0 : 미네랄, 1:가스)
    SPOS = 4,   // 플레이어의 시작 위치
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

    public void MapLeader(int count, IEnumerable<int> data)
    {
        int[] mapData = data.ToArray();
        int index = 0;

        for(int i = 0; i < count; i++)
        {
            MapSection sectionType = (MapSection)mapData[index++];
            int size = mapData[index];
            int[] temp = new int[size];

            Array.Copy(mapData, index, temp, 0, size);

            index += size + 1;

            switch (sectionType)
            {
                case MapSection.OWNR:
                    break;
                case MapSection.SIZE:
                    break;
                case MapSection.MTXM:
                    break;
                case MapSection.RESO:
                    break;
                case MapSection.SPOS:
                    break;
                default:
                    break;
            }
        }
    }
}
