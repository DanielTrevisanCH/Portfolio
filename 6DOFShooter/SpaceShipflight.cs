using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SpaceShipflight : MonoBehaviour {

	internal float ControlHorizontal; 
	internal float ControlVertical; 

    float altitudeControl;
    float pitch = 0f, yaw = 0f, roll = 0f;


	[Tooltip("Velocidad de guiñada. Valor por defecto 2")] 
    public float speedYaw = 2.0f;
	[Tooltip("Velocidad de cabeceo. Valor por defecto 2")] 
	public float speedPitch = 2.0f;
	[Tooltip("Velocidad de alabeo. Valor por defecto 2")] 
	public float speedRoll = 2.0f;
	[Tooltip("Velocidad elevacion y bajada. Valor por defecto 1")]  
	public float speedRaise = 1.0f;
	[Tooltip("Velocidad frontal y posterior. Valor por defecto 1")] 
	public float speedMove = 1.0f;
	[Tooltip("Velocidad lateral. Valor por defecto 1")] 
	public float speedStrafe = 1.0f;
	[Tooltip("Aceleración de turbo. Valor por defecto 1")] 
	public float turboSpeed = 1.0f;
	[Tooltip("Velocidad de turbo MAXIMA. Valor por defecto 10")] 
	public float maxTurboSpeed = 10.0f;

	private float defaultMoveSpeed = 0.0f;

	private Rigidbody rb;

	// Use this for initialization
	void Start () {
		rb = gameObject.GetComponent<Rigidbody> ();

		Cursor.lockState = CursorLockMode.Locked;

		defaultMoveSpeed = speedMove;

    }

    void FixedUpdate () {

		if(Input.GetKey(KeyCode.LeftControl) && speedMove <= maxTurboSpeed)
		{
			speedMove += turboSpeed;
		}
		else if (speedMove > defaultMoveSpeed)
		{	
			speedMove -= turboSpeed;
		}

		ControlHorizontal = speedStrafe * Input.GetAxis ("Horizontal");
		ControlVertical = speedMove * Input.GetAxis ("Vertical");

        altitudeControl = -speedRaise * Input.GetAxis("Raise");

        yaw = speedYaw * Input.GetAxis("Mouse X");
        pitch = -speedPitch * Input.GetAxis("Mouse Y");
        roll = speedRoll * Input.GetAxis("Spin");


        rb.AddRelativeForce(0f, altitudeControl,0f,  ForceMode.VelocityChange);
        rb.AddRelativeForce(0f, 0f, ControlVertical, ForceMode.VelocityChange);
        rb.AddRelativeForce(ControlHorizontal, 0f, 0f, ForceMode.VelocityChange);

        rb.AddRelativeTorque(pitch, yaw, roll, ForceMode.Force);
	}
}
