/** Copyright: @FreeWings
 *	Author: Daniel Trevisan Cohen-Henriquez
 *	Date: 21/04/2021
 */


#include "Landing_Target.h"
#include "DrawDebugHelpers.h"
#include "Ala_delta.h"
#include "Engine.h"
#include "Score_Manager.h"




ALanding_Target::ALanding_Target()
{

	PrimaryActorTick.bCanEverTick = true;


	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Mesh->BodyInstance.bNotifyRigidBodyCollision = true;

	Mesh->BodyInstance.SetCollisionProfileName("OverlapAll");
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &ALanding_Target::OnTriggerEnter);
	Mesh->OnComponentEndOverlap.AddDynamic(this, &ALanding_Target::OnTriggerExit);
}


void ALanding_Target::BeginPlay()
{
	Super::BeginPlay();
	
	

}


void ALanding_Target::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ALanding_Target::OnTriggerEnter(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor->IsA(AAla_delta::StaticClass()) && !score_granted) // Me aseguro que el objeto que haya colisionado con este mesh sea una ala delta y que este metodo no haya sido llamado previamente
	{
		
		ala_delta_ptr = Cast <AAla_delta>(OtherActor);

		//ala_delta_ptr->testingString = "CRITICAL HIT!"; 

		grant_score();
	} 
}


void ALanding_Target::OnTriggerExit(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(AAla_delta::StaticClass())) 
	{

	}
}

void ALanding_Target::grant_score()
{
	score_manager_ptr->add_score(score_to_grant);

	score_granted = !score_granted;

	ala_delta_ptr->set_state(AAla_delta::State::None);
}
