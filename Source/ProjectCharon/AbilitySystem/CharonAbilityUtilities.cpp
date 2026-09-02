// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonAbilityUtilities.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"

bool UCharonAbilityUtilities::CanCommitAbilityByClass(UAbilitySystemComponent* TargetASC,
                                                      TSubclassOf<UGameplayAbility> AbilityClass, FGameplayAbilitySpecHandle& OutFoundSpecHandle)
{
	// 제미나이로 생성. 
	
	OutFoundSpecHandle = FGameplayAbilitySpecHandle();

	// 1. 유효성 검사
	if (!TargetASC || !AbilityClass)
	{
		return false;
	}

	// 2. ASC에 부여된 ActivatableAbilities 목록 중 해당 클래스와 일치하는 Spec 찾기
	FGameplayAbilitySpec* FoundSpec = nullptr;
	for (FGameplayAbilitySpec& Spec : TargetASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
		{
			FoundSpec = &Spec;
			OutFoundSpecHandle = Spec.Handle;
			break;
		}
	}

	// 어빌리티가 부여되어 있지 않음
	if (!FoundSpec)
	{
		return false;
	}

	// 3. 인스턴스 또는 CDO를 얻어와 CanCommitAbility 검사
	// (InstancedPerActor, InstancedPerExecution, Non-Instanced 모두 대응)
	UGameplayAbility* PrimaryInstance = FoundSpec->GetPrimaryInstance();
	UGameplayAbility* AbilityToCheck = PrimaryInstance ? PrimaryInstance : FoundSpec->Ability.Get();

	if (!AbilityToCheck)
	{
		return false;
	}

	// 4. CanCommitAbility 호출
	// Cooldown 태그, Cost(마나/스태미나 등 어트리뷰트 잔여량) 충족 여부를 내부적으로 검사합니다.
	const FGameplayAbilityActorInfo* ActorInfo = TargetASC->AbilityActorInfo.Get();
	if (!ActorInfo)
	{
		return false;
	}
	
	return AbilityToCheck->CommitCheck(FoundSpec->Handle, ActorInfo, FoundSpec->ActivationInfo);
}

FAbilityCooldownInfo UCharonAbilityUtilities::CheckAbilityStatusAndCooldown(UAbilitySystemComponent* TargetASC,
	TSubclassOf<UGameplayAbility> TargetAbilityClass)
{
	// 제미나이로 생성. 
	
	FAbilityCooldownInfo ResultInfo;

    if (!TargetASC || !TargetAbilityClass)
    {
        return ResultInfo;
    }

    // 1. Target ASC에서 Spec 탐색
    FGameplayAbilitySpec* FoundSpec = TargetASC->FindAbilitySpecFromClass(TargetAbilityClass);
    if (!FoundSpec)
    {
        return ResultInfo;
    }
	
	ResultInfo.bHasAbility = true;

    const FGameplayAbilityActorInfo* ActorInfo = TargetASC->AbilityActorInfo.Get();
    if (!ActorInfo)
    {
        return ResultInfo;
    }

    // 2. 인스턴스 또는 CDO 가져오기 (모든 InstancingPolicy 대응)
    UGameplayAbility* PrimaryInstance = FoundSpec->GetPrimaryInstance();
    UGameplayAbility* AbilityToCheck = PrimaryInstance ? PrimaryInstance : FoundSpec->Ability.Get();

    if (!AbilityToCheck)
    {
        return ResultInfo;
    }
	
	
    // 3. 발동 가능 여부 검사 (CanActivateAbility)
    FGameplayTagContainer FailureTags;
    ResultInfo.bCanActivate = AbilityToCheck->CanActivateAbility(FoundSpec->Handle, ActorInfo, nullptr, nullptr, &FailureTags);
	
    // 4. 남아있는 쿨다운 시간 계산
	const FGameplayTagContainer* CooldownTags = AbilityToCheck->GetCooldownTags();
    if (CooldownTags && !CooldownTags->IsEmpty())
    {
        // 쿨다운 태그를 가진 지속형 GameplayEffect 쿼리
        FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
    	
        TArray<TPair<float, float>> DurationAndTimeRemaining = TargetASC->GetActiveEffectsTimeRemainingAndDuration(Query);
        if (DurationAndTimeRemaining.Num() > 0)
        {
            // 가장 긴 남은 시간을 기준으로 계산
            float MaxTimeRemaining = 0.0f;
            float MatchingDuration = 0.0f;
        	
            for (const TPair<float, float>& Entry : DurationAndTimeRemaining)
            {
                if (Entry.Key > MaxTimeRemaining) // Key = TimeRemaining
                {
                    MaxTimeRemaining = Entry.Key;
                    MatchingDuration = Entry.Value; // Value = TotalDuration
                }
            }

            if (MaxTimeRemaining > 0.0f)
            {
                ResultInfo.bIsOnCooldown = true;
                ResultInfo.TimeRemaining = MaxTimeRemaining;
                ResultInfo.TotalDuration = MatchingDuration;
            }
        }
    }
	

    return ResultInfo;
}

FGameplayTagContainer UCharonAbilityUtilities::GetCooldownTagsFromAbilityClass(
	TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (AbilityClass)
	{
		const UGameplayAbility* CDO = AbilityClass->GetDefaultObject<UGameplayAbility>();
		if (CDO)
		{
			if (const FGameplayTagContainer* CooldownTagsPtr = CDO->GetCooldownTags())
			{
				return *CooldownTagsPtr;
			}
		}
	}
	
	return FGameplayTagContainer();
}

FGameplayTagContainer UCharonAbilityUtilities::GetCooldownTagsFromAbilityInstance(UGameplayAbility* Ability)
{
	if (Ability)
	{
		if (const FGameplayTagContainer* CooldownTagsPtr = Ability->GetCooldownTags())
		{
			return *CooldownTagsPtr;
		}
	}
	return FGameplayTagContainer();
}
