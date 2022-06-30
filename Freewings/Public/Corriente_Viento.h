/** Copyright: @FreeWings
 *	Author: Daniel Trevisan Cohen-Henriquez
 *	Date: 20/04/2021
 */

#pragma once

#include "Components/BoxComponent.h"
#include "Math/Vector.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Corriente_Viento.generated.h"


UCLASS()
class FREEWINGS_API ACorriente_Viento : public AActor
{
	GENERATED_BODY()
	
public:	

	ACorriente_Viento();

	class UBoxComponent *collider; 

	class AAla_delta *ala_delta_ptr;

protected:

	virtual void BeginPlay() override;

private:

	bool inside_trigger{ false }; ///< Booleano de control usado para saber si el ala se encuentra dentro del trigger o no

	void calculate_force(); ///< Metodo usado para definir como se deberian llamar los metodos de reduccion de fuerza 

	float reduce_force(float force, float angle, float ala_angle); ///< Metodo usado para reducir la fuerza aplicada sobre el ala en cada frame

	float correct_angles(float angle); ///< Metodo usado para asegurarse que los angulos suministrados por diseno son validos

	float maxAngle{ 0.f }; ///<Variable usada para delimitar el maximo posible angulo

	float ala_pitch{ 0.f }; ///< Variable usada para almacenar el angulo de inclinacion del ala

	float ala_roll{ 0.f }; ///< Variable usada para almacenar el angulo de alabedo del ala

	float final_angle{ 0.f }; ///< Variable usada para almacenar el valor del angulo mayor en el caso que "split_method" sea false

	float final_force{ 0.f }; ///< Variable usada para almacenar el valor de la fuerza total en el caso que "split_method" sea false

	UPROPERTY(EditAnywhere, Category = "Control de Viento", meta = (ToolTip = "False usara una sola medida limite basada en el mayor de los angulos maximo suministrado para los calculos de fuerza con reduccion basada en los angulos del ala. True usara dos medidas limite distintas para los calculos de fuerza con reduccion basada en los angulos del ala."))
		bool split_method{ false }; ///< Booleano usado para saber cual metodo de reduccion de fuerzas se debe usar

	UPROPERTY(EditAnywhere, Category = "Control de Viento", meta = (ToolTip = "Medida limte. Maximo angulo positivo o negativo del ala delta en inclinacion en el cual no se le aplicara mas la fuerza de viento."))
		float pitch_angle{ 60.f }; ///< Angulo maximo permitido para que le sea aplicado fuerza al ala en el eje de inclinacion

	UPROPERTY(EditAnywhere, Category = "Control de Viento", meta = (ToolTip = "Fuerza ejercida sobre el ala en el eje de inclinacion dentro del trigger. Usada individualmente antes de reducciones si split_method = false, se sumara con roll_force antes de reducciones si split_method = true."))
		float pitch_force{ 500.f }; ///< Fuerza positiva vertical que se le aplicara al ala basada en su angulo de inclinacion

	UPROPERTY(EditAnywhere, Category = "Control de Viento", meta = (ToolTip = "Medida limite. Maximo angulo positivo o negativo del ala delta en alabeo en el cual no se le aplicara mas la fuerza de viento."))
		float roll_angle{ 60.f}; ///< Angulo maximo permitido para que le sea aplicado fuerza al ala en el eje de alabedo

	UPROPERTY(EditAnywhere, Category = "Control de Viento", meta = (ToolTip = "Fuerza ejercida sobre el ala en el eje de alabedo dentro del trigger. Usada individualmente antes de reducciones si split_method = false, se sumara con pitch_force antes de reducciones si split_method = true."))
		float roll_force{ 500.f }; ///< Fuerza positiva vertical que se le aplicara al ala basada en su angulo de alabedo


public:	

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTriggerEnter(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void OnTriggerExit(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);

};
