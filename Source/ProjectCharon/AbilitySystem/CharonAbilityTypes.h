#pragma once

#include "CharonAbilityTypes.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FAbilityCommitInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGameplayAbility> Ability = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float CooldownDuration = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float RemainingCooldown = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityCommitDelegate, FAbilityCommitInfo, CommittedAbility);