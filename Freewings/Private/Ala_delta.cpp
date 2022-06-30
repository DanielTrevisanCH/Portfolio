/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		17/10/2020
  */

#pragma region Includes y declaraciones

#include "Ala_delta.h"
#include "EngineComp.h"
#include "BatteryComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ChildActorComponent.h"
#include "Engine/Engine.h"
#include "MotionControllerComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "../VirtualReality/TP_MotionController.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Collision_detector.h"
#include "Utility.h"
#include "UI/Game_HUD.h"
#include "UI/Calibration_widget.h"
#include "Components/WidgetComponent.h"
#include "Wind_component.h"

#pragma endregion

#pragma region Inicializaciones

AAla_delta::AAla_delta()
{
	// Se habilita el metodo Tick
	PrimaryActorTick.bCanEverTick = true;

	relative_right_hand = CreateDefaultSubobject<USphereComponent>("Relative Right Hand");

	engine_component = CreateDefaultSubobject<UEngineComp>("Engine Component");
	engine_component->set_speed(move_speed);

	wind_generator = CreateDefaultSubobject<UWind_component>("Noise Component");

	// Se inicilizan los componentes que puedan crearse en tiempo de construccion: camara
	// Los controladores no pueden ya que usan GetWorld y AttachToComponent asique se crean en BeginPlay
	init();

	// Se posee al jugador 1 para poder recibir input
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AAla_delta::BeginPlay()
{
	Super::BeginPlay();

	// Elige el modo de rotacion dependiendo de si el modo VR o no
	select_rotation_mode();

	// Se spawnea e inicializa los controladores y se crea sus ajustes 
	initialize_controllers();

	// Se encuentra el widget de calibracion de los componentes del ala delta
	calibration_widget = FindComponentByClass<UWidgetComponent>();

	move_speed = initial_move_speed;
	result_speed = move_speed + boost_speed;

	// Se encuentra el detector de colisiones en la escena
	// TODO: Hacer que Spawnee directamente desde c++
	TArray<AActor *> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACollision_detector::StaticClass(), FoundActors);
	//collision_detector = (ACollision_detector *)(FoundActors[0]);
	//collision_detector->set_ala_delta_ptr(this);

	if (using_VR)
	{
		//UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Type::Floor);
		state = State::Calibration;
	}
	else
	{
		VROrigin->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform);

		state = State::Run;
		hide_HUD();
	}
}

void AAla_delta::init()
{
	initialize_camera();
}

//Creación de la cámara
void AAla_delta::initialize_camera()
{
	camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	VROrigin = CreateDefaultSubobject<USceneComponent>("VROrigin");
	camera->SetupAttachment(VROrigin);

	VROrigin->SetWorldTransform(camera->GetComponentTransform());
}

// Spawn and attach both motion controllers
void AAla_delta::initialize_controllers() //Solo se maneja el controlador derecho
{
	const FTransform SpawnTransform = FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f), FVector(1.0f, 1.0f, 1.0f)); // = FTransform::Identity;

	// "Hand" is available by checking "Expose on Spawn" in the variable on BP_MotionController.
	//  Expose on Spawn Variable parameter setup in c++ -> Use SpawnActorDeferred
	l_controller_ATP = GetWorld()->SpawnActorDeferred<ATP_MotionController>(ATP_MotionController::StaticClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (l_controller_ATP)
	{
		l_controller_ATP->Hand = EControllerHand::Left;
		l_controller_ATP->FinishSpawning(SpawnTransform); // UGameplayStatics::FinishSpawningActor(LeftController, SpawnTransform);
		l_controller_ATP->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	r_controller_ATP = GetWorld()->SpawnActorDeferred<ATP_MotionController>(ATP_MotionController::StaticClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (r_controller_ATP)
	{
		r_controller_ATP->Hand = EControllerHand::Right;
		r_controller_ATP->FinishSpawning(SpawnTransform);
		relative_right_hand->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		r_controller_ATP->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void AAla_delta::select_rotation_mode()
{
	selected_rotation = using_VR ? &AAla_delta::rotate_VR : &AAla_delta::rotate_JCU;
}

#pragma endregion

#pragma region Rotaciones

#pragma region Modo teclado


//Maneja la rotación del ala delta en los tres ejes controlada por el joystick (JCU = Joystick Control Unit)
void AAla_delta::rotate_JCU(const float delta_time)
{
	roll(delta_time, roll_input_JCU, roll_speed_JCU);

	pitch(delta_time, pitch_input_JCU, pitch_speed_JCU);
	yaw(delta_time);
}

// Alabeo
void AAla_delta::roll(const float delta_time, const float axis_input, const float speed)
{

	if(axis_input != 0)
	{
		const float max_roll_right{ max_pitch - wind_generator->get_roll_offset() };
		const float max_roll_left{ max_pitch + wind_generator->get_roll_offset() };

		Utility::print("max roll left: " + FString::SanitizeFloat(max_roll_left));
		//Utility::print("max roll right: " + FString::SanitizeFloat(max_roll_right));

		// Velocidad de rotacion final que tendra el alabeo
		const float rotation_speed
		{
			delta_time *
			// Valor entre 0 y roll_speed dependiendo del axis input usando progresion Ease In	
			UKismetMathLibrary::Ease(0, speed, axis_input, EEasingFunc::EaseIn) * FMath::Sign(axis_input)
		};

		const bool going_left{ axis_input > 0 };
		if (going_left) Utility::print("right");
		if (GetMesh()->GetComponentRotation().Roll >= 0)
		{
			const float current_roll{ GetMesh()->GetComponentRotation().Pitch };

			// Se comprueba que el ala delta no supere los limites de mas roll left y max roll right
			// Y despues simplemente se anade el roll. Ya que sigue la rotacion por defecto que parece 
			// un efecto de "de caer con el peso"
			if ((!going_left && current_roll > -max_roll_right) || (going_left && current_roll < max_roll_left))
			{
				// Esto es para solucionar los temblores del "efecto escalera" al ir del roll de abajo hacia el roll de arriba
				if ((going_left && is_at_left_limit) || (!going_left && is_at_right_limit))
					// Se anade poco roll para que no se noten temblores. Y por esto tendra un efecto
					// de roll progresivo y lento  en caso de haber empezado el roll  maximo con baja 
					// inclinacion y haya continuado con el roll pero inclinandose hacia arriba
					GetMesh()->AddRelativeRotation(FRotator(delta_time * 25 * axis_input, 0, 0));
				// En caso de que no este en un limite se anadew el roll sin ninguna condicion adicional
				else
				{
					GetMesh()->AddRelativeRotation(FRotator(rotation_speed, 0, 0));
					is_at_right_limit = is_at_left_limit = false;
				}

				// Se pierde el offset del viento roll, ya que no esta en una situacion donde no pueda hacer mas roll
				// (Ya que esta aplicando rotacion) Por lo que se pierde el "efecto pared" del viento
				wind_generator->reset_roll_offset();
			}
		}

		// Si mira hacia abajo se rota sobre el eje forward
		else
		{
			// Abajo en la barra si esta en un limite de roll no podra moverse. 
			// Ya que da pequenos tirones que dependen del smooth_speed: cuanto mas grande a la vez se hacen mas fuertes los tirones
			//if (!(is_at_down_pitch_limit && ((going_right && is_at_right_limit) || (!going_right && is_at_left_limit))))
			{

				// Explicacion de la proyeccion de un vector sobre un plano: https://www.youtube.com/watch?v=qz3Q3v84k9Y

				// Vector normal del plano de vista. Para darle una referencia al signo del angulo de giro
				const FVector view_plane{ GetMesh()->GetRightVector() };

				// Vector del giro sobre el que se sacara el angulo
				const FVector dir_vector{ GetMesh()->GetUpVector() };

				// Proyeccion del vector director sobre el plano de proyeccion (para conseguir el angulo con signo)

				FVector zlocked_forward_vector{ FVector(GetMesh()->GetRightVector().X, GetMesh()->GetRightVector().Y, 0) };
				zlocked_forward_vector.Normalize();

				// Vector normal del plano sobre el que se proyectara el vector director (Es el plano que recorre verticalmente y arriba al ala delta)
				const FVector projection_plane{ FVector::UpVector.RotateAngleAxis(90, zlocked_forward_vector) };


				// Se calcula la proyeccion del vector director sobre el vector normal del plano (projection_plane)
				
				const FVector normal_projection{ (FVector::DotProduct(dir_vector, projection_plane) / FMath::Pow(projection_plane.Size(), 2) * projection_plane) };


				// Se calcula finalmente la proyeccion del vector director sobre el plano de proyeccion.
				FVector projected_vector{ dir_vector - normal_projection };
				projected_vector.Normalize();

				// Al hacer giros de 90 grados se invierte el vector de referencia. Asique se pone para que siempre mire hacia arriba y puedan usarse mas de 90 grados
				if (projected_vector.Z < 0) projected_vector.Z = -projected_vector.Z;

				// Se calcula la diferencia de angulo entre el vector proyectado y el vector director
				float angle{ FMath::RadiansToDegrees(acosf(FVector::DotProduct(projected_vector, dir_vector))) };

				// Plano que forma el vector proyectado con el vector director
				const FVector rotation_plane = FVector::CrossProduct(projected_vector, dir_vector);

				// Si el plano proyectado coincide con el plano de vista se revierte el angulo. 
				// Para que a un lado del plano de rotacion el angulo sea positivo y al otro lado sea negativo
				if (FVector::DotProduct(view_plane, rotation_plane) < 0)
					angle = -angle;
				Utility::print("current roll: " + FString::SanitizeFloat(angle));

				FVector origen{ GetMesh()->GetComponentLocation() - GetMesh()->GetRightVector() * 100 };
				DrawDebugLine(GetWorld(), origen, origen + projected_vector * 100, FColor::Red, false, -1, 0, 5.f);
				DrawDebugLine(GetWorld(), origen, origen + projection_plane * 100, FColor::Black, false, -1, 0, 5.f);
				DrawDebugLine(GetWorld(), origen, origen + dir_vector * 100, FColor::White, false, -1, 0, 5.f); 
				
				// Se comprueba que el angulo no se salga de los limites
				if ((going_left && angle > -max_roll_left) || (!going_left && angle < max_roll_right))
				{
					// Eje sobre el que se realizara el giro
					const FVector rotation_axis{ GetMesh()->GetRightVector() };
					
					// La smooth speed es la velocidad que se aplicara en los limites para evitar el "efecto escalera"
					// Que crea temblores al aumentar el pitch mientras se esta al limite del roll
					const float smooth_speed{ delta_time * 60 * axis_input };

					// Esta en el limite donde se aplica el efecto smooth ?
					const bool is_at_smooth_limit{ (going_left && is_at_left_limit) || (!going_left && is_at_right_limit) };


					FRotator new_rotation;
			
					if (is_at_smooth_limit)
					{
						const float value = UKismetMathLibrary::Ease(0, speed * 2, 1 - FMath::Abs(angle) / max_roll, EEasingFunc::EaseIn);
						new_rotation = UKismetMathLibrary::RotatorFromAxisAndAngle(rotation_axis, value * delta_time * -axis_input);
					}
					else
					{
						// Se desactiva los limites, ya que no se esta empujando hacia ellos
						is_at_right_limit = is_at_left_limit = false;
						new_rotation = UKismetMathLibrary::RotatorFromAxisAndAngle(rotation_axis, -rotation_speed);
					}

					// Se calcula la rotacion necesaria para el eje Right del ala delta que se aplicara
					/*const FRotator new_rotation
					{
						is_at_smooth_limit ?
						UKismetMathLibrary::RotatorFromAxisAndAngle(rotation_axis, -smooth_speed) :
						UKismetMathLibrary::RotatorFromAxisAndAngle(rotation_axis, -rotation_speed)
					};
					*/
					// Se aplica la rotacion en el roll
					GetMesh()->AddWorldRotation(new_rotation);

				
					// Se pierde el offset del viento roll, ya que no esta en una situacion donde no pueda hacer mas roll
					// (ya que esta aplicando rotacion) por lo que se pierde el "efecto pared" del viento
					wind_generator->reset_roll_offset();
				}
				else
				{
					if (going_left) is_at_left_limit = true;
					if (!going_left) is_at_right_limit = true;
				}
			}
		}

		//TODO: el max roll no siempre va a ser el max yaw speed, ya que si es 120 grados por ejemplo...
		yaw_input = GetMesh()->GetComponentRotation().Pitch / max_roll;
	}

	// Si no hay input se resetean los valores temporales para limites
	else
	{
		wind_generator->reset_roll_offset();
		is_at_right_limit = is_at_left_limit = false;
	}
}

void AAla_delta::yaw(const float delta_time)
{
	AddControllerYawInput(-yaw_input * yaw_speed * delta_time);
}

//inclinacion

// Inclinacion
void AAla_delta::pitch(const float delta_time, const float axis_input, const float speed)
{
	const float max_pitch_up{ max_pitch + wind_generator->get_pitch_offset() };
	const float max_pitch_down{ max_pitch - wind_generator->get_pitch_offset() };

	// Limite de la zona muerta entre 0 y 1
	const float death_zone_limit{ death_zone_vertical_percent / 100 };

	// La velocidad tiene un efecto Ease In.
	float rotation_speed
	{
		axis_input * delta_time * speed
		//UKismetMathLibrary::Ease(0, speed, FMath::Abs(axis_input / death_zone_limit), EEasingFunc::EaseIn) 
	};

	// Si no hay input se resetea el valor temporal del offset del viento pitch
	if (axis_input == 0) wind_generator->reset_pitch_offset();

	// Si el jugador se inclina hacia abajo en la barra (para elevarse o tirar del roll)
	if (axis_input > 0)
	{
		// Se indica que el ala delta ya no esta en el limite de abajo
		// (Para ver el uso de esto con detalle mirar en la declaracion)
		is_at_down_pitch_limit = false;

		// Si no se ha alcanzado el pitch maximo entonces se anade el pitch sin condiciones especiales
		if (!is_at_up_pitch_limit && GetMesh()->GetComponentRotation().Roll < max_pitch_up)
		{
			wind_generator->reset_pitch_offset();
			GetMesh()->AddRelativeRotation(FRotator(0, 0, rotation_speed));
			//is_at_up_pitch_limit = false;
		}
		// Si se ha alcanzado el pitch maximo entonces solo rota en el eje UP del mundo ({ 0, 0, -1 }) osea izquierda / derecha. Ya que no se puede inclinar mas
		else
		{
			// Eje sobre el que se hara la rotacion
			const FVector up_axis{ 0, 0, -1 };
			// La fuerza del roll sera 0 si no hay roll, y 1 si el roll esta al maximo 
			const float roll_force{ GetMesh()->GetComponentRotation().Pitch / max_roll };
			// Velocidad con la que se realizara la rotacion hacia abajo
			const float actual_speed{ roll_force * rotation_speed };
			// Se calcula la rotacion necesaria para el eje Up que se aplicara al ala delta
			const FRotator new_rotation = UKismetMathLibrary::RotatorFromAxisAndAngle(up_axis, actual_speed);
			GetMesh()->AddWorldRotation(new_rotation);
			is_at_up_pitch_limit = true;
		}
	}
	// Si el jugador se inclina hacia arriba en la barra (para bajar)
	else
	{
		FVector zlocked_forward_vector{ FVector(GetMesh()->GetRightVector().X, GetMesh()->GetRightVector().Y, 0) };
		zlocked_forward_vector.Normalize();

		// Para el pitch de abajo, el angulo por defecto (rotation.Roll) no lo devuelve bien. Asique se calcula manualmente
		const float actual_pitch
		{ Utility::angle_between(zlocked_forward_vector, GetMesh()->GetRightVector()) };
		const bool pitch_is_up{ GetMesh()->GetComponentRotation().Roll >= 0 };

		const bool is_not_at_minimum_pitch{ (!pitch_is_up && actual_pitch < max_pitch_down) || pitch_is_up };
		
		if (is_not_at_minimum_pitch)  
		{
			// Soluciona el punto 2 de su definicion: para mas detalle mirar la definicion de is_at_down_pitch_limit
			// Al estar en el limite de abajo, no permite bajar mas. Aunque el limite cambie debido a un cambio en el roll	
			// La razon de porque hay dos comprobaciones es porque se puede estar en el limite del pitch, pero no abajo del todo
			// ya que cambiar el roll puede influir en el limite alcanzable de pitch
			if (!is_at_down_pitch_limit)
			{
				const FVector rotation_axis{ FVector::UpVector.RotateAngleAxis(90, -zlocked_forward_vector) };
				const FRotator new_rotation{ UKismetMathLibrary::RotatorFromAxisAndAngle(rotation_axis, rotation_speed) };

				GetMesh()->AddWorldRotation(new_rotation);
				wind_generator->reset_pitch_offset();
			}
		}
		else is_at_down_pitch_limit = true;
		
		// Se indica que el ala delta ha llegado al limite de pitch inferior
		// (Para ver el uso de esto con detalle mirar en la declaracion)
		is_at_up_pitch_limit = false;
	}
}

#pragma endregion

#pragma region modo VR	

void AAla_delta::rotate_VR(const float delta_time)
{
	roll_VR(delta_time);
	pitch_VR(delta_time);
	yaw_VR(delta_time);
}

// Mapea el valor de la posición del mando VR clampeando de max_roll y -max_roll a 1 y -1, y se asigna como valor de roll
void AAla_delta::roll_VR(const float delta_time)
{
	const float left_max_roll{ max_roll + wind_generator->get_roll_offset() };
	const float right_max_roll{ max_roll - wind_generator->get_roll_offset() };
	/*//Distancia actual respecto al punto inicial
	const float current_dist{ relative_right_hand->GetRelativeLocation().X + hand_pivot_point.Y - margin.Y };

	//Axis o posicion que tiene el jugador en la barra. 0 es "el centro", -1 tocando el lado extremo de la izquierda y 1 el lado extr derecho
	const float player_roll_pos_axis{ Utility::map_ranged_clamped(current_dist, -stick_distance, stick_distance, -1.f, 1.f) };

	roll(delta_time, -player_roll_pos_axis, roll_speed_VR); */

	const float current_dist{ relative_right_hand->GetRelativeLocation().X + hand_pivot_point.Y - margin.Y }; //Distancia actual respecto al punto inicial

	//Axis o posicion que tiene el jugador en la barra. 0 es "el centro", -1 tocando el lado extremo de la izquierda y 1 el lado extr derecho
	const float player_roll_pos_axis{ Utility::map_ranged_clamped(current_dist, -stick_distance, stick_distance, -1.f, 1.f) };

	const bool going_right{ player_roll_pos_axis > 0 };

	// El limite de la zona muerta entre 0 y 1
	const float death_zone_limit{ death_zone_horizontal_percent / 100 };

	// Si se esta en la zona muerta la velocidad tendra un efecto Ease In.
	const float rotation_speed
	{
		player_roll_pos_axis * delta_time *
		(
			player_roll_pos_axis > -death_zone_limit && player_roll_pos_axis < death_zone_limit ? // Esta en la zona muerta ?
				UKismetMathLibrary::Ease(0, roll_speed, FMath::Abs(player_roll_pos_axis / death_zone_limit), EEasingFunc::EaseIn) :
				roll_speed
		)
	};

	// Si el pitch mira hacia arriba simplemente se anade roll
	if (GetMesh()->GetComponentRotation().Roll >= 0)
	{
		if ((player_roll_pos_axis < 0 && GetMesh()->GetComponentRotation().Pitch < max_roll) || (player_roll_pos_axis > 0 && GetMesh()->GetComponentRotation().Pitch > -max_roll))
			GetMesh()->AddRelativeRotation(FRotator(-rotation_speed, 0, 0));
		
		is_at_left_limit = is_at_right_limit = false;
	}
	// Si mira hacia abajo se rota sobre el eje forward
	else
	{
		// Explicacion de la proyeccion de un vector sobre un plano: https://www.youtube.com/watch?v=qz3Q3v84k9Y

		// Vector normal del plano de vista. Para darle una referencia al signo del angulo de giro
		const FVector view_plane{ GetMesh()->GetRightVector() };

		// Vector del giro sobre el que se sacara el angulo
		const FVector dir_vector{ GetMesh()->GetUpVector() };

		// Proyeccion del vector director sobre el plano de proyeccion (para conseguir el angulo con signo)

		FVector zlocked_forward_vector{ FVector(GetMesh()->GetRightVector().X, GetMesh()->GetRightVector().Y, 0) };
		zlocked_forward_vector.Normalize();

		// Vector normal del plano sobre el que se proyectara el vector director (Es el plano que recorre verticalmente y arriba al ala delta)
		const FVector projection_plane{ FVector::UpVector.RotateAngleAxis(90, zlocked_forward_vector) };

		// Se calcula la proyeccion del vector director sobre el vector normal del plano (projection_plane)
		const FVector normal_projection{ (FVector::DotProduct(dir_vector, projection_plane) / FMath::Pow(projection_plane.Size(), 2) * projection_plane) };

		// Se calcula finalmente la proyeccion del vector director sobre el plano de proyeccion.
		FVector projected_vector{ dir_vector - normal_projection };
		projected_vector.Normalize();

		// Al hacer giros de 90 grados se invierte el vector de referencia. Asique se pone para que siempre mire hacia arriba y puedan usarse mas de 90 grados
		if (projected_vector.Z < 0) projected_vector.Z = -projected_vector.Z;

		// Se calcula la diferencia de angulo entre el vector proyectado y el vector director
		float angle{ FMath::RadiansToDegrees(acosf(FVector::DotProduct(projected_vector, dir_vector))) };

		// Plano que forma el vector proyectado con el vector director
		const FVector rotation_plane = FVector::CrossProduct(projected_vector, dir_vector);

		// Si el plano proyectado coincide con el plano de vista se revierte el angulo. 
		// Para que a un lado del plano de rotacion el angulo sea positivo y al otro lado sea negativo
		if (FVector::DotProduct(view_plane, rotation_plane) < 0)
			angle = -angle;


		// Se comprueba que el angulo no se salga de los limites
	

		if ((!going_right && angle > -max_roll && !is_at_left_limit) || (going_right && angle < max_roll && !is_at_right_limit))
		{
			// Eje sobre el que se realizara el giro
			const FVector rotation_axis{ GetMesh()->GetRightVector() };

			// Se calcula la rotacion necesaria para el eje Right del ala delta que se aplicara
			const FRotator new_rotation = UKismetMathLibrary::RotatorFromAxisAndAngle(rotation_axis, rotation_speed);

			GetMesh()->AddWorldRotation(new_rotation);
			is_at_left_limit = is_at_right_limit = false;
		}
		else if (going_right) is_at_right_limit = true;	
		else if (!going_right) is_at_left_limit = true;
	}

	// TODO: El yaw no se si esta bien del todo asi. Ya que si esta bajando y esta a la derecha tmb se lo esta anadiendo
	yaw_input = GetMesh()->GetComponentRotation().Pitch / max_roll;

}

void AAla_delta::pitch_VR(const float delta_time)
{
	const float max_pitch_up{ max_pitch - wind_generator->get_pitch_offset() };
	const float max_pitch_down{ max_pitch - wind_generator->get_pitch_offset() };

	/*//Distancia actual respecto al punto inicial
	const float current_dist{ relative_right_hand->GetRelativeLocation().Y + hand_pivot_point.X - margin.X - person_height };

	// Limite de posicion vertical. Se ajusta segun si el jugador se esta acercando hacia adelante o hacia atras
	const float vertical_limit{ current_dist < 0 ? up_person_dist : down_person_dist };

	//Axis o posicion que tiene el jugador en la barra: 0 es "el centro" | -1 tocando la barra con el pecho | 1 la distancia máxima que se puede alejar el jugador
	const float player_pitch_pos_axis{ FMath::Clamp(current_dist, -vertical_limit, +vertical_limit) / vertical_limit };

	pitch(delta_time, player_pitch_pos_axis, pitch_speed_VR); */

	const float current_dist{ relative_right_hand->GetRelativeLocation().Y + hand_pivot_point.X - margin.X - person_height }; //Distancia actual respecto al punto inicial

	//Axis o posicion que tiene el jugador en la barra: 0 es "el centro" | -1 tocando la barra con el pecho | 1 la distancia máxima que se puede alejar el jugador
	//const float player_pitch_pos_axis{ map_ranged_clamped(current_dist, person_distance, -person_distance, 1, -1) };
	const float vertical_limit{ current_dist < 0 ? up_person_dist : down_person_dist };
	const float player_pitch_pos_axis{ FMath::Clamp(current_dist, -vertical_limit, +vertical_limit) / vertical_limit };

	// Limite de la zona muerta entre 0 y 1
	const float death_zone_limit{ death_zone_vertical_percent / 100 };

	// Si se esta en la zona muerta la velocidad tendra un efecto Ease In.
	float rotation_speed
	{
		player_pitch_pos_axis * delta_time *
		(
			UKismetMathLibrary::Ease(0, pitch_speed, FMath::Abs(player_pitch_pos_axis / death_zone_limit), EEasingFunc::EaseIn)
			//player_pitch_pos_axis > -death_zone_limit && player_pitch_pos_axis < death_zone_limit ? // Esta en la zona muerta ?
				//UKismetMathLibrary::Ease(0, pitch_speed, FMath::Abs(player_pitch_pos_axis / death_zone_limit), EEasingFunc::EaseIn) :
				//roll_speed
		)
	};

	//actual_speed *= player_pitch_pos_axis;
	// Si el jugador se inclina hacia abajo en la barra (para elevarse o tirar del roll)
	if (player_pitch_pos_axis > 0)
	{

		// Si no se ha alcanzado el pitch maximo entonces se anade el pitch sin condiciones especiales
		if (GetMesh()->GetComponentRotation().Roll < max_pitch_up)
		{
			wind_generator->reset_pitch_offset();
			GetMesh()->AddRelativeRotation(FRotator(0, 0, rotation_speed));
		}
		// Si se ha alcanzado el pitch maximo entonces solo rota en el eje UP del mundo ({ 0, 0, -1 }) osea izquierda / derecha. Ya que no se puede inclinar mas
		else
		{
			// Eje sobre el que se hara la rotacion
			const FVector up_axis{ 0, 0, -1 };
			// La fuerza del roll sera 0 si no hay roll, y 1 si el roll esta al maximo 
			const float roll_force{ GetMesh()->GetComponentRotation().Pitch / max_roll };
			// Velocidad con la que se realizara la rotacion hacia abajo
			const float actual_speed{ roll_force * rotation_speed };
			// Se calcula la rotacion necesaria para el eje Up que se aplicara al ala delta
			const FRotator new_rotation = UKismetMathLibrary::RotatorFromAxisAndAngle(up_axis, actual_speed);
			GetMesh()->AddWorldRotation(new_rotation);
		
		}
	}
	// Si el jugador se inclina hacia arriba en la barra (para bajar)
	else
		// Si el pitch no ha llegado al limite inferior
		if (GetMesh()->GetComponentRotation().Roll > -max_pitch)
		{
			FVector zlocked_forward_vector{ FVector(GetMesh()->GetRightVector().X, GetMesh()->GetRightVector().Y, 0) };
			zlocked_forward_vector.Normalize();

			//FVector rotation_axis{ FVector(-GetMesh()->GetForwardVector().X, -GetMesh()->GetForwardVector().Y, 0) };
			FVector rotation_axis{ FVector::UpVector.RotateAngleAxis(90, -zlocked_forward_vector) };
			const FRotator new_rotation = UKismetMathLibrary::RotatorFromAxisAndAngle(rotation_axis, rotation_speed);

			GetMesh()->AddWorldRotation(new_rotation);
		}
}

void AAla_delta::yaw_VR(const float delta_time)
{
	AddControllerYawInput(yaw_input * -yaw_speed * delta_time);
}

#pragma endregion

//Se asegura que el alabeo no se salga de sus límites. En cuyo caso devuelve true
/*bool AAla_delta::can_roll(float current_axis) const
{
	return false;
	//	return current_axis < 0 && GetActorRotation().Roll > -max_roll || current_axis > 0 && GetActorRotation().Roll < max_roll;
}*/

//Se asegura que la inclinación no se salga de sus límites. En cuyo caso devuelve true
/*bool AAla_delta::can_pitch(const float direction, const float pitch_limit) const
{
	return direction < 0 && GetActorRotation().Pitch > -pitch_limit || direction > 0 && GetActorRotation().Pitch < pitch_limit;
}*/

#pragma endregion


void AAla_delta::IA_rotation(float delta_time)
{
	/*if (vert_dir == Vert_dir::NONE && hor_dir == Hor_dir::NONE)
	{
		// Ir recto (y recuperar el control del ala)
		roll_input = roll_input > 0 ? FMath::Clamp(roll_input + 1.f * -delta_time, 0.f, 1.f) : FMath::Clamp(roll_input + 1.f * delta_time, -1.f, 0.f);
		pitch_input = pitch_input > 0 ? FMath::Clamp(pitch_input + 1.f * -delta_time, 0.f, 1.f) : FMath::Clamp(pitch_input + 1.f * delta_time, -1.f, 0.f);
	}
	else
	{
		switch (vert_dir)
		{
		case Vert_dir::Down:
			pitch_input = FMath::Clamp(pitch_input + 1.f * -delta_time, -1.f, 1.f);
			break;
		case Vert_dir::Up:
			pitch_input = FMath::Clamp(pitch_input + 1.f * delta_time, -1.f, 1.f);
			break;
		}

		switch (hor_dir)
		{
		case Hor_dir::Left:
			roll_input = FMath::Clamp(roll_input + 1.f * -delta_time, -1.f, 1.f);
			break;
		case Hor_dir::Right:
			roll_input = FMath::Clamp(roll_input + 1.f * delta_time, -1.f, 1.f);
			break;
		}
	}

	if (pitch_input != 0)
		if (roll_input > 0) set_yaw_axis(1);
		else if (roll_input < 0) set_yaw_axis(-1);
		else set_yaw_axis(0);
	else set_yaw_axis(0);

	//SetActorRotation(FRotator(pitch_axis * max_pitch, GetActorRotation().Yaw, roll_axis * max_roll));
	//AddControllerYawInput(yaw_axis * yaw_speed * delta_time);
	*/
}

#pragma region Movimiento

void AAla_delta::forward_movement(const float delta_time)
{
	// Movimiento forward
	AddActorWorldOffset(-GetMesh()->GetRightVector() * delta_time * result_speed);

	// Movimiento lateral que aparece al tener roll
	const float lateral_speed{ roll_lateral_speed * yaw_input * delta_time };
	AddActorWorldOffset(-GetMesh()->GetForwardVector() * lateral_speed);

	// Movimiento hacia abajo que representa la gravedad
	const float force_axis{ FMath::Abs(GetMesh()->GetForwardVector().Z) };
	const float down_force{ Utility::map_ranged_clamped(move_speed, min_move_speed, max_move_speed, max_down_force, min_down_force) };
	AddActorWorldOffset(GetActorUpVector() * delta_time * (wind_force.Z  - down_force - down_roll_force * force_axis ));

	const float aceleration{ GetMesh()->GetRightVector().Z > 0 ? down_aceleration : up_aceleration };

	// Se aplica la aceleracion / deceleracion al ir hacia arriba o abajo
	move_speed =  FMath::Clamp(move_speed + GetMesh()->GetRightVector().Z * aceleration * delta_time, min_move_speed, max_move_speed);
	
	result_speed = move_speed + boost_speed >= max_move_speed ? max_move_speed : move_speed + boost_speed;
}

#pragma endregion

void AAla_delta::calibrate(float delta_time)
{
	static FVector previous_camera_location{ camera->GetComponentLocation() };

	// Si la sincronizacion ha acabado se apaga el HUD y se cambia al estado Run para empezar el juego
	if (sync_amount > 1.f) 
	{
		state = State::Run;
		hide_HUD();

		// La camara debera seguir el mesh
		VROrigin->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform);
	}

	else // Carga la barra de progreso de sincronizacion y calibra la posicion y rotacion segun donde este mirando el jugador
	{
		// Se actualiza la cantidad de sincronizacion
		sync_amount = (camera->GetComponentLocation() - previous_camera_location).Size() < sync_sensibility ? sync_amount + delta_time / calibration_time : 0;

		// Se actualiza la cantidad de sincronizacion mostrada por el widget
		UCalibration_widget *widget{ Cast<UCalibration_widget>(calibration_widget->GetUserWidgetObject()) };
		if (widget) widget->set_progress_bar_percent(sync_amount);

		// Posicion para comprobar el frame actual contra el frame anterior y asi ver si se ha movido o no
		previous_camera_location = camera->GetComponentLocation();

		// Calibracion de la posicion del jugador (el ala delta aparecera ajustado segun la posicion de las gafas)
		FVector VectorLocation2{ GetActorLocation() - GetMesh()->GetComponentScale().X * FVector(margin.X + person_height, -margin.Y, margin.Z) };
		VROrigin->SetWorldLocation(VectorLocation2);
		VROrigin->AddLocalOffset(-camera->GetRelativeLocation());

		// Calibracion de la rotacion del jugador (el eje forward del ala delta mirara a la misma direccion que las gafas VR)
		VROrigin->SetWorldRotation(GetActorRotation());
		VROrigin->AddLocalRotation({ 0, -camera->GetComponentRotation().Yaw, 0 });
	}
}

void AAla_delta::hide_HUD()
{
	calibration_widget->SetVisibility(false);
}


void AAla_delta::draw_debug_map(FVector _margin)
{
	// Se asume que la escala es igual hacia todos los lados
	const float scale{ GetMesh()->GetComponentScale().X };
	
	// Distancias del mapa debug
	FVector horizontal_vector{ GetMesh()->GetForwardVector() * stick_distance * scale };
	FVector vertical_vector{ GetMesh()->GetUpVector() /** person_distance*/ * scale };

	FVector top_limite{ GetMesh()->GetUpVector() * up_person_dist * scale };
	FVector bot_limite{ GetMesh()->GetUpVector() * down_person_dist * scale };



	FVector pivot_point{ hand_pivot_point + _margin };


	FVector middle_point
	{
		GetMesh()->GetComponentLocation() -
		GetMesh()->GetForwardVector() * scale * (pivot_point.Y - margin.Y) -
		GetMesh()->GetRightVector() * scale * (pivot_point.Z - margin.X - person_height) +
		GetMesh()->GetUpVector() * scale * (pivot_point.X - margin.Z)
	};

	DrawDebugSphere(GetWorld(),
		GetMesh()->GetComponentLocation() -
		GetMesh()->GetUpVector() * (relative_right_hand->GetRelativeLocation().Y) +
		GetMesh()->GetForwardVector() * relative_right_hand->GetRelativeLocation().X -

		GetMesh()->GetForwardVector() * scale * (_margin.Y) -
		GetMesh()->GetRightVector() * scale * (_margin.Z - margin.X) -
		GetMesh()->GetUpVector() * scale * (-_margin.X + margin.Z - margin.X)
		, draw_hand_sphere_radius, 32, FColor::Red, false, 0.f);

	FVector left_point{ middle_point - horizontal_vector };
	FVector right_point{ middle_point + horizontal_vector };
	FVector top_point{ middle_point + top_limite };
	FVector down_point{ middle_point - bot_limite };

	// Punto limite arriba
	DrawDebugSphere(GetWorld(), top_point, draw_limit_sphere_radius, 32, FColor::Green, false, 0.f);
	// Punto limite abajo
	DrawDebugSphere(GetWorld(), down_point, draw_limit_sphere_radius, 32, FColor::Green, false, 0.f);
	// Punto limite izquieda
	DrawDebugSphere(GetWorld(), left_point, draw_limit_sphere_radius, 32, FColor::Green, false, 0.f);
	// Punto limite derecha
	DrawDebugSphere(GetWorld(), right_point, draw_limit_sphere_radius, 32, FColor::Green, false, 0.f);

	// Se dibujan los ejes que cruzan el debug box
	DrawDebugLine(GetWorld(), left_point, right_point, FColor::Green, false, -1, 0, 1.f);
	DrawDebugLine(GetWorld(), down_point, top_point, FColor::Green, false, -1, 0, 1.f);

	// Se calcula el punto de las 4 esquinas
	FVector right_top{ middle_point + top_limite + horizontal_vector };
	FVector right_down{ middle_point - bot_limite + horizontal_vector };
	FVector left_top{ middle_point + top_limite - horizontal_vector };
	FVector left_down{ middle_point - bot_limite - horizontal_vector };

	// Se dibuja las lineas externas del cuadrado
	DrawDebugLine(GetWorld(), right_top, right_down, FColor::Red, false, -1, 0, 1.f);
	DrawDebugLine(GetWorld(), right_down, left_down, FColor::Red, false, -1, 0, 1.f);
	DrawDebugLine(GetWorld(), left_down, left_top, FColor::Red, false, -1, 0, 1.f);
	DrawDebugLine(GetWorld(), left_top, right_top, FColor::Red, false, -1, 0, 1.f);

	// Distancias de la death zone vertical y horizontal
	FVector horizontal_death_zone_vector{ horizontal_vector * death_zone_horizontal_percent / 100 };
	FVector vertical_death_zone_vector{ vertical_vector * death_zone_vertical_percent / 100 };

	// Se calcula los puntos de las 4 esquinas del cuadrado de la death zone
	FVector death_top_left{ middle_point - horizontal_death_zone_vector + vertical_death_zone_vector };
	FVector death_top_right{ middle_point + horizontal_death_zone_vector + vertical_death_zone_vector };
	FVector death_down_left{ middle_point - horizontal_death_zone_vector - vertical_death_zone_vector };
	FVector death_down_right{ middle_point + horizontal_death_zone_vector - vertical_death_zone_vector };

	// Se dibujan las lineas del cuadrado de la death zone
	//DrawDebugLine(GetWorld(), death_top_right, death_down_right, FColor::Black, false, -1, 0, 1.f);
	//DrawDebugLine(GetWorld(), death_down_right, death_down_left, FColor::Black, false, -1, 0, 1.f);

	//DrawDebugLine(GetWorld(), death_down_left, death_top_left, FColor::Black, false, -1, 0, 1.f);
	//DrawDebugLine(GetWorld(), death_top_left, death_top_right, FColor::Black, false, -1, 0, 1.f);
}

#pragma region Proporcionados por el motor

void AAla_delta::Tick(float delta_time) //Se encarga principalmente de: movimiento, rotación y calibracion
{
	Super::Tick(delta_time);

	switch (state)
	{
	case State::Calibration:
		calibrate(delta_time);
		break;

	case State::Run:
		if (using_VR)
			relative_right_hand->SetWorldLocation(r_controller_ATP->GetMotionController()->GetComponentLocation());
		
		forward_movement(delta_time); // Movimiento

		wind_generator->run(*this, delta_time); // Se genera el efecto del viento
		(this->*selected_rotation)(delta_time); // Rotación. TO DO: cambiar puntero a std::function o algo asi para mejorar la sintaxis

		//IA_rotation(delta_time);
		break;

	case State::None:
		break;
	}
	

	

	// Se dibujan las cajas debug de movimiento
	if (draw_debug_box) draw_debug_map(FVector(0, 0, 0));
	if (draw_aux_debug_box) draw_debug_map(aux_pivot_point);

	FString tempString;

	/*tempString = "Energy: " + FString::SanitizeFloat(engineComp->get_battery_component()->get_current_charge());

	print(tempString, 0.f, FColor::Magenta);

	tempString = "Acceleration: " + FString::SanitizeFloat(engineComp->get_current_acceleration());

	print(tempString, 0.f, FColor::Green);

	tempString = "Max Energy: " + FString::SanitizeFloat(engineComp->get_battery_component()->get_maximum_charge());

	print(tempString, 0.f, FColor::Yellow);

	tempString = "Max Acceleration: " + FString::SanitizeFloat(engineComp->get_maximum_acceleration());

	print(tempString, 0.f, FColor::Cyan);

	tempString = "Speed: " + FString::SanitizeFloat(move_speed);

	print(tempString, 0.f, FColor::Black);*/

	/*tempString = "Wind Force: " + FString::SanitizeFloat(wind_force.Z);

	print(tempString, 0.f, FColor::Green);*/


}

// Hace que se llamen los metodos set_roll_axis y set_pitch_axis con el input
void AAla_delta::SetupPlayerInputComponent(UInputComponent *input_component) // Called to bind functionality to input
{
	Super::SetupPlayerInputComponent(input_component);
	input_component->BindAxis("Roll Input", this, &AAla_delta::set_roll_axis);
	input_component->BindAxis("Pitch Input", this, &AAla_delta::set_pitch_axis);
	input_component->BindAction("EngineGlider", IE_Pressed, this, &AAla_delta::engineOnPressed);
	input_component->BindAction("EngineGlider", IE_Released, this, &AAla_delta::engineOnReleased);
}

void AAla_delta::engineOnPressed() {
	engine_component->apply_acceleration(max_move_speed, true);
}

void AAla_delta::engineOnReleased() {
	engine_component->apply_acceleration(0, false);
}
#pragma endregion

