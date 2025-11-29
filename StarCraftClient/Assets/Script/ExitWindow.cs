using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ExitWindow : MonoBehaviour
{
    public void OnClickExitWindow()
    {
        transform.gameObject.SetActive(false);
    }
}
