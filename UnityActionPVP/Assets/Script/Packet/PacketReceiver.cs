using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using UnityEngine;
using System.Linq;

public class PacketReceiver : MonoBehaviour
{
    private Queue<byte> receiveQueue = new Queue<byte>();
    private Socket clientSocket;

    public void Init(Socket socket)
    {
        clientSocket = socket;
    }

    /// <summary>
    /// 비동기 소켓 통신 함수로 서버로부터 비동기로 패킷을 받는다.
    /// </summary>
    public void StartReceive()
    {
        SocketAsyncEventArgs receiveEventArgs = new SocketAsyncEventArgs();

        receiveEventArgs.Completed += OnReceiveCompleted;

        byte[] buffer = new byte[1024];
        receiveEventArgs.SetBuffer(buffer, 0, buffer.Length);

        if (!clientSocket.ReceiveAsync(receiveEventArgs))
        {
            OnReceiveCompleted(this, receiveEventArgs);
        }
    }

    /// <summary>
    /// 받은 패킷의 크기를 읽어서 크기만큼 자른 후 receiveQueue에 저장. 이후 receiveQueue가 비어있지 않다면 메인 쓰레드에서 꺼내서 처리
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>
    public void OnReceiveCompleted(object sender, SocketAsyncEventArgs e)
    {

        if (e.BytesTransferred > 0 && e.SocketError == SocketError.Success)
        {
            Debug.Log("받은 패킷 정보 : " + BitConverter.ToString(e.Buffer.Take(e.BytesTransferred).ToArray()));

            foreach (var b in e.Buffer.Take(e.BytesTransferred))
            {
                receiveQueue.Enqueue(b);
            }

            while (receiveQueue.Count >= 4)
            {
                byte[] header = new byte[2];
                for (int i = 0; i < 2; i++) header[i] = receiveQueue.ToArray()[i];
                int packetSize = BitConverter.ToInt16(header, 0);

                if (receiveQueue.Count< packetSize)
                {
                    break;
                }

                byte[] packet = new byte[packetSize];
                for (int i = 0; i < packetSize; i++)
                {
                    packet[i] = receiveQueue.Dequeue();
                }

                ServerConnect.Instance.EnqueueRecvData(packet);
            }
        }

        StartReceive();
    }
}