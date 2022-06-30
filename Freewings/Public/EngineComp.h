// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "Components/ActorComponent.h"
#include "EngineComp.generated.h"

class UCurveFloat;
class UBatteryComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEWINGS_API UEngineComp : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = "BatteryComponent", meta = (AllowPrivateAccess = "true"))
		UBatteryComponent* batteryComponent;

	UPROPERTY(EditAnywhere, Category = "EngineComponent", meta = (AllowPrivateAccess = "true"))
		float discharge_battery_ammount{ 0.5f };

	UPROPERTY(EditAnywhere, Category = "EngineComponent", meta = (AllowPrivateAccess = "true"))
		float max_acceleration_possible{ 0.1f };

	UPROPERTY(EditAnywhere, Category = "EngineComponent", meta = (AllowPrivateAccess = "true"))
		float curve_on_time{ 5.0f };

	UPROPERTY(EditAnywhere, Category = "EngineComponent", meta = (AllowPrivateAccess = "true"))
		float curve_off_time{ 5.0f };

	/*UPROPERTY(EditAnywhere, Category = "EngineComponent", meta = (AllowPrivateAccess = "true"))
		float acceleration_gain_rate{ 0.1f };

	UPROPERTY(EditAnywhere, Category = "EngineComponent", meta = (AllowPrivateAccess = "true"))
		float deacceleration_rate{ 0.1f };*/

	UPROPERTY(EditAnywhere, Category = "EngineComponent")
		float current_acceleration;

	UPROPERTY()
		bool is_on{ false };

	UPROPERTY()
		bool button_press{ false };

	float* speed_pointer;

	float speed_limit;

	void set_timeline_position(float value, FTimeline& time, UCurveFloat* curve, float curve_time);

	void accelerate(float accel);

	void deaccelerate(float accel);

	void set_curve_length(UCurveFloat* curve, float lenght, int keytochange);

public:	
	// Sets default values for this component's properties
	UEngineComp();

	//float max_acceleration_possible{ 0.1f };

	void apply_acceleration(float limit, bool state);

	bool is_max_level();

	bool get_is_on();

	bool turn_on();

	void turn_off();

	void set_speed(float& speed);

	float* get_speed_pointer();

	float get_current_acceleration();

	float get_maximum_acceleration();

	UFUNCTION(BlueprintCallable, Category = Ala_Delta_Engine)
		UBatteryComponent* get_battery_component();

	UFUNCTION(BlueprintCallable, Category = Battery)
		void set_engine_values(float use_battery_energy_amount, float max_acceleration_to_reach, float rate_of_acceleration_gain, float rate_of_deacceleration);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FTimeline engine_on;
	FTimeline engine_progress;
	FTimeline engine_off;
	FTimeline battery_charge;

	UPROPERTY(EditAnywhere, Category = "Timeline")
		UCurveFloat* curve_on;
	UPROPERTY(EditAnywhere, Category = "Timeline")
		UCurveFloat* curve_progress;
	UPROPERTY(EditAnywhere, Category = "Timeline")
		UCurveFloat* curve_battery;
	UPROPERTY(EditAnywhere, Category = "Timeline")
		UCurveFloat* curve_off;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void timeline_engine_on(float acceleration);
	UFUNCTION()
	void timeline_engine_progress(float acceleration);
	UFUNCTION()
	void timeline_battery_charge(float acceleration);
	UFUNCTION()
	void timeline_engine_off(float acceleration);

	UFUNCTION()
	void timeline_event_on_finish();
	UFUNCTION()
	void timeline_event_charge();
};
