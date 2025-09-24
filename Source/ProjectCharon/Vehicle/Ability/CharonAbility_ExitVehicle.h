// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CharonGameplayAbility.h"
#include "CharonAbility_ExitVehicle.generated.h"

class AVehicle;
/**
 *  탈것에서 내릴 때 사용하는 어빌리티, 의 베이스.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTCHARON_API UCharonAbility_ExitVehicle : public UCharonGameplayAbility
{
	GENERATED_BODY()

public:
	UCharonAbility_ExitVehicle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UFUNCTION(BlueprintCallable)
	bool TryExitVehicle(AVehicle* VehicleToExit, ACharacter* InRider);
};
