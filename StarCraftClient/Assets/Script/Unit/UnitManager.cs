using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum UnitCode
{
    NONE,
	PLAYER,
	ENEMY,
	BULLET,
	MINERAL,
	GAS,
	PROBE,
	ZEALOT,
	DARKTEMPLAR,
	DRAGOON,
	REAVER,
	SHUTTLE,
	SCOUT,
	ARBITER,
	ARCHON,
	DARKARCHON,
	OBSERVER,
	CARRIER,
	INTERCEPTOR,
	CORSAIR,
	HIGHTEMPLAR,
	NEXUS,
	PYLON,
	ASSIMILATOR,
	GATEWAY,
	FORGE,
	PHOTON_CANNON,
	CYBERNETICS_CORE,
	SHIELD_BATTERY,
	ROBOTICS_FACILITY,
	STARGATE,
	CITADEL_OF_ADUN,
	ROBOTICS_SUPPORT_BAY,
	FLEET_BEACON,
	TEMPLAR_ARCHIVES,
	OBSERVATORY,
	ARBITER_TRIBUNAL,
};

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

    [System.Serializable]
    public struct UnitPrefabEntry
    {
        public UnitCode code;
        public GameObject prefab;
    }

    [SerializeField] private UnitPrefabEntry[] unitPrefabs;

    Dictionary<UnitCode, GameObject> _prefabMap;

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

        _prefabMap = new Dictionary<UnitCode, GameObject>();
        foreach (var entry in unitPrefabs)
            _prefabMap[entry.code] = entry.prefab;
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

    public void SpawnUnit(UnitCode code, uint unitId, int owner, Vector3 position, Quaternion direction, float hp, long spawnTime = 0)
    {
        Debug.Log("SpawnUnit 실행" + code.ToString());

        if(!_prefabMap.TryGetValue(code, out GameObject prefab) || prefab == null)
        {
            Debug.LogWarning($"SpawnUnit: 프리팹이 없는 코드 {code}");
            return;
        }

        GameObject spawn = Instantiate(prefab, position, direction);

        if (spawn != null)
        {
            Debug.Log("prefab 매칭 성공");

            Unit unit = spawn.GetComponent<Unit>();
            unit.Init(unitId, position, direction * Vector3.zero, owner, spawnTime);
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
