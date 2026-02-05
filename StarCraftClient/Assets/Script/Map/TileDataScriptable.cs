using System;
using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.Tilemaps;

[Serializable]
public struct TileData
{
    public Sprite sprite;
    public int value;
}

[Serializable]
public struct ResourceTileData
{
    public Sprite sprite;
    public int value;
}

[CreateAssetMenu(menuName = "Map/Tile Data")]
public class TileDataScriptable : ScriptableObject
{
    public List<TileData> tiles = new List<TileData>();
    
    public List<ResourceTileData> resources = new List<ResourceTileData>();
}
