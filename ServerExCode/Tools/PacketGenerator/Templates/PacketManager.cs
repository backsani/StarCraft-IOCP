using Google.Protobuf;
using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;

public enum PacketType
{
{% for pkt in parser.total_pkt %}
    { { pkt.name} } = { { pkt.id} },
{% endfor %}
}

public static class PacketManager
{
    /// <summary>
    /// PacketType에 맞는 핸들러 함수를 O(1)로 실행시키기 위해 딕셔너리에 저장. buffer만 넘겨주고 실행시켜주면 IMessage형으로 반환해준다.
    /// </summary>
    private static Dictionary<PacketType, Func<byte[], IMessage>> Handlers = new Dictionary<PacketType, Func<byte[], IMessage>>
    {
{% for pkt in parser.recv_pkt %}
        { PacketType.PKT_{{pkt.name } }, (buffer) => PacketMaker<Protocol.{{pkt.name }}>.HandlePacket(buffer, PacketType.PKT_{ { pkt.name} })} { { "," if not loop.last else ""} }
{% endfor %}
    };

// SendXXXX : PKT_C_XXX 패킷을 만들어주는 함수들로 함수의 다형성을 이용해 넘겨받은 매개변수의 자료형을 구분해 적절한 함수를 실행시킨다. IMessage형태의 값을 넘겨주면 자동으로 패킷을 만들어준다.

/// <summary>
/// PKT_C_LOGIN 패킷을 만들어주는 함수
/// </summary>
/// 
{% for pkt in parser.send_pkt %}
    public static void Send(Protocol.{{pkt.name}} pkt)
    {
        PacketMaker<Protocol.{{pkt.name}}>.MakeSendBuffer(pkt, PacketType.PKT_{ { pkt.name } });
    }
{% endfor %}

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
        if (!Enum.IsDefined(typeof(PacketType), type))
        {
            Debug.Log("Packet is not Definition");
            return false;
        }

        // Handlers(type별 실행해야될 핸들러 함수를 담아둔 딕셔너리)에서 type에 맞는 값을 찾아서 함수 실행시키기
        if (Handlers.TryGetValue(type, out var handler))
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
