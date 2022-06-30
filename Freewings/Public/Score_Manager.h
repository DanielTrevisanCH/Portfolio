/** Copyright: @FreeWings
 *	Author: Daniel Trevisan Cohen-Henriquez
 *	Date: 21/04/2021
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Score_Manager.generated.h"

UCLASS()
class FREEWINGS_API AScore_Manager : public AActor
{
	GENERATED_BODY()
	
public:	

	AScore_Manager();

	void add_score(int score); 

	int final_score;

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

};
