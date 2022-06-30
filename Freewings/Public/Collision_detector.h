/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		marzo 2021
  */

#pragma once

#pragma region Bibliotecas

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Collision_detector.generated.h"

#pragma endregion

#pragma region Declaraciones adelantadas

class AOverlap_actor;
class AAla_delta;

#pragma endregion

#pragma region Clase ACollision_detector

UCLASS()
class FREEWINGS_API ACollision_detector : public AActor
{
	GENERATED_BODY()

	#pragma region Variables

private:
	AAla_delta* ala_delta_ptr; ///< Puntero al ala delta

	//@{
	/** Puntero a actor que detecta colisiones para evitar que el ala delta choque contra las paredes */
	AOverlap_actor* left_trigger, *right_trigger, *up_trigger, *down_trigger;
	//@}

	//@{
	/** Peso que indica hacia donde debe girar el ala delta. Cuanto mas peso en una direccion mas predisposicion habra para evitar la zona */
	float left_weight{ 0 }, right_weight{ 0 }, up_weight{ 0 }, down_weight{ 0 };
	//@}

	#pragma endregion

	#pragma region Metodos

public:
	void add_weight(const FString& trigger_name, float weight); ///< Anade el peso al lado indicado para que se adapte la direccion del giro
	void take_action();					     ///< Elige el giro necesario en funcion de los pesos o triggers actuales
	void create_triggers();				     ///< Crea todos los triggers necesarios para detectar las colisiones
	void set_ala_delta_ptr(AAla_delta* ala); ///< Establece el puntero al ala delta

public:
	ACollision_detector();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	#pragma endregion

};

#pragma endregion