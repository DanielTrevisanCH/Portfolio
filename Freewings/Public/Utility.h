/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		21/04/2021
  */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"

///@brief Clase estatica con distintas funcionalidades que se usan en diversos ficheros
class FREEWINGS_API Utility
{
	// Es una clase estatica, asique se elimina el contructor y destructor
private:
	Utility() = delete;
	~Utility() = delete;

public:
	static inline void print(FString message = " ", float duration = 0.f, FColor color = FColor::Red)
	{ GEngine->AddOnScreenDebugMessage(-1, duration, color, message); }

	///@return value mapped from one range into another where the value is clamped to the input range (eg: 0.5 normalized from the range 0->1  0->50 would result in 25) 
	///@brief Funcion auxiliar igual a la funcion Map Ranged Clamped de blueprints
	///@attention from1 tiene que ser menor a to1 al igual que from2 debe de ser menor a to2
	static inline float map_ranged_clamped(float value, float from1, float to1, float from2, float to2)
	{ return (FMath::Clamp(value, from1, to1) - to1) / (from1 - to1) * (from2 - to2) + to2; }


	static inline float angle_between(const FVector &a, const FVector &b)
	{
		return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(a, b)));
	}
};
