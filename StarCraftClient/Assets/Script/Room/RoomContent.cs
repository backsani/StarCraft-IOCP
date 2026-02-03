using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
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


    public void Init(int roomId, uint playerCount, string roomName, bool isPassWord, byte[] mapHash)
    {
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

        Protocol.C_ROOM_REQUEST c_ROOM_REQUEST = new Protocol.C_ROOM_REQUEST();

        c_ROOM_REQUEST.RoomCode = roomId;

        if(passWordInput.text.Length > 0)
            c_ROOM_REQUEST.GamePassWord = passWordInput.text;

        PacketManager.Send(c_ROOM_REQUEST);
    }
}
