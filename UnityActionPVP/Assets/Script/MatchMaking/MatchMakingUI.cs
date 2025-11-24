using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class MatchMakingUI : MonoBehaviour
{
    [SerializeField] private TextMeshProUGUI mUserNameText;
    [SerializeField] private TextMeshProUGUI mTimer;

    private float timer = 0f;
    // Start is called before the first frame update
    void Start()
    {
        mUserNameText.text = ServerConnect.Instance.UserId;
    }

    // Update is called once per frame
    void Update()
    {
        timer += Time.deltaTime;
        mTimer.text = "Time : " + timer.ToString("F1");
    }
}
