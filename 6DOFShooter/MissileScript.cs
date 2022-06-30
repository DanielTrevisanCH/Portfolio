using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MissileScript : MonoBehaviour
{

    [Tooltip("Velocidad del misil, valor por defecto 50")] 
    public float speed = 0.0f;
    [Tooltip("Daño del misil, valor por defecto 300")] 
    public float damage = 0.0f;
    [Tooltip("Radio de la explosion, valor por defecto 50")] 
    public float explosionRadius = 0.0f;
    [Tooltip("Tiempo de vida del misil, valor por defecto 5")] 
    public float lifespan = 0.0f;

    public int team;

    Rigidbody rb;

    GameObject explosion;

    ShipStatus shipStatus;

    // Use this for initialization
    void Start()
    {

        Destroy (gameObject, lifespan);

        rb = GetComponent<Rigidbody>();
    }


    void FixedUpdate()
    {
        rb.AddRelativeForce(0f, 0f, speed, ForceMode.VelocityChange);
    }

    private void OnCollisionEnter(Collision collision)
    {

        rb.constraints = RigidbodyConstraints.FreezeAll;
        

        Collider[] collider = Physics.OverlapSphere(transform.position, explosionRadius);

        foreach (Collider nearbyObject in collider)
        {
            if (nearbyObject.gameObject.tag == "Spaceship" &&
                collision.gameObject.GetComponent<ShipStatus>().team != this.GetComponent<ShotOwner>().team)
            {
                shipStatus = nearbyObject.GetComponent<ShipStatus>();

                shipStatus.DamageShip(damage);               
            }
        }
        
        Destroy(gameObject);
    }
}
