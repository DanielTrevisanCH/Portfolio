// Fill out your copyright notice in the Description page of Project Settings.

#include "BatteryComponent.h"

// Sets default values for this component's properties
UBatteryComponent::UBatteryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UBatteryComponent::BeginPlay()
{
	Super::BeginPlay();

	//min_charge_after_depletion = (min_charge_after_depletion_percent * maximum_charge) / 100;

	// ...
}

// Called every frame
void UBatteryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UBatteryComponent::discharge(float ammount)
{
	if ((current_charge - ammount) > 0 && min_charge_attained)
	{
		current_charge -= ammount;
		is_max_level = false;
		is_charging = false;
		return true;
	}
	else
	{
		min_charge_attained = false;
		return false;
	}
}

void UBatteryComponent::recharge_battery()
{
	if (current_charge < maximum_charge)
	{
		is_charging = true;
		if (current_charge < min_charge_after_depletion)
		{
			current_charge += charge_rate;
		}
		else
		{
			if (!min_charge_attained)
				min_charge_attained = !min_charge_attained;

			current_charge += charge_rate;
		}
	}
	else
	{
		is_charging = false;
		is_max_level = true;
		current_charge = maximum_charge;
	}
}

bool UBatteryComponent::get_is_charging()
{
	return is_charging;
}

bool UBatteryComponent::get_can_discharge()
{
	return min_charge_attained;
}

bool UBatteryComponent::get_is_max_level()
{
	return is_max_level;
}

float UBatteryComponent::get_current_charge()
{
	return current_charge;
}

float UBatteryComponent::get_maximum_charge()
{
	return maximum_charge;
}

void UBatteryComponent::set_battery_values(float starting_energy, float maxi_energy, float energy_charge_rate, float min_percent_to_use)
{
	maximum_charge = maxi_energy;
}