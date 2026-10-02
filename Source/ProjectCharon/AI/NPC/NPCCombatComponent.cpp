// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCCombatComponent.h"

#include "Logging.h"
#include "NPCAbility.h"


UNPCCombatComponent::UNPCCombatComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 어빌리티 선택은 AI가 요청할 때만 하므로 Tick 불필요.
	PrimaryComponentTick.bCanEverTick = false;
}

void UNPCCombatComponent::InitializeWithAbilitySystem(UCharonAbilitySystemComponent* InASC)
{
	check(InASC);

	if (AbilitySystemComponent)
	{
		UE_LOGF(LogCharon, Warning, "UNPCCombatComponent::InitializeWithAbilitySystem :: Already initialized");
		return;
	}

	AbilitySystemComponent = InASC;

	// 부여는 서버에서만. 스펙은 ASC가 복제해줌.
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	for (const UNPCAbilitySet* AbilitySet : AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(InASC, &GrantedAbilitySetHandles, GetOwner());
		}
	}
}

void UNPCCombatComponent::UninitializeFromAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		GrantedAbilitySetHandles.TakeFromAbilitySystem(AbilitySystemComponent);
	}

	CurrentTarget = nullptr;
	AbilitySystemComponent = nullptr;
}

void UNPCCombatComponent::SetCurrentTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
}

bool UNPCCombatComponent::CanUseAnyAbilityAgainstCurrentTarget() const
{
	AActor* Target = CurrentTarget.Get();
	return Target && FindUsableNPCAbility(Target).IsValid();
}

bool UNPCCombatComponent::TryUseAbilityAgainstCurrentTarget()
{
	if (!AbilitySystemComponent || !GetOwner()->HasAuthority())
	{
		return false;
	}

	// NPC 어빌리티는 한 번에 하나만.
	if (IsUsingNPCAbility())
	{
		return false;
	}

	AActor* Target = CurrentTarget.Get();
	if (!Target)
	{
		return false;
	}

	const FGameplayAbilitySpecHandle Handle = FindUsableNPCAbility(Target);
	if (!Handle.IsValid())
	{
		return false;
	}

	return AbilitySystemComponent->TryActivateAbility(Handle);
}

bool UNPCCombatComponent::IsUsingNPCAbility() const
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilitySetHandles.GetNPCAbilitySpecHandles())
	{
		const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (Spec && Spec->IsActive())
		{
			return true;
		}
	}

	return false;
}

FGameplayAbilitySpecHandle UNPCCombatComponent::FindUsableNPCAbility(AActor* Target) const
{
	if (!AbilitySystemComponent)
	{
		return FGameplayAbilitySpecHandle();
	}

	const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilitySetHandles.GetNPCAbilitySpecHandles())
	{
		const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (!Spec)
		{
			continue;
		}

		// CDO로 검사. 사거리/조건 판정은 인스턴스 상태에 의존하지 않도록 작성할 것.
		const UNPCAbility* NPCAbilityCDO = CastChecked<UNPCAbility>(Spec->Ability);
		if (NPCAbilityCDO->CanActivateAgainstTarget(Target, Handle, ActorInfo))
		{
			return Handle;
		}
	}

	return FGameplayAbilitySpecHandle();
}
