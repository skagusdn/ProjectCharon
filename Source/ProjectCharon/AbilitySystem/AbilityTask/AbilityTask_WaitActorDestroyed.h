// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitActorDestroyed.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitActorDestroyedDelegate);

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UAbilityTask_WaitActorDestroyed : public UAbilityTask
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitActorDestroyed* WaitActorDestroyed(UGameplayAbility* OwningAbility, AActor* TargetActor);
	
	virtual void Activate() override;
	
	UPROPERTY(BlueprintAssignable)
	FWaitActorDestroyedDelegate OnTargetActorDestroyed;
protected:

	UFUNCTION()
	void HandleTargetActorDestroyed(AActor* DestroyedActor);
	
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;
	
};
