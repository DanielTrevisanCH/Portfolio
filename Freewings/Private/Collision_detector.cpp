/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		marzo 2021
  */

#include "Collision_detector.h"
#include "Overlap_actor.h"
#include "Engine/World.h"
#include "Ala_delta.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

void print(FString message);

void ACollision_detector::add_weight(const FString& name, float weight)
{
	if (name == "Left") left_weight += weight;
	else if (name == "Right") right_weight += weight;
	else if (name == "Up") up_weight += weight;
	else if (name == "Down") down_weight += weight;

	take_action(); //Tras anadir un peso se vuelve a actualizar la direccion de giro por si deberia cambiar
}

void ACollision_detector::take_action()
{
	// Se maneja el giro horizotal (left-right)
	if (left_weight || right_weight)
	{
		// Si el peso izquierda/derecha esta balanceado y ya estaba girando en una de esas dos direcciones entonces se continuara con la misma direccion
		if (left_weight == right_weight && (ala_delta_ptr->hor_dir == AAla_delta::Hor_dir::Left || ala_delta_ptr->hor_dir == AAla_delta::Hor_dir::Right))
			return;

		// Se ajusta el giro en funcion del contacto con los triggers / pesos
		if (left_weight > right_weight) ala_delta_ptr->go_right();
		else ala_delta_ptr->go_left();

		//Para que gire horizontalmente el ala tambien debe inclinarse
		if (ala_delta_ptr->vert_dir == AAla_delta::Vert_dir::Down)
			ala_delta_ptr->go_down();
		else ala_delta_ptr->go_up();
	}

	// Se maneja el giro vertical (top down)
	if (up_weight || down_weight)
	{
		// Si la el peso arriba/abajo esta balanceado y ya estaba girando en una de esas dos direcciones entonces se continuara con la misma direccion
		if (up_weight == down_weight && ((ala_delta_ptr->vert_dir == AAla_delta::Vert_dir::Down || ala_delta_ptr->vert_dir == AAla_delta::Vert_dir::Up))) return;

		// Se ajusta el giro arriba/ abajo en funcion de los triggers / pesos
		if (up_weight > down_weight) ala_delta_ptr->go_up();
		else ala_delta_ptr->go_down();
	}
	// Si no hay pesos entonces el ala continua adelante
	else if (!left_weight && !right_weight) ala_delta_ptr->go_straight();
}

ACollision_detector::ACollision_detector()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACollision_detector::BeginPlay()
{
	Super::BeginPlay();
	create_triggers(); // Crea todos los triggers que se encargan de comprobar el overlap
}

void ACollision_detector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//TODO: Convertir las distancias y valores en variables

	FVector forward{ ala_delta_ptr->GetActorForwardVector() };
	forward.Z = 0;

	// Se actualiza el transform del left trigger
	FVector look_pos{ ala_delta_ptr->GetActorLocation() + forward * 400 + ala_delta_ptr->GetActorUpVector() * 100 + ala_delta_ptr->GetActorRightVector() * -100 };
	FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(ala_delta_ptr->GetActorLocation(), look_pos);
	FVector dir{ UKismetMathLibrary::GetDirectionUnitVector(ala_delta_ptr->GetActorLocation(), look_pos) };
	FVector location = ala_delta_ptr->GetActorLocation() + dir * 200;
	left_trigger->SetActorLocation(location);
	left_trigger->SetActorRotation(PlayerRot);

	// Se actualiza el transform del right trigger
	look_pos = ala_delta_ptr->GetActorLocation() + forward * 400 + ala_delta_ptr->GetActorUpVector() * 100 + ala_delta_ptr->GetActorRightVector() * 100;
	PlayerRot = UKismetMathLibrary::FindLookAtRotation(ala_delta_ptr->GetActorLocation(), look_pos);
	dir = UKismetMathLibrary::GetDirectionUnitVector(ala_delta_ptr->GetActorLocation(), look_pos);
	location = ala_delta_ptr->GetActorLocation() + dir * 200;
	right_trigger->SetActorLocation(location);
	right_trigger->SetActorRotation(PlayerRot);

	// Se actualiza el transform del down trigger
	look_pos = ala_delta_ptr->GetActorLocation() + forward * 400 + ala_delta_ptr->GetActorUpVector() * 200;
	PlayerRot = UKismetMathLibrary::FindLookAtRotation(ala_delta_ptr->GetActorLocation(), look_pos);
	dir = UKismetMathLibrary::GetDirectionUnitVector(ala_delta_ptr->GetActorLocation(), look_pos);
	location = ala_delta_ptr->GetActorLocation() + dir * 200;
	down_trigger->SetActorLocation(location);
	down_trigger->SetActorRotation(PlayerRot);

	// Se actualiza el transform del up trigger
	look_pos = ala_delta_ptr->GetActorLocation() + forward * 400;
	PlayerRot = UKismetMathLibrary::FindLookAtRotation(ala_delta_ptr->GetActorLocation(), look_pos);
	dir = UKismetMathLibrary::GetDirectionUnitVector(ala_delta_ptr->GetActorLocation(), look_pos);
	location = ala_delta_ptr->GetActorLocation() + dir * 200;
	up_trigger->SetActorLocation(location);
	up_trigger->SetActorRotation(PlayerRot);
}

void ACollision_detector::set_ala_delta_ptr(AAla_delta* ala_ptr)
{
	ala_delta_ptr = ala_ptr;
}

void ACollision_detector::create_triggers()
{
	FActorSpawnParameters SpawnInfo;

	left_trigger = GetWorld()->SpawnActor<AOverlap_actor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	left_trigger->set_size(FVector(200, 50, 10));
	left_trigger->set_name("Left");
	left_trigger->set_detector(this);

	right_trigger = GetWorld()->SpawnActor<AOverlap_actor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	right_trigger->set_size(FVector(200, 50, 10));
	right_trigger->set_name("Right");
	right_trigger->set_detector(this);

	down_trigger = GetWorld()->SpawnActor<AOverlap_actor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	down_trigger->set_size(FVector(200, 10, 50));
	down_trigger->set_name("Down");
	down_trigger->set_detector(this);

	up_trigger = GetWorld()->SpawnActor<AOverlap_actor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	up_trigger->set_size(FVector(200, 10, 50));
	up_trigger->set_name("Up");
	up_trigger->set_detector(this);
}