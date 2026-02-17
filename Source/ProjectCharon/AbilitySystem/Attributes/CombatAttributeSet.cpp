// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatAttributeSet.h"

#include "Net/UnrealNetwork.h"

UCombatAttributeSet::UCombatAttributeSet() : DamageAmount(0.0f), HealingAmount(0.0f)
{
}

void UCombatAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, DamageAmount, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, HealingAmount, COND_OwnerOnly, REPNOTIFY_Always);
}

void UCombatAttributeSet::OnRep_DamageAmount(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, DamageAmount, OldValue);
}

void UCombatAttributeSet::OnRep_HealingAmount(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, HealingAmount, OldValue);
}
