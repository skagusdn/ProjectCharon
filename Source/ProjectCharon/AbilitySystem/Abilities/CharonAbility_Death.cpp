// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonAbility_Death.h"

#include "CharonGameplayTags.h"
#include "Character/CharonCharacter.h"
#include "Character/LifeStateComponent.h"
#include "Framework/CharonGameMode.h"

UCharonAbility_Death::UCharonAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CharonGameplayTags::GameplayEvent_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UCharonAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Lyra GA_Death 참고중
	// 나중에 죽음 면역 같은거 넣을 꺼면 이부분에서 체크하기.
	// TODO : 죽음 면역이 아닌 모든 어빌리티 캔슬

	SetCanBeCanceled(false);
	StartDeath();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UCharonAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	FinishDeath();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCharonAbility_Death::StartDeath()
{
	if(const AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if(ULifeStateComponent* LifeStateComponent = AvatarActor->FindComponentByClass<ULifeStateComponent>())
		{
			if(LifeStateComponent->GetLifeState() == ECharonLifeState::NotDead)
			{
				LifeStateComponent->TryStartDeath();	
			}
		}
	}	
}

void UCharonAbility_Death::FinishDeath()
{
	if(const AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		APlayerController* Player = GetActorInfo().PlayerController.Get();
		
		if(ULifeStateComponent* LifeStateComponent = AvatarActor->FindComponentByClass<ULifeStateComponent>())
		{
			if(LifeStateComponent->GetLifeState() == ECharonLifeState::DeathStarted)
			{
				LifeStateComponent->TryFinishDeath();	
			}
		}

		// if(GetWorld() && GetWorld()->GetAuthGameMode() && Player)
		// {
		// 	if(ACharonGameMode* CharonGameMode = Cast<ACharonGameMode>(GetWorld()->GetAuthGameMode()))
		// 	{
		// 		CharonGameMode->ReportDeath(Player);
		// 	}			
		// }
	}

	
}

