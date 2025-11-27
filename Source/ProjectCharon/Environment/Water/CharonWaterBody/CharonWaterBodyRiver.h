// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaterBodyRiverActor.h"
#include "CharonWaterBodyRiver.generated.h"

UCLASS()
class PROJECTCHARON_API ACharonWaterBodyRiver : public AWaterBodyRiver
{
	GENERATED_BODY()

public:
	
	

	UFUNCTION(BlueprintCallable)
	AWaterZone* GetOwningWaterZone();
protected:
	
	virtual void BeginPlay() override;

public:
	
};
