/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		marzo 2021
  */

#include "UI/Calibration_widget.h"

void print(FString message);



UCalibration_widget::UCalibration_widget(const FObjectInitializer & object_initializer) : UUserWidget(object_initializer)
{}



void UCalibration_widget::set_progress_bar_percent(float value)
{
	if(progress_bar)
		progress_bar->SetPercent(value);	
}
