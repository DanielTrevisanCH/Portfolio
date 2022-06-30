#pragma once

#include "CoreMinimal.h"
#include "TP_Variables.generated.h"

UENUM(BlueprintType)
enum class EGrip_Code : uint8
{
	Open = 0,
	CanGrab,
	Grab
};