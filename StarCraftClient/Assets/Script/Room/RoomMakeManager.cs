using Google.Protobuf;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using TMPro;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.UI;

public class RoomMakeManager : MonoBehaviour
{
    [SerializeField] TMP_InputField GameName;
    [SerializeField] TMP_InputField PassWord;
    [SerializeField] TMP_Dropdown MapId;

    List<string> mapFiles = new List<string>();
    List<string> mapNames = new List<string>();
    int selectedIndex;

    private void Awake()
    {
        string path = Path.Combine(Application.streamingAssetsPath, "Maps");

        if(!Directory.Exists(path))
        {
            Debug.LogError("Map folder not found");
            return;
        }

        MakeDropdown(path);
    }

    private void MakeDropdown(string path)
    {
        MapId.ClearOptions();

        foreach (string p in Directory.GetFiles(path, "*.bin"))
        {
            mapFiles.Add(p);
            mapNames.Add(Path.GetFileNameWithoutExtension(p));
        }


        MapId.AddOptions(mapNames);

        MapId.value = 0;
        // 현재 value로 강제 갱신
        MapId.RefreshShownValue();
    }

    public void OnCreateRoom()
    {
        string gameName = GameName.text;
        string passWord = PassWord.text;

        Protocol.C_ROOM_CREATE roomData = new Protocol.C_ROOM_CREATE();

        roomData.GameId = ulong.Parse(ServerConnect.Instance.UserId);
        roomData.GameName = gameName;
        if (passWord.Length > 0)
            roomData.GamePassWord = passWord;

        byte[] bytes;
        
        if(!GlobalUtils.ExtractionMapHash(mapFiles[MapId.value], out bytes))
        {
            return;
        }
        roomData.MapHash = ByteString.CopyFrom(bytes);
        Debug.Log(bytes.ToString());

        PacketManager.Send(roomData);
    }
}
