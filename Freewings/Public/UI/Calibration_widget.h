/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		marzo 2021
  */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Calibration_widget.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;
class UCanvasPanel;
/**
 * 
 */

UCLASS()
class FREEWINGS_API UCalibration_widget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//virtual void NativeConstruct() override; // Equivalente a BeginPlay

	UCalibration_widget(const FObjectInitializer &object_initializer);
	
	void set_progress_bar_percent(float value); ///< Ajusta el porcentaje del fill del progress_bar

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		UProgressBar *progress_bar; ///< Barra de progreso que se ira rellenando al calibrar las gafas VR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		UImage *black_image; ///< Imagen temporal que pone la vista de negro
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		UTextBlock *calibration_text; ///< Texto que muestra que el jugador mire en frente para calibrar las gafas VR
};
