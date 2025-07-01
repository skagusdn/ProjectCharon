// Fill out your copyright notice in the Description page of Project Settings.


#include "TestObserveStamina.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RunAttributeSet.h"

void UTestObserveStamina::StartStaminaObserving(UAbilitySystemComponent* TargetASC)
{
	check(TargetASC);

	TargetASC->GetGameplayAttributeValueChangeDelegate(URunAttributeSet::GetStaminaAttribute()).AddUObject(this, &ThisClass::HandleTargetStaminaChanged);
	
}

void UTestObserveStamina::HandleTargetStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnTargetStaminaChanged.Broadcast(ChangeData.OldValue, ChangeData.NewValue);
}
