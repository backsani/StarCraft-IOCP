using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;

public class RoomManager : MonoBehaviour
{
    [SerializeField] private TextMeshProUGUI userId;
    [SerializeField] private GameObject room;
    [SerializeField] private GameObject Content;
    // Start is called before the first frame update

    [SerializeField] private GameObject WorngWindow;
    private TextMeshProUGUI worngMessage;

    private void Awake()
    {
        Content = transform.Find("RoomInfo").Find("Viewport").Find("Content").gameObject;

        worngMessage = WorngWindow.transform.Find("Image").transform.Find("Message").GetComponent<TextMeshProUGUI>();
    }

    void Start()
    {
        Init();
        ServerConnect.Instance.callback = WrongPassWordWindow;
        ExitRoomLog();
    }

    private void OnDestroy()
    {
        ServerConnect.Instance.callback = null;
        ServerConnect.Instance.showRoomInfoAction -= ShowRoomInfo;

    }

    public void Init()
    {
        userId.text = ServerConnect.Instance.UserId;
        ServerConnect.Instance.showRoomInfoAction += ShowRoomInfo;
        TakeRoomData();

    }

    public void TakeRoomData()
    {
        foreach(Transform child in  Content.transform)
        {
            Destroy(child.gameObject);
        }

        Protocol.C_ROOM_DATA roomData = new Protocol.C_ROOM_DATA();

        roomData.Dummy = 0;

        PacketManager.Send(roomData);
    }

    /// <summary>
    /// 방의 정보를 만들어서 보여주는 함수.
    /// </summary>
    /// <param name="roomId"> room의 Id 번호 </param>
    /// <param name="playerCount"> 해당 룸의 플레이어 접속 수 </param>
    void ShowRoomInfo(int roomId, uint playerCount, string roomName, bool isPassWord, byte[] mapHash)
    {
        GameObject roomData = Instantiate(room);
        roomData.transform.parent = Content.transform;
        RoomContent roomContent = roomData.GetComponent<RoomContent>();

        roomContent.Init(this, roomId, playerCount, roomName, isPassWord, mapHash);
    }

    public void OnCreateRoom()
    {
        SceneManager.LoadScene("RoomMaking");
    }

    public void WrongPassWordWindow()
    {
        worngMessage.text = "Wrong PassWord";
        WorngWindow.gameObject.SetActive(true);
    }

    public void ExitRoomLog()
    {
        switch (RoomData.Instance.currentDisconnectCode)
        {
            case DisconnectCode.DISCONNECT_NONE:
                break;
            case DisconnectCode.EXIT:
                worngMessage.text = "Room Exit";
                WorngWindow.gameObject.SetActive(true);
                break;
            case DisconnectCode.ADMIN_EXIT:
                worngMessage.text = "Room Admin Exit";
                WorngWindow.gameObject.SetActive(true);
                break;
            case DisconnectCode.RESIGN:
                worngMessage.text = "Kicked Out Room";
                WorngWindow.gameObject.SetActive(true);
                break;
            default:
                break;
        }
    }

    public void WornWindowShow(string str)
    {
        worngMessage.text = str;
        WorngWindow.gameObject.SetActive(true);
    }
}
