// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LifeStateComponent.h"
#include "VehicleLifeStateComponent.generated.h"


class AVehicle;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UVehicleLifeStateComponent : public ULifeStateComponent
{
	GENERATED_BODY()

public:
	
	UVehicleLifeStateComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnRegister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AVehicle> OwnerVehicle;

public:
	
};
