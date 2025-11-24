using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class RoomContent : MonoBehaviour
{
    public int roomId;
    public uint roomPlayerCount;

    [SerializeField] private TextMeshProUGUI roomCodeText;
    [SerializeField] private TextMeshProUGUI playerCountText;


    public void Init(int roomId, uint playerCount)
    {
        this.roomId = roomId;
        this.roomPlayerCount = playerCount;

        roomCodeText.text = "World Id : " + this.roomId.ToString();
        playerCountText.text = "Players : " + this.roomPlayerCount.ToString();
    }

    public void OnAcceptButton()
    {
        Protocol.C_ROOM_REQUEST c_ROOM_REQUEST = new Protocol.C_ROOM_REQUEST();

        c_ROOM_REQUEST.RoomCode = roomId;

        PacketManager.Send(c_ROOM_REQUEST);
    }
}
