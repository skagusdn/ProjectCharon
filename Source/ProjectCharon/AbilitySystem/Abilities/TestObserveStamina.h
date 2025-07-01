// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonGameplayAbility.h"
#include "TestObserveStamina.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTestObserveStaminaDelegate, float, OldValue, float, NewValue);

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UTestObserveStamina : public UCharonGameplayAbility
{
	GENERATED_BODY()


public:

	
	//virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	UFUNCTION(BlueprintCallable)
	void StartStaminaObserving(UAbilitySystemComponent* TargetASC);
	void HandleTargetStaminaChanged(const FOnAttributeChangeData& ChangeData);

	UPROPERTY(BlueprintAssignable)
	FTestObserveStaminaDelegate OnTargetStaminaChanged;
	
};
