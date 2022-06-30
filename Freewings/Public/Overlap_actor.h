/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		marzo 2021
  */
#pragma once

#pragma region Bibliotecas

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Overlap_actor.generated.h"

#pragma endregion


#pragma region Declaraciones adelantadas

class UBoxComponent;
class AAla_delta;
class ACollision_detector;

#pragma endregion


#pragma region Clase Overlap_actor

/// @brief Clase que recibe colisiones para hacer que el ala delta evite obstaculos. 
/// Varios AOverlap_actors estan cotrolados por un ACollisionDetector, que se encarga de recopilar los datos de las colisiones y actuar sobre el ala delta
UCLASS()
class FREEWINGS_API AOverlap_actor : public AActor
{
	GENERATED_BODY()

	#pragma region Variables y componentes

private:
	FString name;				   ///< Nombre del actor. Se usa para diferenciarlo si es por ejemplo "left", "right", "up", etc.
	ACollision_detector* detector; ///< Manager de los detectores de colision que controlara a este objeto

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* box_trigger; ///< Caja que actua como trigger

	#pragma endregion

	#pragma region Getters y setters

public:
	void set_name(const FString& n);    ///< Asigna el nombre del actor
	void set_size(const FVector& size); ///< Cambia el tamano de la caja del trigger
	void set_detector(ACollision_detector* collision_detector); ///< Asigna el manager de colision que controlara este objeto

	#pragma endregion

	#pragma region Metodos propios

private: void suscribe_to_overlap_events();

	#pragma endregion

	#pragma region Metodos proporcionador por Unreal

public:
	AOverlap_actor();

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	#pragma endregion

};

#pragma endregion
