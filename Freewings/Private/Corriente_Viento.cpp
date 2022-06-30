/** Copyright: @FreeWings
 *	Author: Daniel Trevisan Cohen-Henriquez
 *	Date: 20/04/2021
 */


#include "Corriente_Viento.h"
#include "Engine.h"
#include "Ala_delta.h"

ACorriente_Viento::ACorriente_Viento()
{

	PrimaryActorTick.bCanEverTick = true;

	collider = CreateDefaultSubobject<UBoxComponent>("Collider");
	collider->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}


void ACorriente_Viento::BeginPlay()
{
	Super::BeginPlay();

	collider->OnComponentBeginOverlap.AddDynamic(this, &ACorriente_Viento::OnTriggerEnter);
	
	collider->OnComponentEndOverlap.AddDynamic(this, &ACorriente_Viento::OnTriggerExit);

	pitch_angle = correct_angles(pitch_angle); //We check if any mistakes were made by inputing a larger angle than accepted

	roll_angle = correct_angles(roll_angle);
	
	if (roll_force < 0)
		roll_force = 0;
	if (pitch_force < 0)
		pitch_force = 0;

}

void ACorriente_Viento::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (inside_trigger)
		calculate_force();

}

void ACorriente_Viento::OnTriggerEnter(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor->IsA(AAla_delta::StaticClass())) { // Se comprueba que el objeto dentro del collider es de clase AAla_delta

		inside_trigger = true;

		ala_delta_ptr = Cast <AAla_delta>(OtherActor);
	}
 
}

void ACorriente_Viento::OnTriggerExit(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(AAla_delta::StaticClass())) { // Se comprueba que el objeto dentro del collider es de clase AAla_delta
		inside_trigger = false;

		ala_delta_ptr->wind_force.Z = 0.f;
	}
}

void ACorriente_Viento::calculate_force()
{
	ala_pitch = FMath::Abs(ala_delta_ptr->GetMesh()->GetComponentRotation().Roll); 

	ala_roll = FMath::Abs(ala_delta_ptr->GetMesh()->GetComponentRotation().Pitch);

	//Se reduce las fuerzas a aplicarse sobre el ala delta basandose en la angulacion de la misma. De ser alguno de sus angulos muy cercano a 90, ninguna... 
	//...fuerza sera aplicada, puesto a que el ala delta estaria en caida libre.

	if (split_method) // Las fuerzas estan separadas de tal manera que los angulos maximos que permiten dar fuerza al ala son distintos
	{			

		ala_delta_ptr->wind_force.Z = (reduce_force(reduce_force(pitch_force, pitch_angle, ala_pitch), maxAngle, ala_roll)
									+  reduce_force(reduce_force(roll_force, roll_angle, ala_roll), maxAngle, ala_pitch));
	}
	else // Las fuerzas estan juntas de tal manera que solo usa un angulo maximo que permite dar fuerza al ala delta
	{
		final_force = roll_force + pitch_force;

		if (pitch_angle <= roll_angle)
		{
			final_angle = roll_angle;
		}
		else
		{
			final_angle = pitch_angle;
		}

		ala_delta_ptr->wind_force.Z = reduce_force(reduce_force(final_force, final_angle, ala_pitch), final_angle, ala_roll); 
	}
}

float ACorriente_Viento::reduce_force(float force, float angle, float ala_angle)
{
	if(angle - ala_angle > 0) // De estar el ala delta dentro del angulo permitido se le aplicara una reduccion de fuerza basada en que tan cerca esta ese angulo del maximo
	{
		return force * ((angle - ala_angle) / angle);
	}
	else
	{
		return 0.f;
	}
}


float ACorriente_Viento::correct_angles(float angle) 
{
	if (angle < 0)
		angle = 0.f;
	if (angle > 90)
		angle = 90.f;

	return angle;
}