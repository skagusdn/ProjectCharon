// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CharonAttributeSet.h"
#include "CombatAttributeSet.generated.h"


/**
 *  전투 데미지 계산에 사용할 어트리뷰트 셋
 */
UCLASS()
class PROJECTCHARON_API UCombatAttributeSet : public UCharonAttributeSet
{
	GENERATED_BODY()
	
public :
	UCombatAttributeSet();

	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, DamageAmount);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, HealingAmount);
	
protected :

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_DamageAmount(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_HealingAmount(const FGameplayAttributeData& OldValue);
	
protected:
	
	/** Execution Calculation에 사용할 데미지 어트리뷰트 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageAmount)
	FGameplayAttributeData DamageAmount;

	/** Execution Calculation에 사용할 힐링 어트리뷰트 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealingAmount)
	FGameplayAttributeData HealingAmount;
	
};
