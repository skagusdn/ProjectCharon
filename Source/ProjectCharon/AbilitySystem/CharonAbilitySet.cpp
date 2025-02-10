
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

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			CharonASC->ClearAbility(Handle);
		}
	}
	
	AbilitySpecHandles.Reset();
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
		AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = CharonASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}
}
