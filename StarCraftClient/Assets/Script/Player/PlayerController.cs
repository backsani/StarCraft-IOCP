using Protocol;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum MyEnum
{
    
}
public class PlayerController : MonoBehaviour
{
    private float attackCycle;
    private UnityEngine.Vector3 dir;
    private UnityEngine.Vector3 prev;
    private PlayerUnit player;
    GameObjectState currentState;

    private float timer;

    private void Start()
    {
        Init();
    }

    // Update is called once per frame
    void Update()
    {
        if(timer > 0)
            timer -= Time.deltaTime;

        dir = UnityEngine.Vector3.zero;

        if (Input.GetKey(KeyCode.W))
            dir += UnityEngine.Vector3.up;
        if (Input.GetKey(KeyCode.S))
            dir += UnityEngine.Vector3.down;
        if (Input.GetKey(KeyCode.A))
            dir += UnityEngine.Vector3.left;
        if (Input.GetKey(KeyCode.D))
            dir += UnityEngine.Vector3.right;

        if(Input.GetMouseButtonDown(0))
        {
            if(timer <= 0)
            {
                UnityEngine.Vector3 mouseWorldPos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
                mouseWorldPos.z = 0f; // 2D니까 Z축 제거

                UnityEngine.Vector3 direct = (mouseWorldPos - transform.position).normalized;

                Protocol.C_ATTACK c_ATTACK = new Protocol.C_ATTACK();

                c_ATTACK.RoomCode = UnitManager.Instance.roomCode;

                c_ATTACK.ObjectId = player.GetObjectId();
                c_ATTACK.ObjectType = (Protocol.ObjectType)UnitCode.BULLET;

                Protocol.Vector3 dir = new Protocol.Vector3();
                dir.X = direct.x;
                dir.Y = direct.y;
                dir.Z = direct.z;

                c_ATTACK.Direction = dir;

                PacketManager.Send(c_ATTACK);

                timer = attackCycle;
            }
        }

        if (dir.sqrMagnitude > 1f)
            dir = dir.normalized;

        if(dir.sqrMagnitude == 0.0f)
            currentState = GameObjectState.IDLE;
        else
            currentState = GameObjectState.MOVE;

        if (prev != dir)
        {
            Protocol.C_MOVE c_MOVE = new Protocol.C_MOVE();

            c_MOVE.RoomCode = UnitManager.Instance.roomCode;
            c_MOVE.ObjectId = player.GetObjectId();
            c_MOVE.State = (Protocol.GameObjectState)currentState;

            Protocol.Vector3 direction = new Protocol.Vector3();
            direction.X = dir.x;
            direction.Y = dir.y;
            direction.Z = dir.z;

            c_MOVE.Direction = direction;

            PacketManager.Send(c_MOVE);
        }

        prev = dir;
    }

    void Init()
    {
        this.player = transform.GetComponent<PlayerUnit>();
        currentState = GameObjectState.IDLE;
        timer = 0;
        attackCycle = 3.0f;
    }
}
