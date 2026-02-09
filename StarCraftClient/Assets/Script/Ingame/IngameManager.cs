using Protocol;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class IngameManager : MonoBehaviour
{
    private Camera camera;
    private LayerMask hitMask = ~0;

    private int ingamePlayerId;
    [SerializeField] private List<Unit> selectUnit = new List<Unit>();
    [SerializeField] private RectTransform selectionBox;

    private Vector2 startScreen;
    private bool dragging;

    private void Awake()
    {
        camera = Camera.main;
        
    }
    // Start is called before the first frame update
    void Start()
    {
        ingamePlayerId = ServerConnect.Instance.playerIndex;
    }

    // Update is called once per frame
    void Update()
    {
        if(Input.GetMouseButtonDown(0))
        {
            ClearUnit();

            // UI 클릭 시
            if (EventSystem.current != null && EventSystem.current.IsPointerOverGameObject())
                return;

            dragging = true;
            startScreen = Input.mousePosition;

            if(selectionBox != null)
            {
                selectionBox.gameObject.SetActive(true);
                UpdateSelectionUI(startScreen, startScreen);
            }



            Vector2 worldPos = camera.ScreenToWorldPoint(Input.mousePosition);

            RaycastHit2D hit = Physics2D.Raycast(worldPos, Vector2.zero, hitMask);

            if(hit.collider != null &&  hit.collider.gameObject.TryGetComponent<Unit>(out Unit unit))
            {
                selectUnit.Add(unit);
                unit.SelectObject();
            }
        }

        if(dragging && Input.GetMouseButton(0))
        {
            if(selectionBox != null)
            {
                UpdateSelectionUI(startScreen, Input.mousePosition);
            }
        }

        if(dragging && Input.GetMouseButtonUp(0))
        {
            dragging = false;
            if(selectionBox != null)
            {
                selectionBox.gameObject.SetActive(false);
            }
            selectInDragArea(startScreen, Input.mousePosition);
        }

        if(Input.GetMouseButtonDown(1))
        {
            Protocol.C_MOVE c_move = new Protocol.C_MOVE();

            c_move.RoomCode = ServerConnect.Instance.currentRoomCode;

            foreach (Unit unit in selectUnit)
            {
                c_move.ObjectId.Add(unit.GetObjectId());
            }

            UnityEngine.Vector3 worldPos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            worldPos.z = 0f;

            Protocol.Vector3 pos = new Protocol.Vector3();

            pos.X = worldPos.x; pos.Y = worldPos.y; pos.Z = worldPos.z;

            c_move.Position = pos;

            PacketManager.Send(c_move);
        }
    }

    private void selectInDragArea(Vector2 startScreen, Vector2 endScreen)
    {
        Vector2 pos1 = camera.ScreenToWorldPoint(startScreen);
        Vector2 pos2 = camera.ScreenToWorldPoint(endScreen);

        Vector2 min = Vector2.Min(pos1, pos2);
        Vector2 max = Vector2.Max(pos1, pos2);

        Collider2D[] hits = Physics2D.OverlapAreaAll(min, max, hitMask);

        foreach(Collider2D hit in hits)
        {
            Unit unit = hit.GetComponentInParent<Unit>();
            if(unit != null && unit is ISelectableObject && unit.owerId == ingamePlayerId)
            {
                selectUnit.Add(unit);
                unit.SelectObject();
            }
        }
    }

    // 마우스를 드래그 중일때 드래그 창 UI를 업데이트 시켜주는 함수
    private void UpdateSelectionUI(Vector2 startScreen1, Vector2 startScreen2)
    {
        float xMin = Mathf.Min(startScreen1.x, startScreen2.x);
        float xMax = Mathf.Max(startScreen1.x, startScreen2.x);
        float yMin = Mathf.Min(startScreen1.y, startScreen2.y);
        float yMax = Mathf.Max(startScreen1.y, startScreen2.y);

        selectionBox.anchoredPosition = new Vector2(xMin, yMin);
        selectionBox.sizeDelta = new Vector2(xMax - xMin, yMax - yMin);
    }

    // 선택중인 유닛 모두 취소
    public void ClearUnit()
    {
        foreach (Unit unit in selectUnit)
        {
            unit.DeselectObject();
        }
            
        selectUnit.Clear();
    }
}
