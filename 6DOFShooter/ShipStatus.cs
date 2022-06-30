using Photon.Pun;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ShipStatus : MonoBehaviour
{
    [HideInInspector]
    [Tooltip("Nombre de la nave. Se muestra en consola al recibir daño")]
    public string ShipID = "Test Ship";

    [Tooltip("Identificador de equipo, '0' no tiene equipo")]
    public int team = -1;

    [Tooltip("Vida de la nave, valor por defecto 1000")]
    [Range(0, 5000)]
    public float health = 1000.0f;

    [Tooltip("Escudo de la nave, valor por defecto 500")]
    [Range(0, 2500)]
    public float shield = 500.0f;

    [Tooltip("Tiempo de espera para que el escudo se empieze a recargar")]
    public float TimeForShieldRecharge = 2f;
    [Tooltip("Porcentaje de recarga del escudo en funcion al escudo total base")]
    public float shieldRechargeRate = 10f; 
    [Tooltip("Velocidad de la recarga de escudo. Ejemplo: 0.75 para 3/4 de segundo")]
    public float shieldRechargeSpeed = 0.2f;
    
    
    [Tooltip("Hace a la nave invulnerable")]
    public bool invulnerable = false;  

    float maxHealth, maxShield, damagelessCounter;

    float burnAmmount, burnSpeed, burnDuration;

    float healPerTick, healTickSpeed, healDuration;

    float trackDuration;

    int rechargeID = 0;

    bool damageTaken = false, burning = false, tracked = false;

    string randomName;

    MeshRenderer meshRenderer;

    Material traceMaterial;

    Material oldMat;

    // Meto el transforma para poder "reaparecer"
    [SerializeField]
    [Tooltip("El punto de spawn")]
    Transform[] spawns;

    PhotonView pV;

    void Start()
    {
        gameObject.tag = "Spaceship";

        maxHealth = health;

        maxShield = shield;

        //gameObject.name = ShipID;

        // El objeto "SpawnPoint" es el único con este tag (o debe serlo)
        //spawn = GameObject.FindGameObjectWithTag("Respawn").transform;

        traceMaterial = Resources.Load<Material>("Saturn");

        meshRenderer = GetComponent<MeshRenderer>();

        randomName = Random.Range(-100.0f, 100.0f).ToString();

        ShipID = randomName;

        gameObject.name = ShipID;
    }

    void Update()
    {
        if(team == -1)
        {
            // AsignTeam();
        }
        if(this.transform.parent == null)
        {
            transform.SetParent(GameObject.FindGameObjectWithTag("PadreJugadores").transform);
        }
    }

    public void DamageShip(float damage)
    {
        if (!invulnerable)
        {
            damageTaken = true;

            rechargeID++;

            StartCoroutine(RechargeShieldPrep(rechargeID));

            if (shield <= 0)
            {
                health -= damage;
            }
            else
            {
                shield -= damage;              

                if (shield < 0)
                {
                    health += shield;
                    shield = 0;
                }
            }
            if (health <= 0)
            {
            // No contempla respawn para "team" = 0 ó -1
            spawns = GameObject.FindGameObjectWithTag("Respawn").GetComponent<Spawns>().GetSpawnListTeam(team);

            if (spawns != null)
            {
                int spawn = Random.Range(0, spawns.Length);
            
                transform.position = spawns[spawn].position;
                transform.rotation = spawns[spawn].rotation;
                health = maxHealth;
                shield = maxShield;
            }
            else
            {
                transform.position = Vector3.zero;
                transform.rotation = Quaternion.identity;
                health = maxHealth;
                shield = maxShield;
            }
            }
        }
    }

    public void TrackShip(float duration)
    {
        trackDuration = duration;

        if (!tracked)
        {
            tracked = true;

            meshRenderer.material = traceMaterial;

            StartCoroutine(LoseTrack());
        }
    }

    public void BurnCheck(float ammount, float tickSpeed, float duration)
    {
        if (!burning)
        {
            burnAmmount = ammount;

            burnSpeed = tickSpeed;

            burnDuration = duration;

            StartCoroutine(BurnTicks());

            burning = true;
        }
        else
        {
            if (burnAmmount < ammount)
                burnAmmount = ammount;
            if (burnSpeed < tickSpeed)
                burnSpeed = tickSpeed;

            burnDuration = duration;
        }
    }

    public void HealShip(float otherHealAmount, bool otherTickHealing, float otherHealPerTick, float otherHealTickSpeed, float otherHealDuration)
    {
        if(health < maxHealth && (health + otherHealAmount <= maxHealth))
        {
            health += otherHealAmount;
        }
        else if((health + otherHealAmount) > maxHealth)
        {
            health = maxHealth;
        }

        if (otherTickHealing)
        {
            healPerTick = otherHealPerTick;

            healTickSpeed = otherHealTickSpeed;

            healDuration = otherHealDuration;

            StartCoroutine(HealTicks());
        }           
    }

    IEnumerator HealTicks()
    {
        yield return new WaitForSeconds(healTickSpeed);

        if (health < maxHealth && (health + healPerTick <= maxHealth))
        {
            health += healPerTick;
        }
        else if ((health + healPerTick) > maxHealth)
        {
            health = maxHealth;
        }

        if(healDuration > 0)
        {
            healDuration -= healTickSpeed;

            StartCoroutine(HealTicks());
        }
    }

    IEnumerator BurnTicks()
    {
        if (health > 0)
        {
            if (burnAmmount > 0)
            {
                damageTaken = true;
                rechargeID++;

                if (shield <= 0)
                {
                    health -= burnAmmount;
                }
                else
                {
                    shield -= burnAmmount;

                    if (shield < 0)
                    {
                        health += shield;
                        shield = 0;
                    }
                }

                yield return new WaitForSeconds(burnSpeed);

                burnDuration -= burnSpeed;

                if (burnDuration > 0)
                {
                    StartCoroutine(BurnTicks());
                }
                else
                {
                    StartCoroutine(RechargeShieldPrep(rechargeID));
                    burning = false;
                }
            }
        }
    }

    IEnumerator LoseTrack()
    {      
        yield return new WaitForSeconds(0.1f);
        if (trackDuration > 0)
        {
            trackDuration -= 0.1f;
            StartCoroutine(LoseTrack());
        }
        else
        {
            tracked = false;
            meshRenderer.material = oldMat;
        }
    }

    IEnumerator RechargeShieldPrep(int Id)
    {
        yield return new WaitForSeconds(TimeForShieldRecharge);

        if(Id == rechargeID)
        {
            damageTaken = false;

            rechargeID = 0;

            StartCoroutine(RechargeShield());
        }
    }

    IEnumerator RechargeShield()
    {  
        if (!damageTaken && shield < maxShield)
        {
            shield += (maxShield * shieldRechargeRate / 100);

            if(shield > maxShield)
            {
                shield = maxShield;
            }

            yield return new WaitForSeconds(shieldRechargeSpeed);

            StartCoroutine(RechargeShield());
        }            
    }

    // Asigna un equipo al jugador en caso de no tener ninguno
    void AsignTeam()
    {/*
        int[] teams = { 0, 0 };
        Transform jugadores = GameObject.FindGameObjectWithTag("PadreJugadores").transform;

        for (int i = 0; i < Photon.Pun.PhotonNetwork.PlayerList.Length; i++)
        {
            int temp = jugadores.GetChild(i).GetComponent<ShipStatus>().team;

            if (temp == 1)
            {
                teams[0]++;
            }
            else if (temp == 2)
            {
                teams[1]++;
            }
        }

        // Sólo si hay menos jugadores en el segundo equipo se le añadirá a ese equipo
        if(teams[0] >= teams[1])
        {
            team = 1;
        }
        else
        {
            team = 2;
        }
       */ 
    }
}
