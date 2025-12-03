using System.Collections;
using System.Collections.Generic;
using UnityEditor.Build;
using UnityEngine;

public class MapManager : MonoBehaviour
{
    [SerializeField] private List<GameObject> tiles = new List<GameObject>();
    [SerializeField] private List<GameObject> resourceTiles = new List<GameObject>();
    // Start is called before the first frame update
    void Start()
    {
        ServerConnect.Instance.callback = null;
        Init();
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void Init()
    {
        BuildTileMap();
        BuildResource();
        SettingCamera();
    }

    public void BuildTileMap()
    {
        MapData map = PacketRelay.Instance.currentMapData;
        for (int i = 0; i < map.height; i++)
        {
            for(int j = 0; j < map.width; j++) 
            {
                GameObject tile = Instantiate(tiles[map.map[i * map.width + j]], new Vector3(j * 0.32f, i * 0.32f, 0), Quaternion.identity);
                tile.transform.parent = transform;
            }
        }
    }

    public void BuildResource()
    {

        foreach (ResourceData resource in PacketRelay.Instance.currentResourceData)
        {
            GameObject tile = Instantiate(resourceTiles[(int)resource.ResourceType], new Vector3(resource.x * 0.32f, resource.y * 0.32f, 0), Quaternion.identity);
            tile.transform.parent = transform;
        }
    }

    public void SettingCamera()
    {
        int index = 0;
        PlayerStartPosition spos = default;

        foreach (PlayerInfo player in ServerConnect.Instance.playerInfo)
        {
            if(player.PlayerId.ToString() == ServerConnect.Instance.UserId)
            {
                spos = PacketRelay.Instance.playerStartPositions[index];
                ServerConnect.Instance.playerIndex = index;
                break;
            }
            index++;
        }

        Camera.main.transform.position = new Vector3(spos.x * 0.32f, spos.y * 0.32f, -10);
    }
}
