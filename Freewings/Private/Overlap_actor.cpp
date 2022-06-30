/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		marzo 2021
  */

#include "Overlap_actor.h"
#include "Components/BoxComponent.h"
#include "Ala_delta.h"
#include "DrawDebugHelpers.h"
#include "Collision_detector.h"

void print(FString message);

AOverlap_actor::AOverlap_actor()
{
	//PrimaryActorTick.bCanEverTick = true;

	static int id{ 0 };
	box_trigger = CreateDefaultSubobject<UBoxComponent>(*FString::FromInt(++id));
	box_trigger->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	box_trigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	box_trigger->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
}

void AOverlap_actor::set_detector(ACollision_detector* collision_detector)
{
	detector = collision_detector;
}

void AOverlap_actor::BeginPlay()
{
	Super::BeginPlay();
	suscribe_to_overlap_events();
}

void AOverlap_actor::suscribe_to_overlap_events()
{
	box_trigger->OnComponentBeginOverlap.AddDynamic(this, &AOverlap_actor::BeginOverlap);
	box_trigger->OnComponentEndOverlap.AddDynamic(this, &AOverlap_actor::OnOverlapEnd);
}

void AOverlap_actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DrawDebugBox(GetWorld(), box_trigger->GetComponentLocation(), box_trigger->GetScaledBoxExtent(), box_trigger->GetComponentRotation().Quaternion(), FColor::Red, false, 0.05f, 0, 4);
}

void AOverlap_actor::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	detector->add_weight(name, 1);
}

void AOverlap_actor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	detector->add_weight(name, -1);
}

void AOverlap_actor::set_size(const FVector& size)
{
	box_trigger->SetBoxExtent(size);
}

void AOverlap_actor::set_name(const FString& n)
{
	name = n;
}