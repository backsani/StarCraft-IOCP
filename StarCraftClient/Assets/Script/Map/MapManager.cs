using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Tilemaps;
using System.IO;


public class MapManager : MonoBehaviour
{
    public TileDataScriptable tileDataScriptable;

    private Dictionary<int, Tile> tileDictionary = new Dictionary<int, Tile>();

    private Dictionary<MapSection, byte[]> mapSectionData;

    // 그려줄 TileMap;
    public Tilemap Tilemap;

    // Start is called before the first frame update
    void Start()
    {
        ServerConnect.Instance.callback = null;
        Init();
    }

    public void Init()
    {
        MapDataInit();
        BuildTileMap();
        BuildResource();
        SettingCamera();
    }

    public void BuildTileMap()
    {
        byte[] mapData = mapSectionData[MapSection.MTXM];
        byte[] mapSize = mapSectionData[MapSection.SIZE];
        int index = 0;

        int width = BitConverter.ToInt32(mapSize, index);
        index += sizeof(Int32);

        int height = BitConverter.ToInt32(mapSize, index);
        index += sizeof(Int32);

        ushort scale = BitConverter.ToUInt16(mapSize, index);
        index += sizeof(ushort);

        index = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                Vector3Int pos = new Vector3Int(j, i, 1);

                Tilemap.SetTile(pos, tileDictionary[(int)BitConverter.ToUInt16(mapData, index)]);
                index += sizeof(ushort);
                index += sizeof(ushort);
            }
        }
    }

    public void BuildResource()
    {
        byte[] resourceData = mapSectionData[MapSection.RESO];
        int index = 0;


        while(index < resourceData.Length)
        {
            ushort resourceId = BitConverter.ToUInt16(resourceData, index);
            index += sizeof(ushort);

            short x = BitConverter.ToInt16(resourceData, index);
            index += sizeof(short);

            short y = BitConverter.ToInt16(resourceData, index);
            index += sizeof(short);

            Vector3 pos = new Vector3(x, y, 0);
            // pos = GlobalUtils.WorldToCell(Tilemap, pos);

            GameObject ob = new GameObject($"Resource_{resourceId}");
            ob.transform.position = pos;


            ob.AddComponent<SpriteRenderer>().sprite = tileDataScriptable.resources[(int)resourceId].sprite;
        }

    }

    public void SettingCamera()
    {
        //int index = 0;
        PlayerStartPosition spos = PacketRelay.Instance.playerStartPositions;

        //foreach (PlayerInfo player in ServerConnect.Instance.playerInfo)
        //{
        //    if(player.PlayerId.ToString() == ServerConnect.Instance.UserId)
        //    {
        //        spos = PacketRelay.Instance.playerStartPositions[index];
        //        ServerConnect.Instance.playerIndex = index;
        //        break;
        //    }
        //    index++;
        //}

        Camera.main.transform.position = new Vector3(spos.x, spos.y, -10);
    }

    private void MapDataInit()
    {
        List<TileData> tiles = tileDataScriptable.tiles;

        tileDictionary = new Dictionary<int, Tile>(tiles.Count);

        // ScriptableObject에 저장된 sprite, Id를 Dictionary로 초기화
        foreach (TileData tile in tiles)
        {
            if (tile.sprite == null)
                continue;

            Tile data = ScriptableObject.CreateInstance<Tile>();
            data.sprite = tile.sprite;
            data.color = Color.white;
            data.transform = Matrix4x4.identity;

            tileDictionary[tile.value] = data;
        }



        string path = RoomData.Instance.HashToMappath[Convert.ToBase64String(RoomData.Instance.MapHash)];

        if (!File.Exists(path))
        {
            Debug.LogError($"File not found: {path}");
            return;
        }

        // file을 불러와서 해당 파일을 섹션별로 Dictionary로 저장
        byte[] fileData = File.ReadAllBytes(path);
        GlobalUtils.ExtractionMapSection(fileData, out mapSectionData);
    }
}
