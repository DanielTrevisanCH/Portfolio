// Fill out your copyright notice in the Description page of Project Settings.


#include "Booster.h"
#include "Components/TimelineComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Utility.h"

// Sets default values
ABooster::ABooster()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	root = CreateDefaultSubobject<USceneComponent>("Root");
	root->SetupAttachment(RootComponent);

	trigger = CreateDefaultSubobject<UStaticMeshComponent>("Trigger");
	trigger->SetupAttachment(root);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderAsset.Succeeded())
	{
		trigger->SetStaticMesh(CylinderAsset.Object);
		trigger->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		trigger->SetRelativeRotation(FRotator(-90.0f, 0, 0));
		trigger->SetWorldScale3D(FVector(1.0f, 1.0f, 0.05f));
	}

	trigger->OnComponentBeginOverlap.AddDynamic(this, &ABooster::OnTriggerBeginOverlap);
}

// Called when the game starts or when spawned
void ABooster::BeginPlay()
{
	Super::BeginPlay();

	if (curve_boost) {
		FOnTimelineFloat timeline_boost;

		timeline_boost.BindUFunction(this, FName("timeline_boost"));

		boost.AddInterpFloat(curve_boost, timeline_boost);

		boost.SetLooping(false);
	}
}

// Called every frame
void ABooster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	boost.TickTimeline(DeltaTime);
}

void ABooster::timeline_boost(float distance)
{
	if(glider)
	glider->set_booster_speed(distance);
}

void ABooster::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) {
		if (curve_boost) {
				
			glider = glider ? glider : Cast<AAla_delta>(OtherActor);

			if (glider) {
				if (glider->GetMesh() == OtherComp) {
				TArray<FRichCurveEditInfo> Curves = curve_boost->GetCurves();
				FKeyHandle key;

				key = Curves[0].CurveToEdit->GetFirstKeyHandle();
				key = Curves[0].CurveToEdit->GetNextKey(key);
				Curves[0].CurveToEdit->SetKeyTime(key, acceleration_time);
				Curves[0].CurveToEdit->SetKeyValue(key, speed_boost);
				key = Curves[0].CurveToEdit->GetNextKey(key);
				Curves[0].CurveToEdit->SetKeyTime(key, (acceleration_time + max_velocity_time));
				Curves[0].CurveToEdit->SetKeyValue(key, speed_boost);
				key = Curves[0].CurveToEdit->GetNextKey(key);
				Curves[0].CurveToEdit->SetKeyTime(key, (acceleration_time + max_velocity_time + decceleration_time));
				Curves[0].CurveToEdit->SetKeyValue(key, 0);

				boost.PlayFromStart();
				}
			}
		}
	}
}