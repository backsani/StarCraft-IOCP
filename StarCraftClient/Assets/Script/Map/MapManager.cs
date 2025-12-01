using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MapManager : MonoBehaviour
{
    [SerializeField] private List<GameObject> tiles = new List<GameObject>();
    // Start is called before the first frame update
    void Start()
    {
        ServerConnect.Instance.callback = null;
        ServerConnect.Instance.callback = Init;
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void Init()
    {
        BuildTileMap(PacketRelay.Instance.currentMapData);
    }

    public void BuildTileMap(MapData map)
    {

        for(int i = 0; i < map.height; i++)
        {
            for(int j = 0; j < map.width; j++) 
            {
                GameObject tile = Instantiate(tiles[i * map.width + j], new Vector3(j * 0.32f, i * 0.32f, 0), Quaternion.identity);
                tile.transform.parent = transform;
            }
        }
    }
}
