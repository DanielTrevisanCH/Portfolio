/** Copyright: @FreeWings
 *	Author: Daniel Trevisan Cohen-Henriquez
 *	Date: 21/04/2021
 */

#include "Score_Manager.h"


AScore_Manager::AScore_Manager()
{
	PrimaryActorTick.bCanEverTick = true;

}


void AScore_Manager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AScore_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AScore_Manager::add_score(int score)
{
	final_score = score;

	GEngine->AddOnScreenDebugMessage(-1, 30, FColor::Orange, "Your score is: " + FString::SanitizeFloat(final_score) + " points!");
}
