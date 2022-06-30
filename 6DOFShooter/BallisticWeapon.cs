using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public class BallisticWeapon : MonoBehaviour
{
    public string weaponID = "Test Weapon";

    //Regular Weapon Values
    #region
    [Header("Standard Weapon Values")]

    [Tooltip("Daño por disparo en impacto")]
    [Range(0, 5000)]
    public float damage = 0;
    [Tooltip("Cadencia de tiro")]
    [Range(0.01f, 60)]
    public float fireRate = 0.01f;
    [Tooltip("Velocidad del disparo")]
    [Range(0, 100)]
    public float speed = 100;
    [Tooltip("Rango de dispersión del foco del disparo")]
    [Range(0, 10)]
    public float spread = 0.0f;
    [Tooltip("Tiempo de vida del disparo. Superado este, el disparo desaparece")]
    [Range(1, 10)]
    public float lifespan = 1;
    [Tooltip("Munición total que puede llevar el arma y con la cual dispondra al spawnear")]
    [Range(0, 10000)]
    public int ammo = 100;
    #endregion

    //Homing

    //Explosives
    #region
    [Header("Explosions")]

    [Tooltip("Concede atributo explosivo")]
    public bool explosive = false;
    [Tooltip("Rango de la explosion. Es usado tambien para explosiones con atributo")]
    [ConditionalHide("explosive", true)]
    public float explosionRange = 1.0f;
    [Tooltip("Daño de la explosion")]
    [ConditionalHide("explosive", true)]
    public float explosionDamage = 0.0f;
    #endregion

    //Special properties
    #region

    [Header("Healing")]
    [Tooltip("Concede la habilidad de curación")]
    public bool canHeal = false;
    [Tooltip("Cantidad de curación al impacto")]
    [ConditionalHide("canHeal", true)]
    public float healAmount = 100;
    [Tooltip("Concede la habilidad de curar por ticks")]
    [ConditionalHide("canHeal", true)]
    public bool tickHealing = false;
    [Tooltip("Cantidad de curación por tick")]
    [ConditionalHide("tickHealing", true)]
    public float healPerTick = 0;
    [Tooltip("Velocidad por segundo de curacion. (Ticks)")]
    [ConditionalHide("tickHealing", true)]
    public float healTickSpeed = 0.1f;
    [Tooltip("Duración de los ticks")]
    [ConditionalHide("tickHealing", true)]
    public float healDuration = 1;

    [Header("Burn")]
    [Tooltip("Concede el atributo de fuego")]
    public bool causesBurn = false;
    [Tooltip("Daño de fuego por tick")]
    [ConditionalHide("causesBurn", true)]
    public float burnDamagePerTick = 0.0f;
    [Tooltip("Velocidad por segundo del daño de fuego. (Ticks)")]
    [ConditionalHide("causesBurn", true)]
    public float burnTickSpeed = 0.1f;
    [Tooltip("Duracion del fuego en segundos")]
    [ConditionalHide("causesBurn", true)]
    public float burnDuration = 100.0f;

    [Header("Slow")]
    [Tooltip("Concede el atributo de ralentizar")]
    public bool causesSlow = false;
    [Tooltip("Fuerza del ralentamiento del movimiento en porcentaje. EJ: '20' = 20%")]
    [ConditionalHide("causesSlow", true)]
    public float slowPercentMovement = 0.0f;
    [Tooltip("Fuerza del ralentamiento del movimiento de camara en porcentaje. EJ: '20' = 20%")]
    [ConditionalHide("causesSlow", true)]
    public float slowPercentRotation = 0.0f;
    [Tooltip("Duracion del ralentamiento en segundos")]
    [ConditionalHide("causesSlow", true)]
    public float slowDuration = 0.0f;

    [Header("Stun")]
    [Tooltip("Concede el atributo de aturdir")]
    public bool canStun = false;
    [Tooltip("Duracion de aturdimiento en segundos")]
    [ConditionalHide("canStun", true)]
    public float stunDuration = 0.0f;

    [Header("Tracking")]
    [Tooltip("Concede el atributo de rastrear")]
    public bool canTrack = false;
    [Tooltip("Duracion del rastreo")]
    [ConditionalHide("canTrack", true)]
    public float trackDuration;
    #endregion 

    //VFX   

    #region
    [Header("Target Homing (In Development. Don't use)")]

    public bool homing;
    [ConditionalHide("homing", true)]
    public float turnRate;
    [ConditionalHide("homing", true)]
    public Transform target;
    #endregion

    [Header("VFX (In Development. Don't use)")]
    public GameObject graficosBala;
    public GameObject fxSalida;
    public GameObject fxImpacto;

    string ownerName;

    int team;

    Rigidbody rb;

    ShipStatus shipStatus;

    ShotOwner shotOwner;

    NuevosInputs nuevosInputs;

    WeaponManager weaponManager;

    DisparoTemporalParaOnline disparoTemporalParaOnline;

    private void Start()
    {
        gameObject.tag = "Projectile";

        Destroy(gameObject, lifespan);

        shotOwner = GetComponent<ShotOwner>();

        team = shotOwner.team;

        ownerName = shotOwner.owner;

        rb = GetComponent<Rigidbody>();

        Physics.IgnoreCollision(gameObject.GetComponentInParent<Collider>(), GameObject.Find(ownerName).GetComponentInParent<Collider>());
    }

    private void FixedUpdate()
    {
        rb.AddRelativeForce(0f, 0f, speed, ForceMode.VelocityChange);

        if (homing)
        {
            var targetRotation = Quaternion.LookRotation(target.position - transform.position);

            rb.MoveRotation(Quaternion.RotateTowards(transform.rotation, targetRotation, turnRate));
        }
    }

    private void OnCollisionEnter(Collision collision)
    {    
        if (explosive)
        {
            if (collision.gameObject.tag == "Spaceship")
            {
                shipStatus = collision.gameObject.GetComponent<ShipStatus>();

                if (damage > 0)
                {
                    shipStatus.DamageShip(damage);
                }
            }

            Collider[] collider = Physics.OverlapSphere(transform.position, explosionRange);

            foreach (Collider nearbyObject in collider)
            {
                if (nearbyObject.gameObject.tag == "Spaceship")
                {
                    shipStatus = nearbyObject.GetComponent<ShipStatus>();

                    nuevosInputs = nearbyObject.GetComponent<NuevosInputs>();

                    if(nearbyObject.GetComponent<WeaponManager>() == null)
                    {
                    disparoTemporalParaOnline = nearbyObject.GetComponent<DisparoTemporalParaOnline>();
                    }
                    else
                    {
                    weaponManager = nearbyObject.GetComponent<WeaponManager>();
                    }

                    if (canHeal && team == shipStatus.team)
                    {
                        shipStatus.HealShip(healAmount, tickHealing, healPerTick, healTickSpeed, healDuration);
                    }
                    else if (team != shipStatus.team)
                    {
                        if (explosionDamage > 0)
                        {
                            shipStatus.DamageShip(explosionDamage);
                        }

                        if (causesBurn)
                        {
                            shipStatus.BurnCheck(burnDamagePerTick, burnTickSpeed, burnDuration);
                        }

                        if (canTrack)
                        {
                            shipStatus.TrackShip(trackDuration);
                        }

                        if (causesSlow)
                        {
                            nuevosInputs.RalentizarPotencia(slowPercentMovement, slowPercentRotation, slowDuration);
                        }

                        if (canStun)
                        {
                            nuevosInputs.RalentizarPotencia(100f, 100f, stunDuration);

                            if (weaponManager == null)
                            {
                                disparoTemporalParaOnline.StunShip(stunDuration);
                            }
                            else
                            {
                                weaponManager.StunShip(stunDuration);
                            }
                        }
                    }
                }
            }
                Destroy(gameObject);
        }
        else
        {
            if (collision.gameObject.tag == "Spaceship")
            {
                shipStatus = collision.gameObject.GetComponent<ShipStatus>();

                nuevosInputs = collision.gameObject.GetComponent<NuevosInputs>();

                if(collision.gameObject.GetComponent<WeaponManager>() == null)
                {
                    disparoTemporalParaOnline = collision.gameObject.GetComponent<DisparoTemporalParaOnline>();
                }
                else
                {
                    weaponManager = collision.gameObject.GetComponent<WeaponManager>();
                }

                if (canHeal && team == shipStatus.team)
                {
                    shipStatus.HealShip(healAmount, tickHealing, healPerTick, healTickSpeed, healDuration);
                }
                else if (team != shipStatus.team)
                {

                    if (damage > 0)
                    {
                        shipStatus.DamageShip(damage);
                    }

                    if (causesBurn)
                    {
                        shipStatus.BurnCheck(burnDamagePerTick, burnTickSpeed, burnDuration);
                    }

                    if (canTrack)
                    {
                        shipStatus.TrackShip(trackDuration);
                    }

                    if (causesSlow)
                    {
                        nuevosInputs.RalentizarPotencia(slowPercentMovement, slowPercentRotation, slowDuration);
                    }
                    if (canStun)
                    {
                        nuevosInputs.RalentizarPotencia(100f, 100f, stunDuration);

                        if (weaponManager == null)
                        {
                            disparoTemporalParaOnline.StunShip(stunDuration);
                        }
                        else
                        {
                            weaponManager.StunShip(stunDuration);
                        }
                    }
                }
            }              
        }            
        Destroy(gameObject);
    }
}
