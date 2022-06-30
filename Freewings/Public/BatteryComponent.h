// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BatteryComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FREEWINGS_API UBatteryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "BatteryEnergy")
		float current_charge{ 500 };
	UPROPERTY(EditAnywhere, Category = "BatteryEnergy")
		float maximum_charge{ 500 };
	UPROPERTY(EditAnywhere, Category = "BatteryEnergy")
		float charge_rate{ 0.1f };
	UPROPERTY(EditAnywhere, Category = "BatteryEnergy")
		float min_charge_after_depletion_percent{ 40 };

	UPROPERTY()
		bool is_charging{ false };
	UPROPERTY()
		bool is_max_level{ false };

	UPROPERTY()
		bool min_charge_attained{ true };
	UPROPERTY()
		float min_charge_after_depletion{ min_charge_after_depletion_percent * maximum_charge / 100 };

public:
	// Sets default values for this component's properties
	UBatteryComponent();

	bool discharge(float discharge_ammount);

	void recharge_battery();

	bool get_is_charging();

	bool get_can_discharge();

	bool get_is_max_level();

	float get_current_charge();

	float get_maximum_charge();

	UFUNCTION(BlueprintCallable, Category = Battery)
		void set_battery_values(float starting_energy, float maxi_energy, float energy_charge_rate, float min_percent_to_use);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
