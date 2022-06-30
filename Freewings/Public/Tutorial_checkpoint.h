/** Copyright: @FreeWings
 *	Author: Daniel Trevisan Cohen-Henriquez
 *	Date: 07/05/2021
 */

#pragma once

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tutorial_checkpoint.generated.h"

UCLASS()
class FREEWINGS_API ATutorial_checkpoint : public AActor
{
	GENERATED_BODY()
	
public:	

	ATutorial_checkpoint();

	UPROPERTY(VisibleAnywhere, Category = "Meshes")
		UStaticMeshComponent *mesh;

	UPROPERTY(VisibleAnywhere, Category = "Meshes")
		UStaticMeshComponent *collider;

	UPROPERTY(EditAnywhere, Category = "Manage Checkpoints", meta = (ToolTip = "")) ///< Array utilizado para saber cual es el siguiente checkpoint(s) de la pista
		TArray<ATutorial_checkpoint*> next_Checkpoints;

	UPROPERTY(EditAnywhere, Category = "Manage Checkpoints", meta = (ToolTip = "")) ///< Array utilizado para saber cual checkpoint(s) comparten objetivo
		TArray<ATutorial_checkpoint*> sibling_Checkpoints;

	UPROPERTY(EditAnywhere, Category = "Manage Checkpoints", meta = (ToolTip = "")) ///< Array utilizado para saber cual checkpoint(s) lo preceden en la pista 
		TArray<ATutorial_checkpoint*> previous_Checkpoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material List") 
		UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, Category = "Material List") ///< Se utiliza para cambiar el color del checkpoint dandole al jugador indicacion de a cual debe ir
		UMaterialInterface* material_current_waypoint; 

	UPROPERTY(EditAnywhere, Category = "Material List") ///< Se utiliza para cambiar el color del checkpoint dandole al jugador indicacion de por cual ya ha pasado
		UMaterialInterface* material_activated;

	UPROPERTY(EditAnywhere, Category = "Material List") ///< Color base de un checkpoint. Mantendra este color hasta convertirse en el siguiente objetivo
		UMaterialInterface* material_standby;


private:

	class AAla_delta *ala_delta_ptr; ///< Referencia al ala delta

	enum class State { Standby, Activated, Current, Initialize } state; ///< Los diferentes estados de los checkpoints, basandose en que punto de la pista esten

	UFUNCTION()
		void OnTriggerEnter(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
		void OnTriggerExit(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);

	void alter_state(TArray<ATutorial_checkpoint*> check_array, bool method_visibility, UMaterialInterface* method_material, State method_state);


protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
