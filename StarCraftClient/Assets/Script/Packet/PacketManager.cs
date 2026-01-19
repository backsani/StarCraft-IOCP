using Google.Protobuf;
using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;


public enum PacketType
{
    PKT_C_LOGIN = 1000,
    PKT_C_MOVE = 1001,
    PKT_C_ROOM_CREATE = 1002,
    PKT_C_ROOM_DATA = 1003,
    PKT_C_ROOM_REQUEST = 1004,
    PKT_C_ROOM_PLAYER_LIST_REQUEST = 1005,
    PKT_C_START_GAME = 1006,
    PKT_C_ATTACK = 1007,
    PKT_C_RTT_PING = 1008,
    PKT_S_RTT_PONG = 1009,
    PKT_S_LOGIN = 1010,
    PKT_S_ROOM_LOBBY = 1011,
    PKT_S_LOBBY_PLAYER_INFO = 1012,
    PKT_S_GAME_START = 1013,
    PKT_S_ROOM_DATA = 1014,
    PKT_S_ROOM_RESPONSE = 1015,
    PKT_S_MOVE = 1016,
    PKT_S_OBJECT_SPAWN = 1017,
    PKT_S_OBJECT_DEAD = 1018,
    PKT_S_OBJECT_DAMAGE = 1019,
}

public static partial class PacketManager
{
    /// <summary>
    /// PacketType에 맞는 핸들러 함수를 O(1)로 실행시키기 위해 딕셔너리에 저장. buffer만 넘겨주고 실행시켜주면 IMessage형으로 반환해준다.
    /// </summary>
    private static Dictionary<PacketType, Func<byte[], IMessage>> Handlers = new Dictionary<PacketType, Func<byte[], IMessage>>
    {
        { PacketType.PKT_S_RTT_PONG, (buffer) => PacketMaker<Protocol.S_RTT_PONG>.HandlePacket(buffer, PacketType.PKT_S_RTT_PONG) },

        { PacketType.PKT_S_LOGIN, (buffer) => PacketMaker<Protocol.S_LOGIN>.HandlePacket(buffer, PacketType.PKT_S_LOGIN) },
        
        { PacketType.PKT_S_ROOM_LOBBY, (buffer) => PacketMaker<Protocol.S_ROOM_LOBBY>.HandlePacket(buffer, PacketType.PKT_S_ROOM_LOBBY) },

        { PacketType.PKT_S_LOBBY_PLAYER_INFO, (buffer) => PacketMaker<Protocol.S_LOBBY_PLAYER_INFO>.HandlePacket(buffer, PacketType.PKT_S_LOBBY_PLAYER_INFO) },

        { PacketType.PKT_S_GAME_START, (buffer) => PacketMaker<Protocol.S_GAME_START>.HandlePacket(buffer, PacketType.PKT_S_GAME_START) },

        { PacketType.PKT_S_ROOM_DATA, (buffer) => PacketMaker<Protocol.S_ROOM_DATA>.HandlePacket(buffer, PacketType.PKT_S_ROOM_DATA) },

        { PacketType.PKT_S_ROOM_RESPONSE, (buffer) => PacketMaker<Protocol.S_ROOM_RESPONSE>.HandlePacket(buffer, PacketType.PKT_S_ROOM_RESPONSE) },

        { PacketType.PKT_S_MOVE, (buffer) => PacketMaker<Protocol.S_MOVE>.HandlePacket(buffer, PacketType.PKT_S_MOVE) },

        { PacketType.PKT_S_OBJECT_SPAWN, (buffer) => PacketMaker<Protocol.S_OBJECT_SPAWN>.HandlePacket(buffer, PacketType.PKT_S_OBJECT_SPAWN) },

        { PacketType.PKT_S_OBJECT_DEAD, (buffer) => PacketMaker<Protocol.S_OBJECT_DEAD>.HandlePacket(buffer, PacketType.PKT_S_OBJECT_DEAD) },

        { PacketType.PKT_S_OBJECT_DAMAGE, (buffer) => PacketMaker<Protocol.S_OBJECT_DAMAGE>.HandlePacket(buffer, PacketType.PKT_S_OBJECT_DAMAGE) },
    };

    // SendXXXX : PKT_C_XXX 패킷을 만들어주는 함수들로 함수의 다형성을 이용해 넘겨받은 매개변수의 자료형을 구분해 적절한 함수를 실행시킨다. IMessage형태의 값을 넘겨주면 자동으로 패킷을 만들어준다.

    /// <summary>
    /// PKT_C_LOGIN 패킷을 만들어주는 함수
    /// </summary>
    public static void Send(Protocol.C_RTT_PING pkt)
    {
        PacketMaker<Protocol.C_RTT_PING>.MakeSendBuffer(pkt, PacketType.PKT_C_RTT_PING);
    }
    public static void Send(Protocol.C_LOGIN pkt)
    {
        PacketMaker<Protocol.C_LOGIN>.MakeSendBuffer(pkt, PacketType.PKT_C_LOGIN);
    }
    public static void Send(Protocol.C_MOVE pkt)
    {
        PacketMaker<Protocol.C_MOVE>.MakeSendBuffer(pkt, PacketType.PKT_C_MOVE);
    }
    public static void Send(Protocol.C_ROOM_CREATE pkt)
    {
        PacketMaker<Protocol.C_ROOM_CREATE>.MakeSendBuffer(pkt, PacketType.PKT_C_ROOM_CREATE);
    }

    public static void Send(Protocol.C_ROOM_DATA pkt)
    {
        PacketMaker<Protocol.C_ROOM_DATA>.MakeSendBuffer(pkt, PacketType.PKT_C_ROOM_DATA);
    }
    public static void Send(Protocol.C_ROOM_REQUEST pkt)
    {
        PacketMaker<Protocol.C_ROOM_REQUEST>.MakeSendBuffer(pkt, PacketType.PKT_C_ROOM_REQUEST);
    }
    public static void Send(Protocol.C_ROOM_PLAYER_LIST_REQUEST pkt)
    {
        PacketMaker<Protocol.C_ROOM_PLAYER_LIST_REQUEST>.MakeSendBuffer(pkt, PacketType.PKT_C_ROOM_PLAYER_LIST_REQUEST);
    }

    public static void Send(Protocol.C_START_GAME pkt)
    {
        PacketMaker<Protocol.C_START_GAME>.MakeSendBuffer(pkt, PacketType.PKT_C_START_GAME);
    }

    public static void Send(Protocol.C_ATTACK pkt)
    {
        PacketMaker<Protocol.C_ATTACK>.MakeSendBuffer(pkt, PacketType.PKT_C_ATTACK);
    }

    /// <summary>
    /// 받은 패킷을 열어서 PacketType에 따른 적절한 Process 실행하는 함수
    /// </summary>
    /// <param name="message">서버로 부터 받은 패킷</param>
    /// <returns></returns>
    public static bool RecvPacket(byte[] message)
    {
        // PacketType 추출
        PacketType type = (PacketType)BitConverter.ToUInt16(message, sizeof(ushort));

        // type이 PacketType에 존재하는 값인지 검사
        if(!Enum.IsDefined(typeof(PacketType), type))
        {
            Debug.Log("Packet is not Definition");
            return false;
        }

        // Handlers(type별 실행해야될 핸들러 함수를 담아둔 딕셔너리)에서 type에 맞는 값을 찾아서 함수 실행시키기
        if(Handlers.TryGetValue(type, out var handler))
        {
            IMessage packet = handler.Invoke(message);
            Process(type, packet);
        }
        else
        {
            Debug.Log("존재하지 않는 패킷타입");
        }

        return true;
    }
}

public class PacketMaker<T> where T : IMessage<T>, new()
{
    // HandlePacket에서 제네릭 타입인 T의 역직렬화 기능을 이용하기 위해 MessageParser를 정적 메모리에 생성. 이때 생성자를 람다함수를 사용해 자료형 T를 생성하고 이를 MessageParser로 넘겨줌.
    private static readonly MessageParser<T> parser = new MessageParser<T>(() => new T());

    /// <summary>
    /// SendXXXX 함수로부터 실행되는 실제 패킷을 만드는 함수.
    /// PacketType에 따른 형식으로 패킷을 Serialize시켜주고 sendQueue에 넣어준다.
    /// </summary>
    /// <param name="pkt"> 직렬화 시켜줄 데이터 </param>
    /// <param name="type"> 직렬화 시킬 PacketType </param>
    public static void MakeSendBuffer(T pkt, PacketType type)
    {
        int size = pkt.CalculateSize();

        Debug.Log($"Serialized size: {size} bytes");

        byte[] payload = pkt.ToByteArray();
        ushort packetId = (ushort)type;
        ushort packetSize = (ushort)(sizeof(ushort) + sizeof(ushort) + payload.Length);

        using (MemoryStream ms = new MemoryStream())
        {
            ms.Write(BitConverter.GetBytes(packetSize), 0, 2);
            ms.Write(BitConverter.GetBytes(packetId), 0, 2);
            ms.Write(payload, 0, payload.Length);

            Debug.Log("보낸 패킷 정보 : " + BitConverter.ToString(ms.ToArray()));

            ServerConnect.Instance.EnqueueSendData(ms.ToArray());
        }
    }

    /// <summary>
    /// 서버로부터 받은 데이터를 IMessage 형식으로 DeSerialize 시켜주는 실제 함수.
    /// </summary>
    /// <param name="buffer"> 서버로 부터 받은 데이터 </param>
    /// <param name="type"> 해당 데이터의 PacketType </param>
    /// <returns> DeSerialize 시켜준 데이터 </returns>
    public static T HandlePacket(byte[] buffer, PacketType type)
    {
        ushort size = BitConverter.ToUInt16(buffer, 0);

        // packet에서 size와 packetId를 뺀 공간 확보
        byte[] payload = new byte[size - 4];
        // payload로 데이터 복사
        Array.Copy(buffer, 4, payload, 0, payload.Length);

        T data = parser.ParseFrom(payload);

        return data;
    }
}
