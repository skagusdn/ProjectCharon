// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CharonAttributeSet.h"
#include "NativeGameplayTags.h"
#include "HealthAttributeSet.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_State_DamageImmunity);

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UHealthAttributeSet : public UCharonAttributeSet
{
	GENERATED_BODY()

public :
	UHealthAttributeSet();

	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Damage);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Healing);
	
	mutable FCharonAttributeEvent OnDamageApplied;

	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FCharonAttributeEvent OnHealthChanged;
	// Delegate when max health changes
	mutable FCharonAttributeEvent OnMaxHealthChanged;
	// Delegate to broadcast when the health attribute reaches zero
	mutable FCharonAttributeEvent OnOutOfHealth;
	
protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	

	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	//virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	//virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
	
protected:
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Charon|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData Health;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Charon|Health", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData MaxHealth;
	UPROPERTY(BlueprintReadOnly, Category = "Charon|Health", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
	UPROPERTY(BlueprintReadOnly, Category = "Charon|Health", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Store the health before any changes 
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;
	
	bool bOutOfHealth;
};
