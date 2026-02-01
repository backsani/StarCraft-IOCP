using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class RoomMakeManager : MonoBehaviour
{
    [SerializeField] TMP_InputField GameName;
    [SerializeField] TMP_InputField PassWord;
    [SerializeField] TMP_Dropdown MapId;

    string[] mapFiles;
    int selectedIndex;

    private void Awake()
    {
        string path = Path.Combine(Application.streamingAssetsPath, "Maps");

        if(!Directory.Exists(path))
        {
            Debug.LogError("Map folder not found");
            return;
        }

        mapFiles = Directory.GetFiles(path, "*.bin").Select(Path.GetFileName).ToArray();

        MakeDropdown();
    }

    private void MakeDropdown()
    {
        MapId.ClearOptions();

        MapId.AddOptions(mapFiles.ToList());

        MapId.value = 0;
        // 현재 value로 강제 갱신
        MapId.RefreshShownValue();
    }

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
