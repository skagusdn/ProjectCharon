// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CharonHealExecution.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonHealExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public : 
	UCharonHealExecution();
	
protected :
	
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
