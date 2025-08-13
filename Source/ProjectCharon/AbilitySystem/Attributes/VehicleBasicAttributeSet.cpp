// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleBasicAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_VehicleState_DamageImmunity, "Vehicle.State.DamageImmunity");

UVehicleBasicAttributeSet::UVehicleBasicAttributeSet()
{
}



void UVehicleBasicAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UVehicleBasicAttributeSet, VehicleHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVehicleBasicAttributeSet, VehicleMaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVehicleBasicAttributeSet, VehicleSpeed, COND_None, REPNOTIFY_Always);
	//DOREPLIFETIME_CONDITION_NOTIFY(UVehicleBasicAttributeSet, VehicleDamage, COND_None, REPNOTIFY_Always);
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

// void UVehicleBasicAttributeSet::OnRep_VehicleDamage(const FGameplayAttributeData& OldValue)
// {
// 	GAMEPLAYATTRIBUTE_REPNOTIFY(UVehicleBasicAttributeSet, VehicleDamage, OldValue);
// }

bool UVehicleBasicAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	if(!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if(Data.EvaluatedData.Attribute == GetVehicleDamageAttribute())
	{
		if(Data.EvaluatedData.Magnitude > 0.0f)
		{
			// 데미지 면역 상태일 경우.
			if(Data.Target.HasMatchingGameplayTag(TAG_VehicleState_DamageImmunity))
			{
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}
		}
	}
	
	return true;
}

void UVehicleBasicAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute == GetVehicleDamageAttribute())
	{
		if(Data.EvaluatedData.Magnitude > 0.0f)
		{
			OnVehicleDamageApplied.Broadcast(Data.EffectSpec.GetContext().GetInstigator(), Data.EffectSpec.GetContext().GetEffectCauser(),
				&Data.EffectSpec, Data.EvaluatedData.Magnitude, 0, Data.EvaluatedData.Magnitude);

			SetVehicleHP(FMath::Clamp(GetVehicleHP() - GetVehicleDamage(), 0.0f, GetVehicleMaxHP()));
			SetVehicleDamage(0.0f);
		}
		
	}
}
