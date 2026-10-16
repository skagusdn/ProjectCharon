
#include "CharonAbilitySet.h"

void FCharonAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if(Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);	
	}
}

void FCharonAbilitySet_GrantedHandles::TakeFromAbilitySystem(UCharonAbilitySystemComponent* CharonASC)
{
	check(CharonASC);

	if (!CharonASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	// ClearAbility는 실행 중인 어빌리티를 EndAbility시키고, 그 안에서 이 함수가 다시 불릴 수 있다.
	// 핸들 배열을 먼저 비우고 사본으로 회수해서, 다시 불린 쪽은 빈 배열을 보고 끝나게 한다.
	const TArray<FGameplayAbilitySpecHandle> HandlesToClear = MoveTemp(AbilitySpecHandles);
	AbilitySpecHandles.Reset();

	for (const FGameplayAbilitySpecHandle& Handle : HandlesToClear)
	{
		if (Handle.IsValid())
		{
			CharonASC->ClearAbility(Handle);
		}
	}
}

UCharonAbilitySet::UCharonAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UCharonAbilitySet::GiveToAbilitySystem(UCharonAbilitySystemComponent* CharonASC,
	FCharonAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(CharonASC);

	if (!CharonASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	// Grant the gameplay abilities.
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FCharonAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		UGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = CharonASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}
}
