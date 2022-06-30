// Fill out your copyright notice in the Description page of Project Settings.


#include "Booster_tube.h"
#include "Components/TimelineComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Utility.h"

// Sets default values
ABooster_tube::ABooster_tube()
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

	trigger->OnComponentBeginOverlap.AddDynamic(this, &ABooster_tube::OnTriggerBeginOverlap);

	spline = CreateDefaultSubobject<USplineComponent>("Spline");
	spline->SetupAttachment(root);
	spline->Duration = 100;
	spline->bDrawDebug = true;
}

// Called when the game starts or when spawned
void ABooster_tube::BeginPlay()
{
	Super::BeginPlay();
	
	if (curve_boost_tube) {
		FOnTimelineFloat timeline_boost_tube;
		FOnTimelineEvent timeline_boost_tube_finish;

		timeline_boost_tube.BindUFunction(this, FName("timeline_boost_tube"));
		timeline_boost_tube_finish.BindUFunction(this, FName("timeline_boost_tube_finish"));

		boost_tube.AddInterpFloat(curve_boost_tube, timeline_boost_tube);
		boost_tube.SetTimelineFinishedFunc(timeline_boost_tube_finish);

		boost_tube.SetLooping(false);
	}
}

// Called every frame
void ABooster_tube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	boost_tube.TickTimeline(DeltaTime);
}

void ABooster_tube::timeline_boost_tube(float distance){
	calculate_speed(boost_tube.GetPlaybackPosition());
	calculate_distance(boost_tube.GetPlaybackPosition());

	//Controlo la rotacion dentro del tubo
	FVector location = spline->GetLocationAtDistanceAlongSpline(distance_spline, ESplineCoordinateSpace::Type::World);
	FRotator rotator = spline->GetRotationAtDistanceAlongSpline(distance_spline, ESplineCoordinateSpace::Type::World);
	rotator.Yaw += 90;
	if (rotator.Pitch != 0) {
		rotator.Pitch = Utility::map_ranged_clamped(distance_spline, 0, spline->GetDistanceAlongSplineAtSplinePoint(1), initial_pitch, 0);
	}
	rotator.Roll = Utility::map_ranged_clamped(distance_spline,  0, spline->GetDistanceAlongSplineAtSplinePoint(1),  initial_roll, - rotator.Roll);
	
	glider->SetActorLocation(location);
	glider->GetMesh()->SetRelativeRotation(rotator);
}

void ABooster_tube::timeline_boost_tube_finish(){
	glider->set_state(AAla_delta::State::Run);
	glider->set_move_speed(speed_along_tube);
	glider = nullptr;
}

void ABooster_tube::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	/*
	t = (2d/(vi+vf))
	a = (vf-vi/t)
	*/
	if (OtherActor) {
		if (curve_boost_tube) {
			glider = glider ? glider : Cast<AAla_delta>(OtherActor);

			if (glider) {
				if (glider->GetMesh() == OtherComp && glider->get_state() != AAla_delta::State::None) {
					//controla la rotacion inicial de entrada
					initial_pitch = glider->GetMesh()->GetRelativeRotation().Pitch;
					initial_roll = glider->GetMesh()->GetRelativeRotation().Roll;
					//paro el ala
					glider->set_state(AAla_delta::State::None);
					//asigno las tangentes de los puntos del spline teniendo en cuenta la entrada del ala al tubo
					spline->SetLocationAtSplinePoint(0, glider->GetActorLocation(), ESplineCoordinateSpace::Type::World, true);
					spline->SetTangentsAtSplinePoint(0, spline->GetArriveTangentAtSplinePoint(0, ESplineCoordinateSpace::Type::World), glider->GetMesh()->GetRightVector() * - force_tangent, ESplineCoordinateSpace::Type::World, true);
					//spline->SetTangentsAtSplinePoint(1, spline->GetArriveTangentAtSplinePoint(1, ESplineCoordinateSpace::Type::World), {0,0,0}, ESplineCoordinateSpace::Type::World, true);
					//spline->SetTangentsAtSplinePoint(2, spline->GetArriveTangentAtSplinePoint(2, ESplineCoordinateSpace::Type::World), {0,0,0}, ESplineCoordinateSpace::Type::World, true);

					//Genero el tiempo necesario para recorrer el tubo en base a la velocidad final asignada
					TArray<FRichCurveEditInfo> Curves = curve_boost_tube->GetCurves();
					FKeyHandle key;
					float time_key = (2*spline->GetSplineLength()) / (glider->get_result_speed() + speed_along_tube);

					last_speed = initial_speed = glider->get_result_speed();
					acceleration = (speed_along_tube - initial_speed) / time_key;

					key = Curves[0].CurveToEdit->GetLastKeyHandle();
					Curves[0].CurveToEdit->SetKeyTime(key, time_key);
					Curves[0].CurveToEdit->SetKeyValue(key, spline->GetSplineLength());

					boost_tube.PlayFromStart();
				}
			}
		}
	}
}

void ABooster_tube::calculate_speed(float deltatime)
{
	last_speed = initial_speed + acceleration * deltatime;
}

void ABooster_tube::calculate_distance(float deltatime)
{
	distance_spline =  ((initial_speed + last_speed)/2) * deltatime < spline->GetSplineLength() ? ((initial_speed + last_speed) / 2) * deltatime : spline->GetSplineLength();
}
