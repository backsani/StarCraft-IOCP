using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public struct LobbyData
{
    public ulong hostId;
    public string gameName;
    public string gamePassWord;
    public int mapId;
}

public enum DisconnectCode
{
    DISCONNECT_NONE = 0,
    EXIT = 1,
    ADMIN_EXIT = 2,
    RESIGN = 3,
}

public class RoomData : MonoBehaviour
{
    private static RoomData instance;

    public static RoomData Instance
    {
        get
        {
            // 인스턴스가 없으면 생성
            if (instance == null)
            {
                instance = FindObjectOfType<RoomData>();

                // 인스턴스가 씬에 없다면 새로 생성
                if (instance == null)
                {
                    GameObject go = new GameObject("RoomData");
                    instance = go.AddComponent<RoomData>();
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

        currentDisconnectCode = DisconnectCode.DISCONNECT_NONE;
    }

    public LobbyData currentRoom;
    public ulong hostId;
    public DisconnectCode currentDisconnectCode;

    public void SaveLobbyData(ulong hostId, string gameName, string gamePassWord, int mapId)
    {
        this.hostId = hostId;
        currentRoom.hostId = hostId;
        currentRoom.gameName = gameName;
        currentRoom.gamePassWord = gamePassWord;
        currentRoom.mapId = mapId;

    }
}
