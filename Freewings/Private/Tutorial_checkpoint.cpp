/** Copyright: @FreeWings
 *	Author: Daniel Trevisan Cohen-Henriquez
 *	Date: 07/05/2021
 */

#include "Tutorial_checkpoint.h"
#include "Ala_delta.h"

ATutorial_checkpoint::ATutorial_checkpoint() // Constructor de los checkpoints
{
	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	mesh->BodyInstance.SetCollisionProfileName("NoCollision");

	collider = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collider"));
	collider->AttachToComponent(mesh, FAttachmentTransformRules::KeepRelativeTransform);
	collider->BodyInstance.SetCollisionProfileName("OverlapAll");
	collider->OnComponentBeginOverlap.AddDynamic(this, &ATutorial_checkpoint::OnTriggerEnter);
	collider->OnComponentEndOverlap.AddDynamic(this, &ATutorial_checkpoint::OnTriggerExit);

	state = State::Initialize;
}

void ATutorial_checkpoint::BeginPlay()
{
	Super::BeginPlay();

	Material = mesh->GetMaterial(0);

	if (previous_Checkpoints.Num() <= 0 && state == State::Initialize) // Se comprueba cual es el primer checkpoint de la lista y se asigna a si mismo como el primer objetivo
	{
		if (sibling_Checkpoints.Num() > 0) {

			alter_state(sibling_Checkpoints, true, material_current_waypoint, State::Current);
		}

		collider->SetMaterial(0, material_current_waypoint);
		collider->SetVisibility(true);
		state = State::Current;

		if (next_Checkpoints.Num() > 0) { // Se le indica al siguiente(s) checkpoint(s) a encenderse de haber alguno que le siga

			alter_state(next_Checkpoints, true, material_standby, State::Standby);
		}
	}
}

void ATutorial_checkpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATutorial_checkpoint::OnTriggerEnter(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor->IsA(AAla_delta::StaticClass()) && state == State::Current) // Se comprueba que el objeto que haya colisionado con este mesh sea una ala delta y que este sea el checkpoint objetivo
	{
		ala_delta_ptr = Cast <AAla_delta>(OtherActor);

		if (sibling_Checkpoints.Num() > 0) { // De haber algun objetivo paralelo, lo marcamos como superado

			alter_state(sibling_Checkpoints, true, material_activated, State::Activated);
		}

		collider->SetMaterial(0, material_activated); // Marcamos como superado el checkpoint
		collider->SetMaterial(0, material_activated);
		state = State::Activated;

		if (next_Checkpoints.Num() > 0) // Activamos el siguiente grupo de checkpoints como objetivos
		{
			alter_state(next_Checkpoints, true, material_current_waypoint, State::Current);

			if(next_Checkpoints[0]->next_Checkpoints.Num() > 0)
			{
				alter_state(next_Checkpoints[0]->next_Checkpoints, true, material_standby, State::Standby);
			}
		}

		if(previous_Checkpoints.Num() > 0) // De haber alguno, apagamos los checkpoints anteriores para evitar confusion para el jugador
		{
			alter_state(previous_Checkpoints, false, material_activated, State::Activated);
		}
	}
}

void ATutorial_checkpoint::OnTriggerExit(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(AAla_delta::StaticClass()))
	{
	}
}

void ATutorial_checkpoint::alter_state(TArray<ATutorial_checkpoint*> check_array, bool method_visibility, UMaterialInterface* method_material, State method_state)
{
	for (int i = 0; i < check_array.Num(); ++i)
	{
		check_array[i]->collider->SetVisibility(method_visibility);
		check_array[i]->collider->SetMaterial(0, method_material);
		check_array[i]->state = method_state;
	}
}