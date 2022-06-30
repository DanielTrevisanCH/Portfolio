using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PwrUpManager : MonoBehaviour
{
    public bool ammoBoxes;

    GameObject[] boxes;

    AmmoBox[] boxesScripts;

    void Start()
    {
        if (ammoBoxes)
            boxes = GameObject.FindGameObjectsWithTag("AmmoBox");
    }

    void Update()
    {
        
    }
}
