using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class RoomMakeManager : MonoBehaviour
{
    [SerializeField] TMP_InputField GameName;
    [SerializeField] TMP_InputField PassWord;
    [SerializeField] TMP_Dropdown MapId;

    public void OnCreateRoom()
    {
        uint[] gameName = GlobalUtils.PackStringToBytes(GameName.text);
        uint[] passWord = GlobalUtils.PackStringToBytes(PassWord.text.Trim());

        Protocol.C_ROOM_CREATE roomData = new Protocol.C_ROOM_CREATE();

        roomData.GameId = ulong.Parse(ServerConnect.Instance.UserId);
        roomData.GameName.AddRange(gameName);
        if (passWord.Length > 0)
            roomData.GamePassWord.AddRange(passWord);
        roomData.MapId = MapId.value;

        PacketManager.Send(roomData);
    }
}
