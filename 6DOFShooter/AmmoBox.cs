using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AmmoBox : MonoBehaviour
{
    public float respawnTimer = 0.5f;

    public bool resupplyPrimary;

    public bool resupplySecondary;

    [ConditionalHide("resupplyPrimary", true)]
    public float resupplyPrimaryPercentage;

    [ConditionalHide("resupplySecondary", true)]
    public float resupplySecondaryPercent;

    WeaponManager weaponManager;

    MeshRenderer meshRenderer;

    bool used;

    int ammoState = 0;

    private void Start()
    {
        if (resupplyPrimary)
            ammoState++;
        if (resupplySecondary)
            ammoState += 2;

        meshRenderer = gameObject.GetComponent<MeshRenderer>();
    }

    private void OnTriggerEnter(Collider collider)
    {     
        if(!used)

        if (collider.GetComponent<WeaponManager>() != null)
        {
            weaponManager = collider.GetComponent<WeaponManager>();

            switch (ammoState)
            {
                case 1:

                    if(weaponManager.ammo + (weaponManager.maxAmmo * resupplyPrimaryPercentage / 100) < weaponManager.maxAmmo)
                    {
                        weaponManager.ammo += weaponManager.maxAmmo * resupplyPrimaryPercentage / 100;
                    }                     
                    else
                    {
                        weaponManager.ammo = weaponManager.maxAmmo;
                    }
                    break;
                case 2:
                    if (weaponManager.ammo2 + (weaponManager.maxAmmo2 * resupplySecondaryPercent / 100) < weaponManager.maxAmmo2)
                    {
                        weaponManager.ammo2 += weaponManager.maxAmmo2 * resupplySecondaryPercent / 100;
                    }
                    else
                    {
                        weaponManager.ammo2 = weaponManager.maxAmmo2;
                    }                   
                    break;
                case 3:

                    if (weaponManager.ammo + (weaponManager.maxAmmo * resupplyPrimaryPercentage / 100) < weaponManager.maxAmmo)
                    {
                        weaponManager.ammo += weaponManager.maxAmmo * resupplyPrimaryPercentage / 100;
                    }
                    else
                    {
                        weaponManager.ammo = weaponManager.maxAmmo;
                    }
                    if (weaponManager.ammo2 + (weaponManager.maxAmmo2 * resupplySecondaryPercent / 100) < weaponManager.maxAmmo2)
                    {
                        weaponManager.ammo2 += weaponManager.maxAmmo2 * resupplySecondaryPercent / 100;
                    }
                    else
                    {
                        weaponManager.ammo2 = weaponManager.maxAmmo2;
                    }
                    break;
            }

            StartCoroutine(respawnBox(respawnTimer));
        }
    }


    IEnumerator respawnBox(float time)
    {
        used = true;

        meshRenderer.enabled = false;

        yield return new WaitForSeconds(time);

        meshRenderer.enabled = true;

        used = false;
    }
}
