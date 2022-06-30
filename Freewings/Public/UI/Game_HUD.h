// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		marzo 2021
  */

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Components/WidgetComponent.h"
#include "Game_HUD.generated.h"

class UCalibration_widget;
/**
 * 
 */
UCLASS()
class FREEWINGS_API AGame_HUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AGame_HUD();

	virtual void DrawHUD() override; // The main Draw loop for the hud

	virtual void BeginPlay() override;

	virtual void Tick(float delta_time) override;

	UFUNCTION()
		void set_progress_bar_percent(float value); ///< Ajusta el porcentaje del fill del progress_bar

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<UUserWidget> calibration_widget_class; ///< Clase o tipo del Widget de calibracion (que se arrastrara desde el blueprint)

private:
	UCalibration_widget *calibration_widget; ///< Puntero al widget de calibracion
public:
	UCalibration_widget *get_calibration_widget() { return calibration_widget; }

	ESlateVisibility calibration_visibility = ESlateVisibility::Visible;

public:
	void set_calibration_widget_visibility(ESlateVisibility);

};
