// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonGameplayAbility.h"
#include "CharonAbility_Death.generated.h"

USTRUCT(BlueprintType)
struct FDeathMessage
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> DeadActor;

	//TODO : 나중에 죽음 원인 정보 추가.
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Killer;
};

/**
 *  Death 어빌리티. LifeStateComponent와 연계.
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
