// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CharonGameplayAbility.generated.h"




/**
 * ECharonAbilityActivationPolicy
 *
 *	Defines how an ability is meant to activate. Lyra꺼 긁어옴
 */
UENUM(BlueprintType)
enum class ECharonAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	// TODO : 아직 구현 안했음 이거. 했낭?
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned. (패시브 어빌리티)
	OnSpawn
};


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTCHARON_API UCharonGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable)
	ECharonAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; };
	
	
protected :
	//~UGameplayAbility interface
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	//~End of UGameplayAbility interface
	
	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	// OnSpawn인 어빌리티 발동용. (어빌리티 부여되자마자 발동하게). Lyra꺼 긁어옴
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ECharonAbilityActivationPolicy ActivationPolicy;

	
};
