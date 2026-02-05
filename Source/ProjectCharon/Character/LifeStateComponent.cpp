// Fill out your copyright notice in the Description page of Project Settings.


#include "LifeStateComponent.h"

#include "CharonGameplayTags.h"
#include "Logging.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/HealthAttributeSet.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
ULifeStateComponent::ULifeStateComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	AbilitySystemComponent = nullptr;
	LifeState = ECharonLifeState::NotDead;

	// //
	// UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent HealthTest Constructor In Owner - %s // ASC :: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));
	// //
}



void ULifeStateComponent::StartDeath()
{
	if(LifeState != ECharonLifeState::NotDead)
	{
		return;
	}

	LifeState = ECharonLifeState::DeathStarted;

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);
	Owner->ForceNetUpdate();
}

void ULifeStateComponent::FinishDeath()
{
	if(LifeState != ECharonLifeState::DeathStarted)
	{
		return;
	}

	LifeState = ECharonLifeState::DeathFinished;

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);
	Owner->ForceNetUpdate();
}

void ULifeStateComponent::OnRep_LifeState(ECharonLifeState OldLifeState)
{
	// 실제 State 변경은 다른 함수에서. -> Lyra 로직.
	ECharonLifeState NewLifeState = LifeState;
	LifeState = OldLifeState;

	if(OldLifeState > NewLifeState)
	{
		// 부활이 되버린 경우. 서버에서 리플리케이션된 값을 받았을 때.
		return;
	}
	
	if(OldLifeState == ECharonLifeState::NotDead)
	{
		if(NewLifeState == ECharonLifeState::DeathStarted)
		{
			StartDeath();
		}
		else if(NewLifeState == ECharonLifeState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
	}
	else if(OldLifeState == ECharonLifeState::DeathStarted)
	{
		if(NewLifeState == ECharonLifeState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
	}

	ensureMsgf((LifeState == NewLifeState), TEXT("LifeStateComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldLifeState, (uint8)NewLifeState, *GetNameSafe(GetOwner()));
	return;
}

void ULifeStateComponent::OnRep_AbilitySystemComponent(UCharonAbilitySystemComponent* OldASC)
{
	// //
	// UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent HealthTest OnRep_AbilitySystemComponent In Owner - %s  // ASC :: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));
	// //
	
	if(!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent  OnRep_AbilitySystemComponent: ASC Removed for owner [%s]."), *GetNameSafe(GetOwner()));
		return;
	}
	HealthSet = AbilitySystemComponent->GetSet<UHealthAttributeSet>();
	if(!HealthSet)
	{
		UE_LOG(LogCharon, Error, TEXT("ULifeStateComponent  OnRep_AbilitySystemComponent: Cannot initialize Life State component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(GetOwner()));
		return;
	}

	// Register to listen for attribute changes.
	HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);
	
	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
}

void ULifeStateComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser,
                                              const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	// ///////
	// if(GetOwner()){
	// 	if(GetOwner()-> HasAuthority())
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("LifeStateComponent HealthTest HandleHealthChanged Server - Health Changed %f"), NewValue);	///				
	// 	}else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("LifeStateComponent HealthTest HandleHealthChanged Client - Health Changed %f"), NewValue);	///	
	// 	}
	// }
	// ///////
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void ULifeStateComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void ULifeStateComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	// /////////////
	// if(GetOwner()->HasAuthority())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent HealthTest OutOfHealth - Server"));	
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent HealthTest OutOfHealth - Client"));
	// }
	// /////////
	
	
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// 라이라에서 가져온거. Payload 속 옵셔널 오브젝트나 태그에 뭐가 들어가는진 잘 모르겠다.
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = CharonGameplayTags::GameplayEvent_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}

#endif
}

void ULifeStateComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	
	Super::OnUnregister();
}

void ULifeStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULifeStateComponent, AbilitySystemComponent);
	DOREPLIFETIME(ULifeStateComponent, LifeState);
}

void ULifeStateComponent::InitializeWithAbilitySystem(UCharonAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	// //
	// UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent HealthTest InitializeWithAbilitySystem Start In Owner - %s // ASC :: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));
	// //
	
	if(AbilitySystemComponent)
	{
		UE_LOG(LogCharon, Error, TEXT("LifeStateComponent : Life State component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	// //
	// UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent HealthTest InitializeWithAbilitySystem Setting to InASC - %s"), *GetNameSafe(InASC));
	// //
	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogCharon, Error, TEXT("LifeStateComponent : Cannot initialize Life State component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HealthSet = AbilitySystemComponent->GetSet<UHealthAttributeSet>();
	if(!HealthSet)
	{
		UE_LOG(LogCharon, Error, TEXT("LifeStateComponent : Cannot initialize Life State component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);
	
	// FDelegateHandle TestOnHealthChangedHandle =  HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	// FDelegateHandle TestOnMaxHealthChangedHandle =  HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	// FDelegateHandle TestOnOutOfHealthHandle =  HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	// //////
	// if(GetOwner()->HasAuthority())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Vehicle LifeStateComponent Test - Server : Handles -> %s %s %s "),
	// 		TestOnHealthChangedHandle.IsValid()? TEXT("O ") : TEXT("X "),
	// 		TestOnMaxHealthChangedHandle.IsValid()? TEXT("O ") : TEXT("X "),
	// 		TestOnOutOfHealthHandle.IsValid()? TEXT("O ") : TEXT("X "));
	// }else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Vehicle LifeStateComponent Test - Client : Handles -> %s %s %s "),
	// 		TestOnHealthChangedHandle.IsValid()? TEXT("O ") : TEXT("X "),
	// 		TestOnMaxHealthChangedHandle.IsValid()? TEXT("O ") : TEXT("X "),
	// 		TestOnOutOfHealthHandle.IsValid()? TEXT("O ") : TEXT("X "));
	// }
	// //////
	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);


	// //
	// UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent HealthTest InitializeWithAbilitySystem Complete In Owner - %s "), *Owner->GetName());
	// //
}

void ULifeStateComponent::UninitializeFromAbilitySystem()
{
	// 아직은 딱히 뭘 더 할게 없네. 
	AbilitySystemComponent = nullptr;
	// //
	// UE_LOG(LogTemp, Warning, TEXT("ULifeStateComponent HealthTest UninitializeFromAbilitySystem Start In Owner - %s // ASC :: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));
	// //
}

void ULifeStateComponent::TryStartDeath()
{
	// 뭔가 데스 체크 로직을 넣을꺼면 여기서.
	//

	StartDeath();
}

void ULifeStateComponent::TryFinishDeath()
{
	// 여긴 뭐 체크할게 있나? 근데 FinishDeath를 Public으로 하긴 좀 그렇다

	FinishDeath();
}

