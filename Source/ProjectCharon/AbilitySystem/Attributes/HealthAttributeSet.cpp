// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_State_DamageImmunity, "Character.State.DamageImmunity");

UHealthAttributeSet::UHealthAttributeSet()
{
	bOutOfHealth = false;
	MaxHealthBeforeAttributeChange = 0.0f;
	HealthBeforeAttributeChange = 0.0f;
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Health, OldValue);

	const float CurrentHealth = GetHealth();
	const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
	
	OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);

	// ///////
	// if(GetOwningActor()){
	// 	if(GetOwningActor()-> HasAuthority())
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("HealthTest OnRep Server - Health Changed %f"), GetHealth());	///				
	// 	}else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("HealthTest OnRep Client - Health Changed %f"), GetHealth());	///
	// 	}
	// }
	// ///////
	
	if (!bOutOfHealth && CurrentHealth <= 0.0f)
	{
		// ///////
		// if(GetOwningActor()){
		// 	if(GetOwningActor()-> HasAuthority())
		// 	{
		// 		UE_LOG(LogTemp, Warning, TEXT("HealthTest OnRep Server - Out of Health First"));	///				
		// 	}else
		// 	{
		// 		UE_LOG(LogTemp, Warning, TEXT("HealthTest OnRep Client - Out of Health First"));	///
		// 	}
		// }
		// ///////
		OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
	}

	bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldValue);

	// Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxHealth() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxHealth());
}

bool UHealthAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	if(!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if(Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if(Data.EvaluatedData.Magnitude > 0.0f)
		{
			// 데미지 면역 상태일 경우.
			if(Data.Target.HasMatchingGameplayTag(TAG_Character_State_DamageImmunity))
			{
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}
		}
	}

	// Save the current health
	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();
	
	return true;
}

void UHealthAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if(Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if(Data.EvaluatedData.Magnitude > 0.0f)
		{
			OnDamageApplied.Broadcast(Data.EffectSpec.GetContext().GetInstigator(), Data.EffectSpec.GetContext().GetEffectCauser(),
				&Data.EffectSpec, Data.EvaluatedData.Magnitude, 0, Data.EvaluatedData.Magnitude);
			
			SetHealth(FMath::Clamp(GetHealth() - GetDamage(), 0.0f, GetMaxHealth()));
			SetDamage(0.0f);
			
			if(GetHealth() <= 0 && !bOutOfHealth)
			{
				// ///////
				// if(GetOwningActor()){
				// 	if(GetOwningActor()-> HasAuthority())
				// 	{
				// 		UE_LOG(LogTemp, Warning, TEXT("HealthTest Server - Out of Health First"));	///				
				// 	}else
				// 	{
				// 		UE_LOG(LogTemp, Warning, TEXT("HealthTest Client - Out of Health First"));	///
				// 	}
				// }
				// ///////
				
				OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
			}

			// ///////
			// if(GetOwningActor()){
			// 	if(GetOwningActor()-> HasAuthority())
			// 	{
			// 		UE_LOG(LogTemp, Warning, TEXT("HealthTest Server - Out of Health :%s"), bOutOfHealth ? TEXT("true") : TEXT("false") );			
			// 	}else
			// 	{
			// 		UE_LOG(LogTemp, Warning, TEXT("HealthTest Client - Out of Health :%s"), bOutOfHealth ? TEXT("true") : TEXT("false") );
			// 	}
			// }
			// ///////
		
			bOutOfHealth = (GetHealth() <= 0);
		}

		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), 0.0f, GetMaxHealth()));
		SetDamage(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		// Convert into +Health and then clamo
		SetHealth(FMath::Clamp(GetHealth() + GetHealing(), 0.0f, GetMaxHealth()));
		SetHealing(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp and fall into out of health handling below
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		// Notify on any requested max health changes
		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());
	}

	// If health has actually changed activate callbacks
	if (GetHealth() != HealthBeforeAttributeChange)
	{
		UE_LOG(LogTemp, Warning, TEXT("HealthTest - GetHealth : %f"), GetHealth());////
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	}

	// if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Health Test - Out of Health First"));	///	
	// 	OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	// }
	//
	// UE_LOG(LogTemp, Warning, TEXT("Health Test - Out of Health :%s"), bOutOfHealth ? TEXT("true") : TEXT("false") );	///
	//
	// // Check health again in case an event above changed it.
	// bOutOfHealth = (GetHealth() <= 0.0f);
	//
	
}

