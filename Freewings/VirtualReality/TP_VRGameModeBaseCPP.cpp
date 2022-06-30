#include "TP_VRGameModeBaseCPP.h"
#include "TP_VirtualRealityPawn_Motion.h"
#include "TP_VirtualRealityPawn_GamePad.h"

ATP_VRGameModeBaseCPP::ATP_VRGameModeBaseCPP()
{
	// DefaultPawnClass
	DefaultPawnClass = ATP_VirtualRealityPawn_Motion::StaticClass();
}