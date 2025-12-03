// Fill out your copyright notice in the Description page of Project Settings.


#include "LifeStateComponent.h"

#include "Logging.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
ULifeStateComponent::ULifeStateComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//PrimaryComponentTick.bCanEverTick = true;
	AbilitySystemComponent = nullptr;
	LifeState = ECharonLifeState::NotDead;
	
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

void ULifeStateComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	
	Super::OnUnregister();
}

void ULifeStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULifeStateComponent, LifeState);
}

void ULifeStateComponent::InitializeWithAbilitySystem(UCharonAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if(AbilitySystemComponent)
	{
		UE_LOG(LogCharon, Error, TEXT("LifeStateComponent: Life State component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogCharon, Error, TEXT("LifeStateComponent: Cannot initialize Life State component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	// 나중에 체력 AttributeSet 설정 추가. 
}

void ULifeStateComponent::UninitializeFromAbilitySystem()
{
	// 아직은 딱히 뭘 더 할게 없네. 
	AbilitySystemComponent = nullptr;
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

