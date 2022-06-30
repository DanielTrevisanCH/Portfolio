#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ala_delta.h"
#include "Components/TimelineComponent.h"
#include "Booster_tube.generated.h"

class UCurveFloat;
class USceneComponent;
class USplineComponent;
class UStaticMeshComponent;

UCLASS()
class FREEWINGS_API ABooster_tube : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABooster_tube();

	UPROPERTY(EditAnywhere, Category = "Component")
		UStaticMeshComponent* trigger;

	UPROPERTY(EditAnywhere, Category = "Component")
		USplineComponent* spline;

	UPROPERTY()
		USceneComponent* root;

//	UPROPERTY()
//		TArray<AAla_delta*> gliders;

	UPROPERTY()
		AAla_delta* glider;

	UPROPERTY()
		float initial_pitch;

	UPROPERTY()
		float initial_roll;

protected:
	// Called when the game starts or when spawned
		virtual void BeginPlay() override;

		FTimeline boost_tube;

	UPROPERTY(EditAnywhere, Category = "Timeline")
		UCurveFloat* curve_boost_tube;
public:	
	// Called every frame
		virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void timeline_boost_tube(float distance);

	UFUNCTION()
		void timeline_boost_tube_finish();

	UFUNCTION()
		void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere, Category = "Tube configuration")
		float speed_along_tube = { 2000.0f };

	UPROPERTY(EditAnywhere, Category = "Tube configuration")
		float force_tangent = { 5000.0f };

	UPROPERTY()
		float last_speed = { 0.f };

	UPROPERTY()
		float initial_speed = { 0.f };

	UPROPERTY()
		float distance_spline = { 0.f };

	UPROPERTY()
		float acceleration = { 0.f };

	UFUNCTION()
		void calculate_speed(float deltatime);

	UFUNCTION()
		void calculate_distance(float deltatime);
};