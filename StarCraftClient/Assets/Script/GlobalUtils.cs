using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public static class GlobalUtils
{
    // ProtoBuf 오류로 bytes 사용이 불가능해 문자열을 uint32에 압축해서 전송
    public static uint[] PackStringToBytes(string str)
    {
        byte[] bytes = System.Text.Encoding.ASCII.GetBytes(str);

        int uintCount = (int)Math.Ceiling(bytes.Length / 4f);
        uint[] result = new uint[uintCount];

        int index = 0;

        for (int i = 0; i < uintCount; i++)
        {
            uint packed = 0;

            for (int b = 0; b < 4; b++)
            {
                packed <<= 8;

                if (index < bytes.Length)
                    packed |= bytes[index];
                else
                    packed |= 0;

                index++;
            }

            result[i] = packed;
        }

        return result;
    }

    // ProtoBuf 오류로 bytes 사용이 불가능해 압축해서 받은 uint32를 문자열로 언패킹
    public static string UnpackBytesToString(IEnumerable<uint> packedData)
    {
        List<byte> byteList = new List<byte>();

        foreach (uint value in packedData)
        {
            byte b1 = (byte)((value >> 24) & 0xFF);
            byte b2 = (byte)((value >> 16) & 0xFF);
            byte b3 = (byte)((value >> 8) & 0xFF);
            byte b4 = (byte)(value & 0xFF);

            byteList.Add(b1);
            byteList.Add(b2);
            byteList.Add(b3);
            byteList.Add(b4);
        }

        while (byteList.Count > 0 && byteList[byteList.Count - 1] == 0)
        {
            byteList.RemoveAt(byteList.Count - 1);
        }

        return System.Text.Encoding.ASCII.GetString(byteList.ToArray());
    }
}
