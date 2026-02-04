using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using System.Security.Cryptography;
using System.Reflection;
using UnityEditor.U2D.Aseprite;
using UnityEngine.UIElements;

public static class GlobalUtils
{
    // 맵 주소를 통해 map의 Hash를 추출해주는 함수
    public static bool ExtractionMapHash(string path, out byte[] bytes)
    {
        if (!File.Exists(path))
        {
            Debug.LogError($"File not found: {path}");
            bytes = new byte[0];
            return false;
        }

        byte[] data = File.ReadAllBytes(path);

        //Dictionary<MapSection, byte[]> dic;

        //ExtractionMapSection(data, out dic);
        int index = 0;

        MapSection section = (MapSection)BitConverter.ToUInt16(data, 0);

        if (section != MapSection.HASH)
        {
            Debug.LogError($"File Section Hash Invaild");
            bytes = new byte[0];
            return false;
        }
        index += sizeof(ushort);

        Int32 length = BitConverter.ToInt32(data, index);
        index += sizeof(Int32);

        byte[] hash = new byte[length];

        Buffer.BlockCopy(data, index, hash, 0, length);

        bytes = hash;

        return true;
    }


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

    // 맵 data를 섹션별로 나누어서 Dictionary로 반환
    public static void ExtractionMapSection(byte[] mapData, out Dictionary<MapSection, byte[]> dictionary)
    {
        dictionary = new Dictionary<MapSection, byte[]>();

        int index = 0;
        while(index < mapData.Length)
        {
            MapSection section = (MapSection)BitConverter.ToUInt16(mapData, index);
            index += sizeof(ushort);

            Int32 length = BitConverter.ToInt32(mapData, index);
            index += sizeof(Int32);

            if(index + length > mapData.Length)
            {
                Debug.Log("Invalid section length" + section.ToString());
                return;
            }

            Debug.Log(section.ToString());

            byte[] data = new byte[length];
            Buffer.BlockCopy(mapData, index, data, 0, length);
            index += length;

            dictionary[section] = data;
        }
    }
}
