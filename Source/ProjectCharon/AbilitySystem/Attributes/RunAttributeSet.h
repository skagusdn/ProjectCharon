// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CharonAttributeSet.h"
#include "RunAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API URunAttributeSet : public UCharonAttributeSet
{
	GENERATED_BODY()

public: 

	URunAttributeSet();


protected:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_RunSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	//
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	//

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RunSpeed)
	FGameplayAttributeData RunSpeed;

public:


	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(URunAttributeSet, Stamina);
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Stamina);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(Stamina);
	void SetStamina(float NewVal);

	//ATTRIBUTE_ACCESORS(URunAttributeSet, Stamina);
	ATTRIBUTE_ACCESSORS(URunAttributeSet, MaxStamina);
	ATTRIBUTE_ACCESSORS(URunAttributeSet, RunSpeed);

	//UPROPERTY(BlueprintAssignable)
	mutable FCharonAttributeEvent OnRunSpeedChanged;
	//mutable FCharonAttributeEvent OnStaminaChanged;


	//virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	

	
};
