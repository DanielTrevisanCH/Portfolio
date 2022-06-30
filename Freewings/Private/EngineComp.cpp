// Fill out your copyright notice in the Description page of Project Settings.

#include "EngineComp.h"
#include "Components/TimelineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../FreeWings/Public/BatteryComponent.h"

// Sets default values for this component's properties
UEngineComp::UEngineComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	batteryComponent = CreateDefaultSubobject<UBatteryComponent>("BatteryComp");
}

// Called when the game starts
void UEngineComp::BeginPlay()
{
	Super::BeginPlay();

	if (curve_on && curve_progress && curve_off && curve_battery) {

		set_curve_length(curve_on, curve_on_time, 1);
		set_curve_length(curve_off, curve_off_time, 0);

		FOnTimelineFloat timeline_engine_on;
		FOnTimelineEvent timeline_event_on_finish;

		FOnTimelineFloat timeline_engine_progress;

		FOnTimelineFloat timeline_engine_off;
		FOnTimelineEvent timeline_event_charge;

		FOnTimelineFloat timeline_battery_charge;

		timeline_engine_on.BindUFunction(this, FName("timeline_engine_on"));
		timeline_event_on_finish.BindUFunction(this, FName("timeline_event_on_finish"));

		timeline_engine_progress.BindUFunction(this, FName("timeline_engine_progress"));

		timeline_engine_off.BindUFunction(this, FName("timeline_engine_off"));
		timeline_event_charge.BindUFunction(this, FName("timeline_event_charge"));

		timeline_battery_charge.BindUFunction(this, FName("timeline_battery_charge"));

		engine_on.AddInterpFloat(curve_on, timeline_engine_on);
		engine_on.SetTimelineFinishedFunc(timeline_event_on_finish);

		engine_progress.AddInterpFloat(curve_progress, timeline_engine_progress);

		engine_off.AddInterpFloat(curve_off, timeline_engine_off);
		engine_off.SetTimelineFinishedFunc(timeline_event_charge);

		battery_charge.AddInterpFloat(curve_battery, timeline_battery_charge);

		engine_on.SetLooping(false);
		engine_progress.SetLooping(true);
		engine_off.SetLooping(false);
		battery_charge.SetLooping(true);
	}
}

// Called every frame
void UEngineComp::TickComponent(float delta_time, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(delta_time, TickType, ThisTickFunction);

	engine_on.TickTimeline(delta_time);
	engine_progress.TickTimeline(delta_time);
	engine_off.TickTimeline(delta_time);
	battery_charge.TickTimeline(delta_time);
}

void UEngineComp::apply_acceleration(float limit, bool state)
{
	speed_limit = limit;
	if (speed_pointer != nullptr) {
		if (state) {
			button_press = true;
			if (!turn_on())
				turn_off();
		}
		else {
			button_press = false;
			turn_off();
		}
	}
}

void UEngineComp::accelerate(float accel)
{
	if (batteryComponent->discharge(discharge_battery_ammount)) {
		is_on = true;
		current_acceleration = accel;
		if (*speed_pointer + current_acceleration >= speed_limit) { 
			if (*speed_pointer > speed_limit) {
				return;
			}
			*speed_pointer = speed_limit;
			return; }
		*speed_pointer += current_acceleration; 
	}
	else {
		turn_off();
	}
}

void UEngineComp::deaccelerate(float accel)
{
	batteryComponent->recharge_battery();
	current_acceleration = accel;
}

UBatteryComponent* UEngineComp::get_battery_component()
{
	return batteryComponent;
}

bool UEngineComp::get_is_on()
{
	return is_on;
}

bool UEngineComp::is_max_level()
{
	return batteryComponent->get_is_max_level();
}

bool UEngineComp::turn_on()
{
	if (*speed_pointer < speed_limit) {
		if (batteryComponent->get_can_discharge()) {
			if (engine_off.IsPlaying())
				engine_off.Stop();
			if (battery_charge.IsPlaying())
				battery_charge.Stop();
			set_timeline_position(current_acceleration, engine_on, curve_on, curve_on_time);
			engine_on.Play();
			is_on = true;
		}
		else
			is_on = false;
	}
	else
		is_on = false;

	return is_on;
}

void UEngineComp::turn_off()
{
	if (engine_on.IsPlaying())
		engine_on.Stop();
	if (engine_progress.IsPlaying())
		engine_progress.Stop();
	set_timeline_position(current_acceleration, engine_off, curve_off, curve_off_time);
	engine_off.Play();
	is_on = false;
}

void UEngineComp::set_speed(float& speed)
{
	speed_pointer = &speed;
}

float* UEngineComp::get_speed_pointer()
{
	return speed_pointer;
}

float UEngineComp::get_current_acceleration()
{
	return current_acceleration;
}

float UEngineComp::get_maximum_acceleration()
{
	return max_acceleration_possible;
}

void UEngineComp::set_engine_values(float discharge_battery_amount, float max_acceleration_to_reach, float rate_of_acceleration_gain, float rate_of_deacceleration)
{
	max_acceleration_possible = max_acceleration_to_reach;
}

void UEngineComp::set_timeline_position(float value, FTimeline& time, UCurveFloat* curve, float curve_time)
{
	float i;
	float range = 1.0f;
	do {
		range /= 10;
		for (i = 0; i < curve_time && !(value - 0.001f < curve->GetFloatValue(i) && value + 0.001f > curve->GetFloatValue(i)); i += range);
	} while (i >= curve_time && range > 0.001f);

	if (i < curve_time) {
		time.SetPlaybackPosition(i, false);
	}
	else {
		time.SetPlaybackPosition(0, false);
	}
}

void UEngineComp::timeline_engine_on(float acceleration)
{
	if (*speed_pointer < speed_limit)
		accelerate(acceleration);
	else
		turn_off();
}

void UEngineComp::timeline_engine_progress(float acceleration)
{
	if (*speed_pointer < speed_limit)
		accelerate(current_acceleration);
	else
		turn_off();
}

void UEngineComp::timeline_battery_charge(float acceleration)
{
	batteryComponent->recharge_battery(); 
	if (button_press && batteryComponent->get_can_discharge()) {
		turn_on();
	}
	if (is_max_level()) {
		if (engine_on.IsPlaying())
			engine_on.Stop();
		if (engine_progress.IsPlaying())
			engine_progress.Stop();
		if (engine_off.IsPlaying())
			engine_off.Stop();
		if (battery_charge.IsPlaying())
			battery_charge.Stop();
	}
}

void UEngineComp::timeline_engine_off(float acceleration)
{
	deaccelerate(acceleration); 
	if (button_press && batteryComponent->get_can_discharge()) {
		turn_on();
	}
}

void UEngineComp::timeline_event_on_finish()
{
	if (!engine_off.IsPlaying()) {
		engine_progress.PlayFromStart();
	}
}

void UEngineComp::timeline_event_charge()
{
	if (!engine_on.IsPlaying()) {
		battery_charge.PlayFromStart();
	}
}

void UEngineComp::set_curve_length(UCurveFloat* curve, float lenght, int keytochange)
{
	TArray<FRichCurveEditInfo> Curves = curve->GetCurves();
	FKeyHandle key;
	switch (keytochange) {
		case 0:
			key = Curves[0].CurveToEdit->GetFirstKeyHandle();
			Curves[0].CurveToEdit->SetKeyValue(key, max_acceleration_possible);
			key = Curves[0].CurveToEdit->GetLastKeyHandle();
			Curves[0].CurveToEdit->SetKeyTime(key, lenght);
			break;
		case 1:
			key = Curves[0].CurveToEdit->GetLastKeyHandle();
			Curves[0].CurveToEdit->SetKeyTime(key, lenght);
			Curves[0].CurveToEdit->SetKeyValue(key, max_acceleration_possible);
			break;
	}
}