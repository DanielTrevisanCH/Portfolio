using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponManager : MonoBehaviour {

    [Tooltip("GameObject Nave")] 
    public GameObject hull;
    [Tooltip("Disparo primario, Balas(Prefab: BasicBolt)")] 
	public GameObject shot;
    [Tooltip("Disparo sedundario, Misil(Prefab: Missile)")]  
    public GameObject shot2;

    float shotDelay = 0.2f, shotDelay2 = 1f;

    GameObject myShot, cannon, cannon2;

    Vector3 spawnPos;

    float spreadMax = 0.0f, spreadMax2 = 0.0f;

    public Vector3 shotSpawnOffset = new Vector3(0.67f, 0.15f, 1.25f);

    ShipStatus shipStatus;

    float timestamp = 0, timestamp2 = 0;

    [HideInInspector]
    public float ammo = 10, ammo2 = 10;

    [HideInInspector]
    public float maxAmmo = 100, maxAmmo2 = 100;

    bool stunned = false;

    bool disparo1, disparo2;

    void Start ()
    {
        shipStatus = GetComponent<ShipStatus>();

        cannon = new GameObject("Cannon");
        cannon.transform.parent = gameObject.transform;
        cannon.transform.position = new Vector3(hull.transform.position.x - shotSpawnOffset.x, hull.transform.position.y + shotSpawnOffset.y, hull.transform.position.z + shotSpawnOffset.z);

        cannon2 = new GameObject("Cannon2");
        cannon2.transform.parent = gameObject.transform;
        cannon2.transform.position = new Vector3(hull.transform.position.x + shotSpawnOffset.x, hull.transform.position.y + shotSpawnOffset.y, hull.transform.position.z + shotSpawnOffset.z);

        spreadMax = shot.GetComponent<BallisticWeapon>().spread;

        spreadMax2 = shot2.GetComponent<BallisticWeapon>().spread;

        maxAmmo = shot.GetComponent<BallisticWeapon>().ammo;       

        ammo = maxAmmo;

        maxAmmo2 = shot2.GetComponent<BallisticWeapon>().ammo;

        ammo2 = maxAmmo2;

        shotDelay = shot.GetComponent<BallisticWeapon>().fireRate;

        shotDelay2 = shot2.GetComponent<BallisticWeapon>().fireRate;
    }


    void Update ()
    {
        if (!stunned)
        {
            if (disparo1 && Time.time > timestamp && ammo != 0)
            {
                timestamp = Time.time + shotDelay;

                spawnPos = new Vector3(cannon.transform.position.x, cannon.transform.position.y, cannon.transform.position.z);

                myShot = Instantiate(shot, spawnPos, hull.transform.rotation);

                myShot.GetComponent<ShotOwner>().owner = shipStatus.ShipID;

                myShot.GetComponent<ShotOwner>().team = shipStatus.team;

                myShot.transform.Rotate(Random.Range(-spreadMax, spreadMax), Random.Range(-spreadMax, spreadMax), 0, Space.Self);

                ammo -= 1;
            }

            if (disparo2 && Time.time > timestamp2 && ammo2 != 0)
            {
                timestamp2 = Time.time + shotDelay2;

                spawnPos = new Vector3(cannon2.transform.position.x, cannon2.transform.position.y, cannon2.transform.position.z);

                myShot = Instantiate(shot2, spawnPos, hull.transform.rotation);

                myShot.GetComponent<ShotOwner>().owner = shipStatus.ShipID;

                myShot.GetComponent<ShotOwner>().team = shipStatus.team;

                myShot.transform.Rotate(Random.Range(-spreadMax2, spreadMax2), Random.Range(-spreadMax2, spreadMax2), 0, Space.Self);

                ammo2 -= 1;
            }
        }
    }

    public void IntercalarDisparoPrimario(bool disparo)
    {
        disparo1 = disparo;
    }

    public void IntercalarDisparoSecundario(bool disparo)
    {
        disparo2 = disparo;
    }

    public void StunShip(float time)
    {
        if (!stunned)
        {
            stunned = true;

            StartCoroutine(ReleaseStun(time));
        }
    }

    IEnumerator ReleaseStun(float time)
    {
        yield return new WaitForSeconds(time);

        stunned = false;
    }

    public void Actualizar_Offset(Vector3 nuevoOffset)
    {
        cannon.transform.position = new Vector3(hull.transform.position.x - nuevoOffset.x, hull.transform.position.y + nuevoOffset.y, hull.transform.position.z + nuevoOffset.z);
        cannon2.transform.position = new Vector3(hull.transform.position.x + nuevoOffset.x, hull.transform.position.y + nuevoOffset.y, hull.transform.position.z + nuevoOffset.z);
    }
}
