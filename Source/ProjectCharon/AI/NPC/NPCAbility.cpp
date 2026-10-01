// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCAbility.h"

#include "Logging.h"
#include "NPCCombatComponent.h"

UNPCAbility::UNPCAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

bool UNPCAbility::CanActivateAgainstTarget(AActor* Target, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{

	if (!CanActivateAbility(Handle, ActorInfo))
	{
		return false;
	}

	if (!IsTargetInRange(Target))
	{
		UE_LOGF(LogCharon, Verbose, "UNPCAbility::CanActivateAgainstTarget :: Target is not in Range");
		return false;
	}

	if (!IsExtraConditionMet(Target))
	{
		UE_LOGF(LogCharon, Verbose, "UNPCAbility::CanActivateAgainstTarget :: Extra Conditions not met");
		return false;
	}

	return true;
}

AActor* UNPCAbility::GetNPCTarget() const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	const UNPCCombatComponent* CombatComponent = Avatar ? Avatar->FindComponentByClass<UNPCCombatComponent>() : nullptr;
	return CombatComponent ? CombatComponent->GetCurrentTarget() : nullptr;
}

bool UNPCAbility::IsExtraConditionMet_Implementation(AActor* Target) const
{
	return true;
}

bool UNPCAbility::IsTargetInRange_Implementation(AActor* Target) const
{
	return true;
}
