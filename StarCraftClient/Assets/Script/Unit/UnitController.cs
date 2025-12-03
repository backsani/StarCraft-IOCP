using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class UnitController : MonoBehaviour
{
    Vector3 dir = Vector3.zero;
    Vector3 prev = Vector3.zero;
    GameObjectState currentState = GameObjectState.IDLE;

    void Update()
    {

        if (Input.GetMouseButtonDown(1))
        {
            // 마우스 위치를 월드 좌표로 변환
            Vector3 mouseWorldPos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            mouseWorldPos.z = 0f; // 2D니까 z 제거

            // 이동 목적지 쪽으로 바라보는 방향(원하면 사용)
            Vector3 dir = (mouseWorldPos - transform.position);

            // C_MOVE 패킷 생성
            Protocol.C_MOVE c_MOVE = new Protocol.C_MOVE();
            c_MOVE.RoomCode = UnitManager.Instance.roomCode;
            c_MOVE.ObjectId = ServerConnect.Instance.currentUnitId;

            // 여기에는 "목적지 위치"를 넣어줌
            Protocol.Vector3 posProto = new Protocol.Vector3();
            posProto.X = mouseWorldPos.x;
            posProto.Y = mouseWorldPos.y;
            posProto.Z = mouseWorldPos.z;

            c_MOVE.Position = posProto;

            PacketManager.Send(c_MOVE);
        }

    }
}
