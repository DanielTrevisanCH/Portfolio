// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ala_delta.h"
#include "Components/TimelineComponent.h"
#include "Booster.generated.h"

class UCurveFloat;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class FREEWINGS_API ABooster : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABooster();

	UPROPERTY(EditAnywhere, Category = "Component")
		UStaticMeshComponent* trigger;

	UPROPERTY()
		USceneComponent* root;

	UPROPERTY()
		AAla_delta* glider;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	FTimeline boost;

	UPROPERTY(EditAnywhere, Category = "Timeline")
		UCurveFloat* curve_boost;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void timeline_boost(float distance);

	UFUNCTION()
		void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:

	UPROPERTY(EditAnywhere, Category = "Tube configuration")
		float speed_boost = { 2000.0f };

	UPROPERTY(EditAnywhere, Category = "Tube configuration")
		float acceleration_time = { 2.f };

	UPROPERTY(EditAnywhere, Category = "Tube configuration")
		float decceleration_time = { 2.f };

	UPROPERTY(EditAnywhere, Category = "Tube configuration")
		float max_velocity_time = { 5.f };
};