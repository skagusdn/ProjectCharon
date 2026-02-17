// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "NativeGameplayTags.h"//
#include "CharonDamageExecution.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UCharonDamageExecution();

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
	
};
