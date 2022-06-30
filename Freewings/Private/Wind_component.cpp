/** Copyright:	@FreeWings
  *	@author:	Pablo Arkadiusz Kalemba
  *	@date:		21/04/2021
  */

#include "Wind_component.h"
#include "SimplexNoiseBPLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "Utility.h"
#include "Ala_delta.h"

UWind_component::UWind_component()
{
	PrimaryComponentTick.bCanEverTick = false; // Se desactiva el Tick
}

void UWind_component::run(AAla_delta &hang_glider, const float delta_time)
{
	// Se genera el efecto de viento
	if(activated) generate_wind_effect(hang_glider, delta_time); 

	// Se ajusta el volumen del viento en funcion de la velocidad del ala delta
	adjust_wind_volume(hang_glider); 
}

void UWind_component::adjust_wind_volume(AAla_delta &hang_glider)
{
	// El volumen final depende de la velocidad del ala delta
	const float volume
	{ Utility::map_ranged_clamped(hang_glider.get_speed(), hang_glider.get_min_speed(), hang_glider.get_max_speed(), 0.2f, 1) };

	wind_audio->SetVolumeMultiplier(volume);
}

void UWind_component::generate_wind_effect(AAla_delta &hang_glider, const float delta_time)
{
	const float time_since_start{ UGameplayStatics::GetRealTimeSeconds(GetWorld()) };

	// Se genera el valor del noise de este frame en funcion del tiempo. 
	// Al pitch se le suma 13 para que genere un valor distinto
	const float roll_noise_value{ USimplexNoiseBPLibrary::SimplexNoise1D(time_since_start, wind_in_factor) };
	const float pitch_noise_value{ USimplexNoiseBPLibrary::SimplexNoise1D(time_since_start + 13, wind_in_factor) };

	// La fuerza del viento tanto roll como pitch dependera de la velocidad del ala delta
	const float roll_speed_value
	{ Utility::map_ranged_clamped(hang_glider.get_speed(), hang_glider.get_min_speed(), hang_glider.get_max_speed(), roll_max_wind, roll_min_wind) };

	const float pitch_speed_value
	{ Utility::map_ranged_clamped(hang_glider.get_speed(), hang_glider.get_min_speed(), hang_glider.get_max_speed(), pitch_max_wind, pitch_min_wind) };

	// Se crea el efecto de viento roll y pitch

	if (FMath::Abs<float>(roll_offset) > roll_windwall_reset)
	{
		reset_roll_offset();
		hang_glider.reset_vertical_limits();
	}

	if (FMath::Abs<float>(pitch_offset) > pitch_windwall_reset)
	{
		reset_pitch_offset();
		hang_glider.reset_horizontal_limits();
	}

	roll_wind(hang_glider, delta_time, roll_noise_value, roll_speed_value);
	pitch_wind(hang_glider, delta_time, pitch_noise_value, pitch_speed_value);
}

void UWind_component::BeginPlay()
{
	Super::BeginPlay();	
	find_wind_audio_component();
}

void UWind_component::find_wind_audio_component()
{
	// Se busca el sonido de ambiente en la escena y se asigna su componente de audio al puntero wind_audio
	TArray<AActor*> found_actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAmbientSound::StaticClass(), found_actors);
	wind_audio = Cast<AAmbientSound>(found_actors[0])->GetAudioComponent();
}

void UWind_component::roll_wind(AAla_delta &hang_glider, const float delta_time, const float axis_input, const float speed)
{
	const bool going_left{ axis_input > 0 };

	// Para solucionar buggs de temblor. Si esta en el limite de izquierda o derecha no se podra mover en esa direccion
	// (aunque cambie su limite por un cambio del angulo de pitch)
	if ((going_left && is_at_left_limit) || (!going_left && is_at_right_limit)) return;
	else is_at_right_limit = is_at_left_limit = false;

	if ((going_left && hang_glider.is_at_left_limit) || (!going_left && hang_glider.is_at_right_limit))
	{
		return;
	}

	const float max_roll{ hang_glider.get_max_roll() };

	// Velocidad de rotacion final que tendra el efecto de viento sobre el alabeo
	const float rotation_speed
	{
		delta_time *
		// Valor entre 0 y speed dependiendo del axis input usando progresion Ease In	
		UKismetMathLibrary::Ease(0, speed, axis_input, EEasingFunc::EaseIn) * FMath::Sign(axis_input)
	};
	
	// Explicacion de la proyeccion de un vector sobre un plano: https://www.youtube.com/watch?v=qz3Q3v84k9Y

	// Vector normal del plano de vista. Para darle una referencia al signo del angulo de giro
	const FVector view_plane{ hang_glider.GetMesh()->GetRightVector() };

	// Vector del giro sobre el que se sacara el angulo
	const FVector dir_vector{ hang_glider.GetMesh()->GetUpVector() };

	// Proyeccion del vector director sobre el plano de proyeccion (para conseguir el angulo con signo)

	FVector zlocked_forward_vector{ FVector(hang_glider.GetMesh()->GetRightVector().X, hang_glider.GetMesh()->GetRightVector().Y, 0) };
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
	float angle{ Utility::angle_between(projected_vector, dir_vector) };

	// Plano que forma el vector proyectado con el vector director
	const FVector rotation_plane = FVector::CrossProduct(projected_vector, dir_vector);

	// Si el plano proyectado coincide con el plano de vista se revierte el angulo. 
	// Para que a un lado del plano de rotacion el angulo sea positivo y al otro lado sea negativo
	if (FVector::DotProduct(view_plane, rotation_plane) < 0)
		angle = -angle;

	// Se comprueba que el angulo no se salga de los limites
	if ((going_left && angle > -max_roll) || (!going_left && angle < max_roll))
	{
		
		// Eje sobre el que se realizara el giro
		const FVector rotation_axis{ hang_glider.GetMesh()->GetRightVector() };

		// Se calcula la rotacion necesaria para el eje Right del ala delta que se aplicara
		const FRotator new_rotation
		{ UKismetMathLibrary::RotatorFromAxisAndAngle(rotation_axis, -rotation_speed) };
				
		// Se anade la rotacion calculada
		hang_glider.GetMesh()->AddWorldRotation(new_rotation);

		// Se anade el offset de rotacion para que el ala delta pueda
		// calcular un nuevo limite de rotacion en funcion de esto
		add_roll_offset(rotation_speed);		
	}
	else
	{
		 if (going_left) is_at_left_limit = true;
		 else is_at_right_limit = true;
	}
	
}

void UWind_component::pitch_wind(AAla_delta & hang_glider, const float delta_time, const float axis_input, const float speed)
{
	const float max_pitch{ hang_glider.get_max_pitch() };
	const float rotation_speed{ axis_input * delta_time * speed }; 


	// Si el viento inclina el ala delta hacia abajo (para elevarse)	
	if (axis_input > 0)
	{
		// Si no se ha alcanzado el pitch maximo entonces se anade el pitch
		if (hang_glider.GetMesh()->GetComponentRotation().Roll < max_pitch)
		{
			// Para solucionar el temblor de arriba. 
			// (Aunque cambie el limite de pitch por un cambio de roll no se podra subir mas)
			if (!is_at_up_limit)
			{
				// Se anade la rotacion hacia arriba
				hang_glider.GetMesh()->AddRelativeRotation(FRotator(0, 0, rotation_speed));

				// Se anade el offset de rotacion para que el ala delta pueda
				// calcular un nuevo limite de rotacion en funcion de esto
				add_pitch_offset(rotation_speed);
			}
		}
		else is_at_up_limit = true;
	}
	// Si el viento inclina el ala delta hacia arriba (para bajar)	
	else
	{
		FVector zlocked_forward_vector{ FVector(hang_glider.GetMesh()->GetRightVector().X, hang_glider.GetMesh()->GetRightVector().Y, 0) };
		zlocked_forward_vector.Normalize();

		// Para el pitch de abajo, el angulo por defecto (rotation.Roll) no lo devuelve bien. Asique se calcula manualmente
		const float actual_pitch
		{ Utility::angle_between(zlocked_forward_vector, hang_glider.GetMesh()->GetRightVector()) };
	
		const bool is_not_at_down_limit{ hang_glider.GetMesh()->GetComponentRotation().Roll > 0 || actual_pitch < max_pitch };

		if (is_not_at_down_limit)
		{
			// Se anade al rotacion hacia abajo
			hang_glider.GetMesh()->AddRelativeRotation(FRotator(0, 0, rotation_speed));

			// Se anade el offset de rotacion para que el ala delta pueda
			// calcular un nuevo limite de rotacion en funcion de esto
			add_pitch_offset(rotation_speed);
		}

		is_at_up_limit = false;
	}	
}

void UWind_component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}