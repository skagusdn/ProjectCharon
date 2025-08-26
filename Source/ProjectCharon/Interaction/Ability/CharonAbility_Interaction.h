// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CharonGameplayAbility.h"
#include "Interaction/InteractiveInterface.h"

#include "CharonAbility_Interaction.generated.h"

enum class EInteractResult : uint8;

/**
 *  상호작용 어빌리티!
 */
UCLASS()
class PROJECTCHARON_API UCharonAbility_Interaction : public UCharonGameplayAbility
{
	GENERATED_BODY()

public:

	UCharonAbility_Interaction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// UFUNCTION(BlueprintCallable, Category="Charon|Interaction")
	// FGameplayAbilitySpecHandle GrantAbilityAndActivate(TSubclassOf<UGameplayAbility> AbilityForInteraction,
	// 	AActor* InteractionObject, bool HasOptionalEventData = false, FGameplayEventData OptionalEventData = FGameplayEventData());
protected:
	UFUNCTION(BlueprintCallable, Category = "Charon|Interaction")
	void UpdateInteractionTarget(AActor* InInteractionTarget);
	UFUNCTION(BlueprintCallable, Category = "Charon|Interaction")
	EInteractResult TriggerInteraction();
	UFUNCTION(BlueprintCallable, Category = "Charon|Interaction")
	void PressInteractionInputOnceMore();

	//void Interact_GrantAbilityAndActivate();
	
	UPROPERTY(BlueprintReadOnly, Category = "Charon|Interaction")
	FInteractionDescription DescriptionData;

	UPROPERTY(BlueprintReadOnly, Category = "Charon|Interaction")
	FInteractionInfo TargetInteractionInfo;
	
	UPROPERTY(BlueprintReadOnly, Category = "Charon|Interaction")
	TObjectPtr<AActor> InteractionTarget;
};
