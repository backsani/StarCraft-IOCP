using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MapManager : MonoBehaviour
{
    [SerializeField] private List<Sprite> tiles = new List<Sprite>();
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void BuildTileMap(byte[] map)
    {
        for(int i = 0; i < map.Length / 4;  i++)
        {

        }
    }
}
