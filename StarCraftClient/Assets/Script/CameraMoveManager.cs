using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class CameraMoveManager : MonoBehaviour
{
    public float moveSpeed = 20.0f;
    public float edeSize = 20.0f;

    public float minX, minY, maxX, maxY;

    // Update is called once per frame
    void Update()
    {
        Vector3 direction = Vector3.zero;
        Vector3 mousePos = Input.mousePosition;

        if (mousePos.x <= edeSize)
            direction.x = -1;
        else if (mousePos.x >= Screen.width - edeSize)
            direction.x = 1;
        else if (mousePos.y <= edeSize)
            direction.y = - 1;
        else if (mousePos.y >= Screen.height - edeSize)
            direction.y = 1;

        transform.position += direction.normalized * moveSpeed * Time.deltaTime;
    }

    private void LateUpdate()
    {
        float halfHeight = Camera.main.orthographicSize;
        float halfWidth = halfHeight * Camera.main.aspect;

        Vector3 pos = transform.position;

        pos.x = Mathf.Clamp(pos.x, minX + halfWidth, maxX - halfWidth);
        pos.y = Mathf.Clamp(pos.y, minY + halfHeight, maxY - halfHeight);

        transform.position = pos;
    }

    // 맵 범위 넣어주기. 왼쪽 아래, 오른쪽 위
    //private void LateUpdate()
    //{
    //    Vector3 pos = transform.position;
    //    pos.x = Mathf.Clamp(pos.x, minBounds.x, maxBounds.x);
    //    pos.z = Mathf.Clamp(pos.z, minBounds.y, maxBounds.y);
    //    transform.position = pos;
    //}
}
