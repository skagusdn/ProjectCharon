// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaterBodyLakeActor.h"
#include "CharonWaterBodyLake.generated.h"

UCLASS()
class PROJECTCHARON_API ACharonWaterBodyLake : public AWaterBodyLake
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	AWaterZone* GetOwningWaterZone();

protected:
	

public:
	
};
