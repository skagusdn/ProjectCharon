// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "AbilityTask_FindInteractables.generated.h"

class UCharonGameplayAbility;
class IInteractiveInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableFoundDelegate, AActor*, InteractionTarget);

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTCHARON_API UAbilityTask_FindInteractables : public UAbilityTask
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable)
	static UAbilityTask_FindInteractables* AbilityTask_FindInteractables(UCharonGameplayAbility* InteractionAbility, FCollisionProfileName TraceProfile, USceneComponent* TraceAim, float TraceRange, float TraceRadius, float TraceRate = 0.1f, bool ShowDebug = false );
	
	UPROPERTY(BlueprintAssignable, DisplayName = "On Interaction Target Changed")
	FInteractableFoundDelegate OnInteractionTargetChanged;

	UFUNCTION(BlueprintCallable)
	void PauseTrace();
	UFUNCTION(BlueprintCallable)
	void UnpauseTrace();

	// UFUNCTION(BlueprintCallable, meta=(DevelopmentOnly))
	// FString Test_CheckInteractionTarget();
	
protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	void PerformTrace();

	
	FCollisionProfileName TraceProfile;
	UPROPERTY()
	USceneComponent* TraceAim;
	float TraceRate;
	float TraceRange;
	FTimerHandle TraceTimerHandle;
	float TraceRadius;

	UPROPERTY()
	TObjectPtr<AActor> InteractionTarget = nullptr;
	
	bool bShowDebug = false;
	
	
};
