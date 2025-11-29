using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class MatchMakingUI : MonoBehaviour
{
    [SerializeField] private TextMeshProUGUI roomName;
    [SerializeField] private TextMeshProUGUI roomPassWord;
    private ulong hostId;
    [SerializeField] private TextMeshProUGUI mapId;

    LobbyData roomData;

    public List<TextMeshProUGUI> playersId = new List<TextMeshProUGUI>();

    // Start is called before the first frame update
    void Start()
    {
        roomData = RoomData.Instance.currentRoom;

        roomName.text = roomData.gameName;
        roomPassWord.text = roomData.gamePassWord;
        mapId.text = "map : " + roomData.mapId.ToString();
        hostId = roomData.hostId;

        ServerConnect.Instance.callback = null;
        ServerConnect.Instance.callback = PlayerListApply;

        Protocol.C_ROOM_PLAYER_LIST_REQUEST c_ROOM_PLAYER_LIST_REQUEST = new Protocol.C_ROOM_PLAYER_LIST_REQUEST();

        PacketManager.Send(c_ROOM_PLAYER_LIST_REQUEST);
    }


    public void PlayerListApply()
    {
        int index = 0;

        foreach (PlayerInfo player in ServerConnect.Instance.playerInfo)
        {
            playersId[index].text = player.PlayerId.ToString();
            index++;
        }
        
    }

    public void OnClickStart()
    {
        if(hostId.ToString() == ServerConnect.Instance.UserId)
        {

        }
    }
}
