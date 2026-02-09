using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;

public class Unit : MonoBehaviour, ISelectableObject
{
    [SerializeField] private uint unitId;
    public int owerId;

    public Vector3 spawnPos;
    [SerializeField] protected Vector3 direction;
    protected float health;
    GameObjectState currentstate;
    protected float speed;
    protected long spawnTime;
    [HideInInspector] public GameObject SelectEffect;

    // Start is called before the first frame update
    protected virtual void Start()
    {
        currentstate = GameObjectState.IDLE;

        SpriteRenderer image = GetComponent<SpriteRenderer>();

        SelectEffect = transform.Find("SelectImage").gameObject;
        SpriteRenderer effectImage = SelectEffect.GetComponent<SpriteRenderer>();

        Vector2 imageSize = image.bounds.size;
        Vector2 effectSize = effectImage.bounds.size;

        Vector3 scale = new Vector3(imageSize.x / effectSize.x, imageSize.y / effectSize.y, 1f);

        SelectEffect.transform.localScale = scale;
        SelectEffect.SetActive(false);
    }

    public void Init(uint id, Vector3 pos, Vector3 dir, int owner, long spawnTime)
    {
        UnityEngine.Debug.Log($"Init called: pos = {pos}, dir = {dir}, spawnTime = {spawnTime}");

        unitId = id;
        spawnPos = pos;
        direction = dir;
        this.spawnTime = spawnTime;
        owerId = owner;

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

    public void SelectObject()
    {
        SelectEffect.SetActive(true);
    }

    public void DeselectObject()
    {
        SelectEffect.SetActive(false);
    }
}
