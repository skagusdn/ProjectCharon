// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleBasicAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

void UVehicleBasicAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UVehicleBasicAttributeSet, VehicleHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVehicleBasicAttributeSet, VehicleMaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVehicleBasicAttributeSet, VehicleSpeed, COND_None, REPNOTIFY_Always);
}

void UVehicleBasicAttributeSet::OnRep_VehicleHP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVehicleBasicAttributeSet, VehicleHP, OldValue);
}

void UVehicleBasicAttributeSet::OnRep_VehicleMaxHP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVehicleBasicAttributeSet, VehicleMaxHP, OldValue);
}

void UVehicleBasicAttributeSet::OnRep_VehicleSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVehicleBasicAttributeSet, VehicleSpeed, OldValue);
}
