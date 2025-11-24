using Google.Protobuf;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using TMPro;
using UnityEngine;
using UnityEngine.Windows;

public class LoginManager : MonoBehaviour
{
    private static LoginManager instance;
    public static LoginManager Instance
    {
        get
        {
            // 인스턴스가 없으면 생성
            if (instance == null)
            {
                instance = FindObjectOfType<LoginManager>();

                // 인스턴스가 씬에 없다면 새로 생성
                if (instance == null)
                {
                    GameObject go = new GameObject("ServerConnect");
                    instance = go.AddComponent<LoginManager>();
                }
            }

            return instance;
        }
    }

    [SerializeField] private TMP_InputField _userId;
    [SerializeField] private GameObject _failMessage;

    public void TryLogin()
    {
        Protocol.C_LOGIN login = new Protocol.C_LOGIN();

        if (ulong.TryParse(_userId.text, out ulong value))
        {
            Debug.Log($"변환 성공: {value}");
            login.LoginCode = value;
        }
        else
        {
            Debug.LogError("변환 실패: 유효한 숫자 아님");
            return;
        }

        PacketManager.Send(login);
    }

    public void FailLogin()
    {
        _failMessage.SetActive(true);
    }

    public void CloseBoard()
    {
        _failMessage.SetActive(false);
    }
}
