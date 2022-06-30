/** Copyright: @FreeWings
 *	Author: Daniel Trevisan Cohen-Henriquez
 *	Date: 21/04/2021
 */

#pragma once

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Landing_Target.generated.h"

class AScore_Manager;

UCLASS()
class FREEWINGS_API ALanding_Target : public AActor
{
	GENERATED_BODY()
	
public:	

	ALanding_Target();

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent *Mesh;

	UPROPERTY(EditAnywhere, Category = "Score System", meta = (ToolTip = "Gerente de la puntuacion. Asignar esta variable al actor de tipo Score_Manager."))
		AScore_Manager *score_manager_ptr; ///< Referencia al gerente de puntuacion

	UPROPERTY(EditAnywhere, Category = "Score System", meta = (ToolTip = "Puntuacion a dar cuando el ala impacte contra esta parte de la diana."))
		int score_to_grant{ 0 }; ///< Variable usada para darle puntaje al ala delta

	

private:

	bool score_granted{ false }; ///< Variable de control para asegurar que no se le de la puntucion multiples veces

	class AAla_delta *ala_delta_ptr; ///< Puntero al ala delta

	void grant_score(); ///< Metodo usado para iniciar el proceso de finalizar la partida

	UFUNCTION()
		void OnTriggerEnter(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
		void OnTriggerExit(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);
	

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
