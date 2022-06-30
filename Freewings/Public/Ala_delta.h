/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		17/10/2020
  */

#pragma once

#pragma region Includes

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Ala_delta.generated.h"

#pragma endregion

#pragma region Declaraciones adelantadas

class USceneComponent;
class UCameraComponent;
class UInputComponent;
class UEngineComp;
class USphereComponent;
class UUserWidget;
class ATP_MotionController;
class UWidgetComponent;
class UWind_component;

#pragma endregion

#pragma region Declaracion de la clase Ala_delta_simulado

UCLASS()
class FREEWINGS_API AAla_delta : public ACharacter
{
	GENERATED_BODY()

#pragma region Componentes

private:
	UPROPERTY(EditAnywhere, Category = "Component")
		UCameraComponent *camera; ///< Camara que se genera mediante codigo c++

	UPROPERTY(EditAnywhere, Category = "Component")
		USceneComponent *VROrigin; ///< Padre de la camara para ajustar su posicion y rotacion en modo VR

	UPROPERTY(EditAnywhere, Category = "Component")
		UEngineComp* engine_component; ///< Motor para aceleraciones

public:
	///@brief Clase o tipo del Widget de calibracion (que se arrastrara desde el blueprint)
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
		TSubclassOf<UUserWidget> calibration_widget_class; 

	UWidgetComponent *calibration_widget; ///< Widget usado para mostrar el panel de calibracion

	UPROPERTY(EditAnywhere, Category = "Component")
		UWind_component *wind_generator; ///< Componente encargado de generar el viento


private:
	/** @name Controladores VR
	* @brief Punteros a los controladores de VR */
	///@{	
	ATP_MotionController *l_controller_ATP;
	ATP_MotionController *r_controller_ATP;	
	///@brief Esfera que sigue a la mano VR izquierda
	///@details Se utiliza para poder acceder al eje de cordenadas relativas del ala delta, ya que es su padre a diferencia de la mano VR (AAla_delta::r_controller_ATP) <br>
	/// Esto se usa en las rotaciones VR para que el ala delta pueda seguir controlandose respecto su propio eje, y no de los controladores o el VROrigin
	USphereComponent *relative_right_hand;
	///@}

#pragma endregion

#pragma region Atributos

	//Variables de rotacion
private:
	/// Puntero a funcion que apuntara al metodo de rotacion que se seleccione (modo Joystick vs modo VR)
	void(AAla_delta:: *selected_rotation)(const float); //TO DO: cambiar de puntero a funcion a algo tipo std::function para mejorar la sintaxis


	/** @name Inputs
	* @brief Variables para el input / axis de rotaciones
	* @details JCU = Joystick Control Unit.  IA = Artificial Inteligence */
	///@{
	float roll_input_JCU, yaw_input, pitch_input_JCU, roll_input_IA{ 0 }, pitch_input_IA{ 0 }; 	
	///@}

	float sync_amount{ 0 }; ///< Porcenteaje de sincronizacion al que se ha llegado durante al calibracion
	float result_speed{ move_speed + boost_speed }; ///< Velocidad resultante de movimiento incluyendo boosters
	float move_speed{ initial_move_speed }; ///< Velocidad de movimiento del ala delta, sin incluir boosters
	float boost_speed{ 0.f }; ///< Velocidad del booster

public:
	/// @name Limites
	/// @brief  Variables que indican que el ala delta esta rotado en su limite y sufre el "efecto escalera" */
	/// @details Estas variable sirven para evitar temblores producidos por los cambios del limite que el roll produce sobre el pitch y viceversa <br>
	/// ya que se produce una especie de "efecto escalera".
	/// De esta forma por ejemplo al estar en el limite de roll y abajo del pitch, al subir de pitch se disminuye la velocidad de roll al <br>
	/// seguir tirando hacia el limitepara que no de tirones <br>
	/// y se ajuste poco a poco
	/// @note No es lo mismo estar en la maxima inclinacion o roll que estar en el limite. Limite esque esta en el "efecto escalera" aunque no sea el limite real, <br>
	/// sino que el limite del roll ha cambiado debido al pitch o viceversa
	///@{

	/// @brief indica que se esta en el limite de abajo
	///	@details Esto tiene varios usos:

	///	1. El hacer roll al limite estando en el pitch de abajo crea temblores. Asique se impide el movimiento
	///	@see AAla_delta::roll()
	///	2. Al estar en un roll al limite, y el pitch limite de abajo. Si se va hacia arriba un poco. Y luego se
	///	vuelve a bajar a la vez que se gira el roll al otro lado crea un temblor o deslizamiento raro
	///	@see AAla_delta::pitch() */
	bool is_at_down_pitch_limit;
	bool is_at_up_pitch_limit; ///<indica que se esta en el limite de arriba	
	bool is_at_left_limit; ///<indica que se esta en el limite de la izquierda
	bool is_at_right_limit; ///<indica que se esta en el limite de la derecha

	///@} 

public:

	enum class State { Calibration, Run, None } state; ///< Variable para maquina de estados

	FVector wind_force{ 0.f,0.f,0.f };
	/// @name Variables blueprint Modo VR
	/// @brief  Variables modificables mediante blueprints relacionadas con el modo VR */
	///@{

private:
	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Indica si esta activado el modo VR o no"))
		bool using_VR{ false }; ///< Indica si esta activado el modo VR o no.

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Sensibilidad de sincronizacion. Cuanto mas alto menos sensible sera."))
		float sync_sensibility{ 0.5f }; ///< Sensibilidad de sincronizacion. Cuanto mas alto menos sensible sera		

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Activa el dibujo del box o mando virtual"))
		bool draw_debug_box{ true }; ///< Indica si esta activado el dibujo del box o mando virtual

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Activa el dibujo de un box o mando virtual auxiliar que tiene un offset respecto el original"))
		bool draw_aux_debug_box{ true }; ///< Activa el dibujo de un box o mando virtual auxiliar que tiene un offset respecto el original

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Radio de la esfera que representa el mando en el box virtual"))
		float draw_hand_sphere_radius{ 10 }; ///< Radio de la esfera que representa el mando en el box virtual

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Radio de las esferas que representan los limites"))
		float draw_limit_sphere_radius{ 10 }; ///< Radio de las esferas que representan los limites del box virtual

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Segundos que habra que estar quieto para que se calibran las gafas VR"))
		float calibration_time{ 4.f }; ///< Segundos que habra que estar quieto para que se calibren las gafas VR

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Margen donde se mostrara el Debug Box o mando virtual y por lo tanto cambiara el punto de referencia de las rotaciones. PD: No tocar la Z que esta bugg"))
		FVector hand_pivot_point; ///< Posicion o margen donde se mostrara el Debug Box o mando virtual

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Margen respecto al pivot point original, donde se mostrara el mando virtual auxiliar"))
		FVector aux_pivot_point; ///< Posicion o margen donde se mostrara el Debug Box o mando virtual auxiliar

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Distancia horizontal o longitud del stick. Debe ajustarse para que la persona pueda tocar los limites izquierdo y derecho del box virtual"))
		float stick_distance; ///< Distancia horizontal del stick

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Margen o posicion para ajustar bien el ala delta al jugador. X = vertical, Y = horizontal, Z = profundidad"))
		FVector margin { 0, 0, 160 }; ///< Margen o posicion para ajustar bien el ala delta al jugador

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Altura de la persona. Influye en lo cerca o lejos que aparece el ala delta"))
		int person_height{ 12 }; ///< Altura de la persona. Influye en lo cerca o lejos que aparece el ala delta

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Distancia vertical hacia adelante que puede moverse el jugador"))
		float up_person_dist{ 12 }; ///< Distancia vertical hacia adelante que puede moverse el jugador

	UPROPERTY(EditAnywhere, Category = "Modo VR", meta = (ToolTip = "Distancia vertical hacia hacia atras que puede moverse el jugador"))
		float down_person_dist{ 12 }; ///< Distancia vertical hacia hacia atras que puede moverse el jugador

	///@} // Se acaba el grupo de modo VR

	/// @name Variables editables por blueprints de movimiento
	/// @brief  Variables modificables mediante blueprints relacionadas con el movimiento */
	///@{
private:
	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad inicial de movimiento del ala delta"))
		float initial_move_speed{ 2000.f }; ///< Velocidad de movimiento inicial

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad minima que se puede alcanzar"))
		float min_move_speed{ 10 }; ///< Velocidad minima que se puede alcanzar

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad maxima que se puede alcanzar"))
		float max_move_speed{ 100 }; ///< Velocidad maxima que se puede alcanzar

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Aceleracion al bajar"))
		float down_aceleration{ 10.f }; ///< Aceleracion al bajar

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Aceleracion al subir"))
		float up_aceleration{ 10.f }; ///< Aceleracion al subir

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad de alabeo"))
		float roll_speed{ 100 }; ///< Velocidad de alabeo

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad de inclinación"))
		float pitch_speed{ 100 }; ///< Velocidad de inclinacion

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad de alabeo"))
		float roll_speed_JCU{ 100 }; ///< Velocidad de alabeo

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad de alabeo"))
		float roll_speed_VR{ 100 }; ///< Velocidad de alabeo

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad de inclinación"))
		float pitch_speed_JCU{ 100 }; ///< Velocidad de inclinacion

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad de inclinación"))
		float pitch_speed_VR{ 100 }; ///< Velocidad de inclinacion

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Angulo maximo de alabeo"))
		float max_roll{ 70 }; ///< Angulo maximo de alabeo

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Angulo maximo de inclinacion"))
		float max_pitch{ 80 }; ///< Angulo maximo de inclinacion

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Porcentaje respecto a los limites horizontales del movimiento que ocupa la death zone"))
		float death_zone_horizontal_percent{ 50.f }; ///< Porcentaje respecto a los limites horizontales del movimiento que ocupa la death zone

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Porcentaje respecto a los limites verticales del movimiento que ocupa la death zone"))
		float death_zone_vertical_percent{ 50.f }; ///< Porcentaje respecto a los limites verticales del movimiento que ocupa la death zone

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad de yaw al hacer roll"))
		float yaw_speed{ 100 }; ///< Velocidad de yaw al hacer roll (sin inclinacion)

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Velocidad de movimiento lateral al hacer roll. Es lineal, solo alcanzara el maximo en el caso de un roll de 90 grados"))
		float roll_lateral_speed{ 100 }; ///< Velocidad de movimiento lateral al hacer roll. Es lineal, solo alcanzara el maximo en el caso de un roll de 90 grados

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Fuerza o velocidad con la que se tira constantemente del ala delta hacia abajo (gravedad)"))
		float max_down_force{ 200 }; ///< Fuerza o velocidad con la que se tira constantemente del ala delta hacia abajo (gravedad)
	
	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Fuerza o velocidad con la que se tira constantemente del ala delta hacia abajo (gravedad)"))
		float min_down_force{ 20 }; ///< Fuerza o velocidad con la que se tira constantemente del ala delta hacia abajo (gravedad)

	UPROPERTY(EditAnywhere, Category = "Variables de movimiento", meta = (ToolTip = "Fuerza adicional que tira del ala delta hacia abajo al hacer roll, solo alcanzara el maximo en el caso de un roll de 90 grados"))
		float down_roll_force{ 100 }; ///< Fuerza adicional que tira del ala delta hacia abajo al hacer roll
	///@} 
		
#pragma endregion

#pragma region Metodos

#pragma region Inicializaciones
private:
	void init();				   ///< Inicializa todos los componentes necesarios
	void initialize_camera();	   ///< Inicializa todo lo necesario para que funcione la camara
	void select_rotation_mode();   ///< Elige entre rotacion por axis de teclado / mando de videoconsola o rotacion por mando VR
	void initialize_controllers(); ///< Asigna el puntero al mando de VR


#pragma endregion

#pragma region Getters

public:
	// TODO: quizas haya que cambiar el move speed por result speed
	float get_speed() { return move_speed; }
	float get_min_speed() { return min_move_speed; }
	float get_max_speed() { return max_move_speed; }
	float get_max_roll() { return max_roll; }
	float get_max_pitch() { return max_pitch; }
	float get_pitch_input_JCU() { return pitch_input_JCU; }
	AAla_delta::State get_state() { return state; }
	float get_result_speed() { return result_speed; }


#pragma endregion

#pragma region Setters

	void set_move_speed(float move) { move_speed = move; }
	void set_booster_speed(float boost) { boost_speed = boost; }
	void set_state(AAla_delta::State s) { state = s; }

#pragma endregion

#pragma region Manejo de movimiento y rotaciones

private:
	void forward_movement(const float delta_time); ///< Desplaza el ala delta adelante con la velocidad asignada a la variable move_speed
public:
	void pitch(const float delta_time, const float axis_input, const float speed);		 ///< Inclinacion
	void roll(const float delta_time, const float axis_input, const float speed);		 ///< Alabeo
	void yaw(const float delta_time);		 ///< Deriva
	
private:

	/// @name Ajustes de axis
	/// @brief   Asignacion de inputs para rotaciones 
	/// @details Estos metodos son llamados al apretar los inputs, ya que se bindean con SetupPlayerInputComponent en el constructor. O por el joystick VR en caso de usar VR
	///@{
	void set_roll_axis(float axis_value) { roll_input_JCU = axis_value; }	
	void set_pitch_axis(float axis_value) { pitch_input_JCU = axis_value; }
    void set_yaw_axis(float axis_value) { yaw_input = axis_value; }
	///@}

private:
	// Rotaciones
	void rotate_VR(const float delta_time); ///< Llama los metodos de pitch_VR, roll_VR y yaw_VR
	void rotate_JCU(const float delta_time); ///< Maneja las rotaciones para el joystick

	// Rotaciones VR
	void pitch_VR(const float delta_time);  ///< Maneja la inclinacion en modo VR
	void roll_VR(const float delta_time);   ///< Maneja el alabeo para modo VR
	void yaw_VR(const float delta_time);    ///< Maneja la deriva para modo VR

	// Rotaciones modo IA
	void IA_rotation(float delta_time); ///< Maneja las rotaciones modo IA

public:
	enum class Hor_dir { NONE, Left, Right } hor_dir;
	enum class Vert_dir { NONE, Up, Down } vert_dir;
	void go_left() { hor_dir = Hor_dir::Left; }
	void go_right() { hor_dir = Hor_dir::Right; }
	void go_up() { vert_dir = Vert_dir::Up; }
	void go_down() { vert_dir = Vert_dir::Down; }
	void go_straight() { vert_dir = Vert_dir::NONE; hor_dir = Hor_dir::NONE; }

	// Calibracion de gafas VR
	void calibrate(float delta_time); ///< Calibra la posicion y rotacion para las gafas VR
	void hide_HUD(); ///< Oculta el Widget de calibracion
	void draw_debug_map(FVector _margin);

	void reset_horizontal_limits() { is_at_up_pitch_limit = is_at_down_pitch_limit = false; }
	void reset_vertical_limits() { is_at_right_limit = is_at_left_limit = false; }

#pragma endregion


#pragma region Eventos y metodos proporcionados por Unreal

private: AAla_delta(); // Constructor

protected:
	virtual void BeginPlay() override;
	
	void engineOnPressed();
	void engineOnReleased();

private:
	virtual void Tick(float DeltaTime) override;
	void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

#pragma endregion
#pragma endregion

};

#pragma endregion
