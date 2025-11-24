using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;

public abstract class Unit : MonoBehaviour
{
    [SerializeField] private uint unitId;
    public Vector3 spawnPos;
    [SerializeField] protected Vector3 direction;
    protected float health;
    GameObjectState currentstate;
    protected float speed;
    protected long spawnTime;

    // Start is called before the first frame update
    protected virtual void Start()
    {
        currentstate = GameObjectState.IDLE;
    }

    public void Init(uint id, Vector3 pos, Vector3 dir, long spawnTime)
    {
        UnityEngine.Debug.Log($"Init called: pos = {pos}, dir = {dir}, spawnTime = {spawnTime}");

        unitId = id;
        spawnPos = pos;
        direction = dir;
        this.spawnTime = spawnTime;

        transform.position = pos;
    }

    public virtual void Move(GameObjectState state, Vector3 position)
    {
        transform.position = Vector3.Lerp(transform.position, position, 0.3f);

        currentstate = state;
    }

    public uint GetObjectId()
    {
        return unitId;
    }

    public virtual void SetHp(float hp)
    {
        health = hp;
    }
}
