using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum UnitCode
{
    NONE,
    PLAYER,
    ENEMY,
    BULLET
}

public enum GameObjectState
{
    IDLE,
	MOVE,
	ATTACK,
	DEAD,
	SPAWN
};

public class UnitManager : MonoBehaviour
{
    private static UnitManager instance;
    public static UnitManager Instance
    {
        get
        {
            if (instance == null)
            {
                instance = FindObjectOfType<UnitManager>();

                if(instance == null)
                {
                    GameObject manager = new GameObject("UnitManager");
                    instance = manager.AddComponent<UnitManager>();
                }
            }
            return instance;
        }
    }

    [SerializeField] private GameObject playerPrefab;
    [SerializeField] private GameObject enemyPrefab;
    [SerializeField] private GameObject bulletPrefab;
    [SerializeField] private GameObject hpSliderPrefab;

    public int roomCode;

    public Dictionary<uint, Unit> units = new Dictionary<uint, Unit>();

    // RTT
    public long serverOffset = 0;
    public long serverNow = 0;

    private void Awake()
    {
        if (instance == null)
        {
            instance = this;
            Debug.Log("UnitManager 생성");
        }
        else if (instance != this)
        {
            Debug.LogWarning("다른 UnitManager 인스턴스가 이미 존재합니다. 중복 인스턴스 제거합니다.");
            Destroy(gameObject);
        }

        roomCode = ServerConnect.Instance.currentRoomCode;
        PingPong();

    }

    private void Update()
    {
        long clientNow = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
        serverNow = clientNow + serverOffset;

        while (ServerConnect.Instance.spawnQueue.Count > 0)
        {
            Action action = ServerConnect.Instance.spawnQueue.Dequeue();
            action.Invoke();
        }
    }

    public void SpawnUnit(UnitCode code, uint unitId, Vector3 position, Quaternion direction, long spawnTime = 0)
    {
        Debug.Log("SpawnUnit 실행" + code.ToString());

        GameObject spawn = null;
        switch (code)
        {
            case UnitCode.NONE:
                break;

            case UnitCode.PLAYER:
                spawn = Instantiate(playerPrefab, position, direction);
                spawn.AddComponent<PlayerUnit>();

                if (unitId == ServerConnect.Instance.playerObjectId)
                {
                    Camera.main.transform.SetParent(spawn.transform, false);
                    spawn.AddComponent<PlayerController>();
                }

                GameObject hpSlider = Instantiate(hpSliderPrefab);
                hpSlider.transform.SetParent(spawn.transform);
                hpSlider.GetComponent<Canvas>().worldCamera = Camera.main;

                PlayerUnit player = spawn.GetComponent<PlayerUnit>();
                player.InitHealthBar(hpSlider.transform);

                break;

            case UnitCode.ENEMY:
                spawn = Instantiate(enemyPrefab, position, direction);
                break;

            case UnitCode.BULLET:
                spawn = Instantiate(bulletPrefab, position, direction);
                spawn.AddComponent<BulletUnit>();

                break;

            default:
                break;
        }

        if (spawn != null)
        {
            Debug.Log("prefab 매칭 성공");

            Unit unit = spawn.GetComponent<Unit>();
            unit.Init(unitId, position, direction * Vector3.up, spawnTime);
            units.Add(unitId, unit);
        }
        
    }

    public void RemoveUnit(uint objectId)
    {
        Unit unit = units[objectId];
        Destroy(unit.gameObject);
        units.Remove(objectId);
    }

    public void PingPong()
    {
        long clientTime = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();

        Protocol.C_RTT_PING c_RTT_PING = new Protocol.C_RTT_PING();

        c_RTT_PING.ClientTime = clientTime;

        PacketManager.Send(c_RTT_PING);
    }
}
