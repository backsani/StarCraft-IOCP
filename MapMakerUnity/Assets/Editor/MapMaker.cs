using System;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using System.IO;
using System.Security.Cryptography;

public enum MapSection : ushort
{
    HASH = 0,
    OWNR = 1,
    SIZE = 2,
    MTXM = 3,
    RESO = 4,
    SPOS = 5,
}

public class MapMaker : EditorWindow
{
    TileMapLeader tileMapLeader;
    ushort playerNum;
    ushort scale = 2;
    string fileName;

    List<PlayerStartPoint> points;

    [MenuItem("Tools/Map Maker")]
    public static void Open()
    {
        GetWindow<MapMaker>("Map Maker");
    }

    // Unity의 window창으로 Tool을 만들어서 플레이를 시키지 않고 코드를 작성시키는 함수
    void OnGUI()
    {
        GUILayout.Label("TileMap Leader", EditorStyles.boldLabel);

        tileMapLeader = (TileMapLeader)EditorGUILayout.ObjectField("TileMapLeader", tileMapLeader, typeof(TileMapLeader), true);

        EditorGUILayout.Space();


        fileName = EditorGUILayout.TextField("File Name", fileName);
        playerNum = (ushort)EditorGUILayout.IntField("Player Number", playerNum);


        if (GUILayout.Button("Run Map Maker"))
        {
            Run(tileMapLeader);
        }
    }

    // 파일을 작성하기 전 정보들을 정리하는 함수
    private void Run(TileMapLeader tileMapLeader)
    {
        TileMapSize tileMapSize;

        if(tileMapLeader == null)
        {
            Debug.Log("Map Leader Null");
            return;
        }

        tileMapLeader.Init();

        byte[] TileData = tileMapLeader.TileMapDatas(out tileMapSize);
        if(TileData == null || TileData.Length == 0)
        {
            Debug.Log("tileMap is Null");
            return;
        }

        byte[] resourceData = tileMapLeader.ResourceTileMapDatas(out points);
        if (resourceData == null || resourceData.Length == 0)
        {
            Debug.Log("tileMap is Null");
            return;
        }

        if (!WriteMapFile(TileData, tileMapSize, resourceData))
            Debug.Log("Fail Make File");
        

    }

    // 정보들을 가지고 파일을 실제로 작성하는 함수
    private bool WriteMapFile(byte[] mapData, TileMapSize tileMapSize, byte[] resourceData)
    {
        // 파일의 저장 주소를 현재 운영체제에 포멧에 맞게 만들어준다.
        string projectPath = Application.dataPath;
        // 파일을 저장할 위치를 사용자가 지정할 수 있게 창을 연다.
        string path = EditorUtility.SaveFilePanel("Save Map Binary", projectPath, fileName, "bin");

        if (string.IsNullOrEmpty(path))
        {
            return false;
        }


        // 파일 작성, 저장 과정에서 예외처리
        try
        {
            using (FileStream fs = new FileStream(path, FileMode.Create, FileAccess.Write))
            {
                using (BinaryWriter bw = new BinaryWriter(fs))
                {
                    byte[] checksum = MakeChecksum(mapData, resourceData, points);
                    
                    using var sha = SHA256.Create();
                    byte[] data = sha.ComputeHash(checksum);

                    {
                        bw.Write((ushort)MapSection.HASH);
                        bw.Write((Int32)data.Length);
                        bw.Write(data);
                    }
                    // 파일의 형식에 맞게 저장. 스코프로 나누어서 가독성을 높임.
                    {
                        bw.Write((ushort)MapSection.OWNR);
                        bw.Write((Int32)1 * sizeof(ushort));
                        bw.Write(playerNum);
                    }

                    {
                        bw.Write((ushort)MapSection.SIZE);
                        bw.Write((Int32)(2 * sizeof(int) + sizeof(ushort)));
                        bw.Write(tileMapSize.width);
                        bw.Write(tileMapSize.height);
                        bw.Write(scale);
                    }

                    {
                        bw.Write((ushort)MapSection.MTXM);
                        bw.Write((Int32)(mapData.Length));
                        bw.Write(mapData);
                    }

                    {
                        bw.Write((ushort)MapSection.RESO);
                        bw.Write((Int32)resourceData.Length);
                        bw.Write(resourceData);
                    }

                    {
                        bw.Write((ushort)MapSection.SPOS);
                        bw.Write((Int32)(points.Count * sizeof(short) * 2));

                        foreach (PlayerStartPoint point in points)
                        {
                            bw.Write(point.x);
                            bw.Write(point.y);
                        }
                    }

                    bw.Flush();
                }
            }
            Debug.Log("File Make Succes");
        }
        // 예외가 발생하면 로그로 던진다.
        catch(Exception ex)
        {
            Debug.LogException(ex);
            return false;
        }

        return true;
    }

    private byte[] MakeChecksum(byte[] mapData, byte[] resourceData, List<PlayerStartPoint> points)
    {
        byte[] bytes;

        using (var ms = new MemoryStream(capacity: mapData.Length + resourceData.Length + (points.Count * sizeof(short) * 2)))
        {
            using (var bw = new BinaryWriter(ms))
            {
                bw.Write(mapData);
                bw.Write(resourceData);

                foreach (PlayerStartPoint point in points)
                {
                    bw.Write((short)point.x);
                    bw.Write((short)point.y);
                }

                bw.Flush();
            }
            bytes = ms.ToArray();
        }

        return bytes;
    }
}
