using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.Tilemaps;

public struct TileInfo
{
    public ushort tileId;
    public ushort tileHeight;

    public TileInfo(ushort tileId, ushort tileHeight)
    {
        this.tileId = tileId;
        this.tileHeight = tileHeight;
    }
}

public struct ResourceInfo
{
    public ushort resourceId;
    public short x;
    public short y;

    public ResourceInfo(ushort resourceId, short x, short y)
    {
        this.resourceId = resourceId;
        this.x = x;
        this.y = y;
    }
}

public struct TileMapSize
{
    public Int32 width;
    public Int32 height;
}

public struct PlayerStartPoint
{
    public short x;
    public short y;

    public PlayerStartPoint(short x, short y)
    {
        this.x = x;
        this.y = y;
    }
}

public class TileMapLeader : MonoBehaviour
{
    [Header("Tile Maps")]
    public Tilemap tileMap;
    public Tilemap ResourceTileMap;

    [Header("Tiles")]
    public TileBase ground;
    public TileBase hill;
    public TileBase runwayUp;
    public TileBase runwayDown;
    public TileBase runwayRight;
    public TileBase runwayLeft;
    public TileBase water;
    public TileBase wall;
    public TileBase bridge;

    [Header("Resources")]
    public TileBase mineral;
    public TileBase gas;
    public TileBase playerStartPointer;

    private Dictionary<TileBase, Func<Vector3Int, TileInfo>> tileHandler;
    private Dictionary<TileBase, Func<Vector3Int, ResourceInfo>> resourceHandler;

    void Awake()
    {
        Init();
    }

    public void Init()
    {
        // tileMap에 저장된 타일들과 func로 함수를 매핑. 좌표는 모든 타일이 꽉 차있으므로 저장하지 않고 1차원 배열로 나열.
        tileHandler = new Dictionary<TileBase, Func<Vector3Int, TileInfo>>
        {
            [ground] = pos => { return new TileInfo(101, 1); },
            [hill] = pos => { return new TileInfo(201, 3); },
            [runwayUp] = pos => { return new TileInfo(301, 2); },
            [runwayDown] = pos => { return new TileInfo(302, 2); },
            [runwayRight] = pos => { return new TileInfo(303, 2); },
            [runwayLeft] = pos => { return new TileInfo(304, 2); },
            [water] = pos => { return new TileInfo(401, 0); },
            [wall] = pos => { return new TileInfo(501, 0); },
            [bridge] = pos => { return new TileInfo(601, 1); },
        };

        // resourceTileMap에 저장된 타일들과 func로 함수를 매핑. id와 좌표를 반환
        resourceHandler = new Dictionary<TileBase, Func<Vector3Int, ResourceInfo>>
        {
            [mineral] = pos => { return new ResourceInfo(0, (short)pos.x, (short)pos.y); },
            [gas] = pos => { return new ResourceInfo(1, (short)pos.x, (short)pos.y); },
            [playerStartPointer] = pos => { return new ResourceInfo(10, (short)pos.x, (short)pos.y); },
        };
    }

    // tileMap에 관한 데이터를 byte로 담아서 반환
    public byte[] TileMapDatas(out TileMapSize tileMapSize)
    {
        tileMap.CompressBounds();
        BoundsInt b = tileMap.cellBounds;

        int width = b.size.x;
        int height = b.size.y;

        byte[] bytes;

        using (var ms = new MemoryStream(capacity: width * height * 4))
        {
            using (var bw = new BinaryWriter(ms))
            {
                for (int y = b.yMin; y < b.yMax; y++)
                {
                    for (int x = b.xMin; x < b.xMax; x++)
                    {
                        Vector3Int pos = new Vector3Int(x, y, 0);
                        TileBase tile = tileMap.GetTile(pos);

                        if (tile == null)
                            continue;

                        if (tileHandler.TryGetValue(tile, out Func<Vector3Int, TileInfo> action))
                        {
                            TileInfo info = action(pos);
                            bw.Write(info.tileId);
                            bw.Write(info.tileHeight);
                            Debug.Log(info);
                        }
                        else
                        {
                            Debug.Log("***** None Tile *****");
                        }
                    }
                }
                bytes = ms.ToArray();

                tileMapSize.width = width; 
                tileMapSize.height = height;
            }

        }


        return bytes;
    }

    // resourceTileMap에 관한 데이터를 byte로 담아서 반환
    public byte[] ResourceTileMapDatas(out List<PlayerStartPoint> playerStartPoints)
    {
        playerStartPoints = new List<PlayerStartPoint>();

        ResourceTileMap.CompressBounds();
        BoundsInt b = ResourceTileMap.cellBounds;

        // GetTilesBlock으로 모든 타일 정보 저장. 비어있는 타일은 null로 저장됨.
        TileBase[] tiles = ResourceTileMap.GetTilesBlock(b);

        byte[] bytes;

        using (var ms = new MemoryStream(capacity: tiles.Length * 3 * sizeof(ushort)))
        {
            using (var bw = new BinaryWriter(ms))
            {
                for (int i = 0; i < tiles.Length; i++)
                {
                    TileBase tile = tiles[i];

                    if(tile == null) 
                        continue;

                    // 모든 타일의 정보 이기에 i는 2차원 배열이 1차원 배열로 저장된 후 현재의 위치를 나타냄
                    int x = i % b.size.x + b.xMin;
                    int y = i / b.size.x + b.yMin;

                    Vector3Int pos = new Vector3Int(x, y, 0);

                    if (resourceHandler.TryGetValue(tile, out Func<Vector3Int, ResourceInfo> action))
                    {
                        ResourceInfo info = action(pos);

                        // 만약 현재 타일이 플레이어 시작 위치라면
                        if(info.resourceId == 10)
                        {
                            // 해당 좌표를 struct로 담아서 out 매개변수인 리스트에 추가
                            PlayerStartPoint point = new PlayerStartPoint(info.x, info.y);
                            playerStartPoints.Add(point);
                        }
                        // 아니라면 bw에 바이너리로 작성.
                        else
                        {
                            bw.Write(info.resourceId);
                            bw.Write(info.x);
                            bw.Write(info.y);
                        }
                        

                        Debug.Log(info);
                    }
                    else
                    {
                        Debug.Log("***** None Resource *****");
                    }

                }
                bw.Flush();
                // 작성한 바이너리 파일을 byte로 복사
                bytes = ms.ToArray();
            }

        }

        return bytes;
    }
}
