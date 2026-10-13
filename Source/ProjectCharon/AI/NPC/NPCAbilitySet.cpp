// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCAbilitySet.h"

#include "NPCAbility.h"

void FNPCAbilitySet_GrantedHandles::AddNPCAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		NPCAbilitySpecHandles.Add(Handle);
	}
}

void FNPCAbilitySet_GrantedHandles::TakeFromAbilitySystem(UCharonAbilitySystemComponent* CharonASC)
{
	check(CharonASC);

	// 일반 어빌리티 회수는 부모 구현 그대로 재사용.
	FCharonAbilitySet_GrantedHandles::TakeFromAbilitySystem(CharonASC);

	if (!CharonASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : NPCAbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			CharonASC->ClearAbility(Handle);
		}
	}

	NPCAbilitySpecHandles.Reset();
}

UNPCAbilitySet::UNPCAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNPCAbilitySet::GiveToAbilitySystem(UCharonAbilitySystemComponent* CharonASC,
	FNPCAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(CharonASC);

	if (!CharonASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	// 일반 어빌리티 부여 (부모 몫). OutGrantedHandles는 FCharonAbilitySet_GrantedHandles*로 업캐스트되어 전달됨.
	Super::GiveToAbilitySystem(CharonASC, OutGrantedHandles, SourceObject);

	for (int32 AbilityIndex = 0; AbilityIndex < GrantedNPCAbilities.Num(); ++AbilityIndex)
	{
		const FNPCAbilitySet_NPCAbility& AbilityToGrant = GrantedNPCAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedNPCAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		UGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;

		const FGameplayAbilitySpecHandle AbilitySpecHandle = CharonASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddNPCAbilitySpecHandle(AbilitySpecHandle);
		}
	}
}
