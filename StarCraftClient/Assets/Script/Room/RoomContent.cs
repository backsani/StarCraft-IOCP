using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using TMPro;
using UnityEditor.EditorTools;
using UnityEngine;
using UnityEngine.UI;

public class RoomContent : MonoBehaviour
{
    public int roomId;
    public uint roomPlayerCount;
    public string roomName;
    public bool isPassWord;
    private byte[] mapHash;

    [SerializeField] private TextMeshProUGUI roomNameText;
    [SerializeField] private TextMeshProUGUI playerCountText;
    [SerializeField] private Toggle passWordToggle;
    [SerializeField] private GameObject passWordWindow;
    [SerializeField] private TMP_InputField passWordInput;

    RoomManager roomManager;

    public void Init(RoomManager roomManager, int roomId, uint playerCount, string roomName, bool isPassWord, byte[] mapHash)
    {
        this.roomManager = roomManager;
        this.roomId = roomId;
        this.roomPlayerCount = playerCount;

        roomNameText.text = roomName;
        playerCountText.text = "Players : " + this.roomPlayerCount.ToString();
        passWordToggle.isOn = isPassWord;
        this.isPassWord = isPassWord;

        this.mapHash = mapHash;
    }

    public void OnAcceptButton()
    {
        if(isPassWord && !passWordWindow.activeInHierarchy)
        {
            passWordWindow.SetActive(true);

            return;
        }

        // RoomData는 MatchingScene을 로드할 때 모든 맵 파일을 읽어서
        // <Hash, path>로 HashToMapname Dictionary를 초기화한다.
        // 즉 현재 선택한 룸의 MapHash 키가 없다면 맵이 없는 것이다.
        if (!RoomData.Instance.HashToMapname.ContainsKey(Convert.ToBase64String(mapHash)))
        {
            roomManager.WornWindowShow("Can't find map");
            return;
        }

        Protocol.C_ROOM_REQUEST c_ROOM_REQUEST = new Protocol.C_ROOM_REQUEST();

        c_ROOM_REQUEST.RoomCode = roomId;

        if(passWordInput.text.Length > 0)
            c_ROOM_REQUEST.GamePassWord = passWordInput.text;

        PacketManager.Send(c_ROOM_REQUEST);
    }
}
