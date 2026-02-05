// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/CharonGameplayAbility.h"

#include "AbilitySystemComponent.h"

void UCharonGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UCharonGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// 만약 bRemoveAbilityOnEnd 플래그가 켜져 있다면, 어빌리티 스펙을 제거합니다.
	if (bRemoveAbilityOnEnd && HasAuthority(&GetCurrentActivationInfoRef()))
	{
		if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
		{
			// 아직 스펙이 유효한지 확인 후 제거
			if (ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle))
			{
				ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCharonGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo,
                                                       const FGameplayAbilitySpec& Spec) const
{
	
	const bool bIsPredicting = (Spec.Ability->GetCurrentActivationInfo().ActivationMode == EGameplayAbilityActivationMode::Predicting);

	// Try to activate if activation policy is on spawn.
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && (ActivationPolicy == ECharonAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		// ?? 이 함수는 서버에서밖에 호출안되던데 왜 이렇게 서버/클라 경우를 나눠놓았을까?????????????
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);
			
			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}
