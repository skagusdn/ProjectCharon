// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuoyancyComponent.h"
#include "CharonBuoyancyComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Config = Game, meta = (BlueprintSpawnableComponent))
class PROJECTCHARON_API UCharonBuoyancyComponent : public UBuoyancyComponent
{
	GENERATED_BODY()
	

public :
	
	UFUNCTION(BlueprintCallable)
	void AddPontoons(float Radius, TArray<FVector> Locations);
	
	UFUNCTION(BlueprintCallable)
	TArray<FVector> CalculateBallsLocation(FVector UpLeftFront, FVector DownRightBack, int32 Radius);

	UFUNCTION(BlueprintCallable)
	void InitPontoons(FVector UpLeftFront, FVector DownRightBack, int32 Radius);

	
};
