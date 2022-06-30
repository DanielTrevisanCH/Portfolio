/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		marzo 2021
  */

#include "UI/Game_HUD.h"
#include "UI/Calibration_widget.h"

void print(FString message = " ", float duration = 0.f, FColor color = FColor::Red);

AGame_HUD::AGame_HUD() 
{
	
}

void AGame_HUD::DrawHUD()
{
	Super::DrawHUD();
}

void AGame_HUD::BeginPlay()
{
	Super::BeginPlay();

	if (false)
	{
		print("CREATING");
		calibration_widget = CreateWidget<UCalibration_widget>(GetWorld(), calibration_widget_class, FName("Calibration Widget"));
		if (calibration_widget)
		{
			calibration_widget->SetVisibility(calibration_visibility);
			calibration_widget->AddToViewport();
		}
	}
}

void AGame_HUD::Tick(float delta_time)
{
	Super::Tick(delta_time);
}

void AGame_HUD::set_progress_bar_percent(float value)
{
	if (calibration_widget)
		calibration_widget->set_progress_bar_percent(value);	
}

void AGame_HUD::set_calibration_widget_visibility(ESlateVisibility new_visibility)
{
	calibration_visibility = new_visibility;
	if (calibration_widget) calibration_widget->SetVisibility(calibration_visibility);
}