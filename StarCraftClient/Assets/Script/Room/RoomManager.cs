using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class RoomManager : MonoBehaviour
{
    [SerializeField] private TextMeshProUGUI userId;
    [SerializeField] private GameObject room;
    [SerializeField] private GameObject Content;
    // Start is called before the first frame update
    void Start()
    {
        userId.text = ServerConnect.Instance.UserId;
        ServerConnect.Instance.showRoomInfoAction += ShowRoomInfo;
        TakeRoomData();
    }

    void TakeRoomData()
    {
        Protocol.C_ROOM_DATA roomData = new Protocol.C_ROOM_DATA();

        roomData.Dummy = 0;

        PacketManager.Send(roomData);
    }

    /// <summary>
    /// 방의 정보를 만들어서 보여주는 함수.
    /// </summary>
    /// <param name="roomId"> room의 Id 번호 </param>
    /// <param name="playerCount"> 해당 룸의 플레이어 접속 수 </param>
    void ShowRoomInfo(int roomId, uint playerCount)
    {
        GameObject roomData = Instantiate(room);
        roomData.transform.parent = Content.transform;
        RoomContent roomContent = roomData.GetComponent<RoomContent>();

        roomContent.Init(roomId, playerCount);
    }
}
