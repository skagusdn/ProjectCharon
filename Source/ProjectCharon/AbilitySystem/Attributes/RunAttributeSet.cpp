// Fill out your copyright notice in the Description page of Project Settings.


#include "RunAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

//void URunAttributeSet::OnRunSpeedChanged(const FOnAttributeChangeData& ChangeData);

URunAttributeSet::URunAttributeSet()
{
	//InitStamina(777.f);
}

void URunAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URunAttributeSet, RunSpeed, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URunAttributeSet, Stamina, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URunAttributeSet, MaxStamina, COND_OwnerOnly, REPNOTIFY_Always);

}

void URunAttributeSet::OnRep_RunSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URunAttributeSet, RunSpeed, OldValue);

	const float CurrentRunSpeed = GetRunSpeed();
	const float ChangedAmount = CurrentRunSpeed - OldValue.GetCurrentValue();

	OnRunSpeedChanged.Broadcast(nullptr, nullptr, nullptr, ChangedAmount, OldValue.GetCurrentValue(), CurrentRunSpeed);
}

void URunAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URunAttributeSet, Stamina, OldValue);
	
}

void URunAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URunAttributeSet, MaxStamina, OldValue);
}

void URunAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if(Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
}

// void URunAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
// {
// 	Super::PostGameplayEffectExecute(Data);
//
// 	if(Data.EvaluatedData.Attribute == GetStaminaAttribute())
// 	{
// 		SetStamina(FMath::Clamp(GetStamina(), 0.f, 1000.f));
// 	}
// }

//

void URunAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	UE_LOG(LogTemp, Warning, TEXT("%s : PostAttributeChange From %f To %f"), *Attribute.GetName(), OldValue, NewValue);
	
}

void URunAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
}

void URunAttributeSet::SetStamina(const float NewVal)
{
	Stamina.SetCurrentValue(FMath::Clamp(NewVal, 0, GetMaxStamina()));
}


