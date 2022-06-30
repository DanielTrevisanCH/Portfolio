/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		21/04/2021
  */

#pragma once

#pragma region includes

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Wind_component.generated.h"

#pragma endregion

#pragma region Declaraciones adelantadas

class AAla_delta;
class UAudioComponent;

#pragma endregion


///@brief Componente que genera un efecto de viento en el ala delta.
///@details Se genera el viento mediante un perling noise para el roll y el pitch.
/// Al estar a un limite ya sea vertical u horizontal el viento no puede superar los limites de movimientos
/// del ala delta, sin embargo si puede empujar en direccion opuesta al limite, generando asi una especie de 
/// "barrera de viento", que disminuye los angulos de los limites (esto solo ocurre al presionar contra un limite)
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEWINGS_API UWind_component : public UActorComponent
{
	GENERATED_BODY()

#pragma region Componentes

private: UAudioComponent *wind_audio; ///< Componente de audio del sonido de ambiente / viento

#pragma endregion

#pragma region Metodos

public:
	///@brief Ejecuta todas las funcionalidades del componente
	///@details Ajusta el volumen del viento en funcion de la velocidad y genera un efecto de viento
	void run(AAla_delta &, const float delta_time);
	
	

private:
	// 
	//@{
	/** Evita temblores que se producen en los limites horizontales al estar al limite del roll y aumentar el pitch */
	bool is_at_left_limit, is_at_right_limit;
	//@}

	// Evita temblores que se producen al estar en el limte arriba y cambiar de roll (lo que cambia tambien la cantidad posible de pitch)
	bool is_at_up_limit;

	float pitch_offset{ 0 };
	float roll_offset{ 0 };

private:
	UPROPERTY(EditAnywhere, Category = "Ajustes de viento", meta = (AllowPrivateAccess = "true"))
		bool activated{ true };

	UPROPERTY(EditAnywhere, Category = "Ajustes de viento", meta = (ToolTip = "Velocidad del viento en el eje roll cuando el ala delta va a minima velocidad"))
		float roll_max_wind{ 400 }; ///< Velocidad del viento en el eje roll cuando el ala delta va a minima velocidad

	UPROPERTY(EditAnywhere, Category = "Ajustes de viento", meta = (ToolTip = "Velocidad del viento en el eje roll cuando el ala delta va a maxima velocidad"))
		float roll_min_wind{ 80 }; ///< Velocidad del viento en el eje roll cuando el ala delta va a maxima velocidad

	UPROPERTY(EditAnywhere, Category = "Ajustes de viento", meta = (ToolTip = "Velocidad del viento en el eje pitch cuando el ala delta va a minima velocidad"))
		float pitch_max_wind{ 100 }; ///< Velocidad del viento en el eje pitch cuando el ala delta va a minima velocidad

	UPROPERTY(EditAnywhere, Category = "Ajustes de viento", meta = (ToolTip = "Velocidad del viento en el eje pitch cuando el ala delta va a maxima velocidad"))
		float pitch_min_wind{ 20 }; ///< Velocidad del viento en el eje pitch cuando el ala delta va a maxima velocidad

	UPROPERTY(EditAnywhere, Category = "Ajustes de viento", meta = (ToolTip = "Factor pasado al generador de noise. Cuanto mayor menos duran las rafagas y mas brusco sera"))
		float wind_in_factor{ 1 }; ///< Factor pasado al generador de noise. Cuanto mayor menos duran las rafagas y mas brusco sera

	UPROPERTY(EditAnywhere, Category = "Ajustes de viento", meta = (ToolTip = "Angulo maximo de barrera del viento al estar pegado a un limite de roll"))
		float roll_windwall_reset{ 30 }; ///< Angulo maximo de barrera del viento al estar pegado a un limite de roll

	UPROPERTY(EditAnywhere, Category = "Ajustes de viento", meta = (ToolTip = "Angulo maximo de barrera del viento al estar pegado a un limite de pitch"))
		float pitch_windwall_reset{ 30 }; ///< Angulo maximo de barrera del viento al estar pegado a un limite de pitch

private:
	void add_roll_offset(const float value) { roll_offset += value; }
	void add_pitch_offset(const float value) { pitch_offset += value; }

public:
	float get_roll_offset() { return roll_offset; }
	float get_pitch_offset() { return pitch_offset; }
	void reset_roll_offset() { roll_offset = 0; }
	void reset_pitch_offset() { pitch_offset = 0;}

private:
	///@brief Se ajusta el volumen del viento en funcion de la velocidad 
	/// Cuanto mayor sea la velocidad mayor sera el volumen del viento
	void adjust_wind_volume(AAla_delta &); 

	///@brief Se genera el efecto de movimeinto del ala delta por el viento usando perling noise
	/// Cuanto mayor es la velocidad del ala delta mayor el efecto del viento
	void generate_wind_effect(AAla_delta &, const float delta_time);

	///@brief Encuentra el componente de audio al que se le bajara el volumen
	///@attention Para funcionar correctamente necesita que en la escena este el sonido de ambiente, y solo haya uno
	void find_wind_audio_component();

	void roll_wind(AAla_delta &hang_glider, const float delta_time, const float axis_input, const float speed);

	void pitch_wind(AAla_delta &hang_glider, const float delta_time, const float axis_input, const float speed);
#pragma endregion

#pragma region Metodos proporcionados por Unreal

public:
	UWind_component();

protected:
	virtual void BeginPlay() override;

private:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion

};
