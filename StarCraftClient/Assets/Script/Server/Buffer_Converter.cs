using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class Buffer_Converter
{
    public int GetHeaderType(byte[] buffer)
    {
        byte[] dataHeaderType = new byte[sizeof(int)];

        Buffer.BlockCopy(buffer, Marshal.SizeOf(typeof(int)), dataHeaderType, 0, dataHeaderType.Length);

        return BitConverter.ToInt32(dataHeaderType);
    }

    public int GetLength(byte[] buffer)
    {
        byte[] dataLength = new byte[sizeof(int)];

        Buffer.BlockCopy(buffer, 0, dataLength, 0, dataLength.Length);

        return BitConverter.ToInt32(dataLength);
    }
}
