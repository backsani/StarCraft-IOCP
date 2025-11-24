using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class PlayerUnit : Unit
{
    private Slider hpSlider;
    private Transform hpSliderCanvas;

    private float maxHp = 100f;
    private Vector3 offset = new Vector3(0, 1f, 0);

    private void Update()
    {
        hpSliderCanvas.position = transform.position + offset;
        hpSliderCanvas.forward = Camera.main.transform.forward;
    }

    public void InitHealthBar(Transform sliderCanvas)
    {
        hpSlider = sliderCanvas.GetChild(0).GetComponent<Slider>();
        hpSliderCanvas = sliderCanvas;
        hpSlider.value = 1f;
    }

    public override void SetHp(float hp)
    {
        health = Mathf.Clamp(hp, 0f, maxHp);

        if(hpSlider != null)
        {
            hpSlider.value = health / maxHp;
        }
        else
        {
            Debug.Log("hpSlider∞° null¿Ã¥Ÿ");
        }
    }
}
