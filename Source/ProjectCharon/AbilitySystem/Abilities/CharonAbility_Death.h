// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonGameplayAbility.h"
#include "CharonAbility_Death.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonAbility_Death : public UCharonGameplayAbility
{
	GENERATED_BODY()

public :
	UCharonAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Starts the death sequence.
	UFUNCTION(BlueprintCallable, Category = "Charon|Ability")
	void StartDeath();
	
	// Finishes the death sequence.
	UFUNCTION(BlueprintCallable, Category = "Charon|Ability")
	void FinishDeath();
};
