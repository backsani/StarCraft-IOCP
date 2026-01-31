using System;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using System.IO;

public enum MapSection : ushort
{
    OWNR = 0,
    SIZE = 1,
    MTXM = 2,
    RESO = 3,
    SPOS = 4,
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
                    // 파일의 형식에 맞게 저장. 스코프로 나누어서 가독성을 높임.
                    {
                        bw.Write((ushort)MapSection.OWNR);
                        bw.Write((ushort)1);
                        bw.Write(playerNum);
                    }

                    {
                        bw.Write((ushort)MapSection.SIZE);
                        bw.Write((ushort)3);
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
                        bw.Write((ushort)resourceData.Length * sizeof(ushort));
                        bw.Write(resourceData);
                    }

                    {
                        foreach (PlayerStartPoint point in points)
                        {
                            bw.Write((ushort)MapSection.SPOS);
                            bw.Write((ushort)(points.Count * sizeof(ushort) * 2));
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
}
