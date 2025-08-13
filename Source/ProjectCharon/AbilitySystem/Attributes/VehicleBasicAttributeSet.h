// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CharonAttributeSet.h"
#include "NativeGameplayTags.h"
#include "VehicleBasicAttributeSet.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_VehicleState_DamageImmunity);

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UVehicleBasicAttributeSet : public UCharonAttributeSet
{
	GENERATED_BODY()

public :
	UVehicleBasicAttributeSet();

	ATTRIBUTE_ACCESSORS(UVehicleBasicAttributeSet, VehicleHP);
	ATTRIBUTE_ACCESSORS(UVehicleBasicAttributeSet, VehicleMaxHP);
	ATTRIBUTE_ACCESSORS(UVehicleBasicAttributeSet, VehicleSpeed);
	ATTRIBUTE_ACCESSORS(UVehicleBasicAttributeSet, VehicleDamage);

	mutable FCharonAttributeEvent OnVehicleDamageApplied;
	
protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_VehicleHP(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_VehicleMaxHP(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_VehicleSpeed(const FGameplayAttributeData& OldValue);
	// UFUNCTION()
	// void OnRep_VehicleDamage(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VehicleHP, Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData VehicleHP;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VehicleMaxHP, Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData VehicleMaxHP;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VehicleSpeed, Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData VehicleSpeed;
	UPROPERTY(BlueprintReadOnly, Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData VehicleDamage;
	
};
