using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BulletUnit : Unit
{
    protected override void Start()
    {
        base.Start();
        speed = 0.3f / 0.025f;
    }

    // Update is called once per frame
    void Update()
    {
        float elapsedSec = (UnitManager.Instance.serverNow - spawnTime) / 1250f;

        // 예측 위치 계산
        Vector3 predictedPos = spawnPos + direction * speed * elapsedSec;

        transform.position = predictedPos;
    }

    //서버 위치에 따른 보정
    public override void Move(GameObjectState state, Vector3 position)
    {
        base.Move(state, position);

        spawnPos = position;
        spawnTime = UnitManager.Instance.serverNow;
    }
}
