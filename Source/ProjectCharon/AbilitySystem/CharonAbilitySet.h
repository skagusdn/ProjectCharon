#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "CharonAbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#include "GameplayAbilitySpecHandle.h"
#include "Abilities/CharonGameplayAbility.h"
#include "CharonAbilitySet.generated.h"

USTRUCT(BlueprintType)
struct FCharonAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;	
};

/**
 * FCharonAbilitySet_GrantedHandles
 *
 *	Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct FCharonAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public :
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);

	void TakeFromAbilitySystem(UCharonAbilitySystemComponent* CharonASC);

	explicit operator bool() const
	{
		return !AbilitySpecHandles.IsEmpty();
	}
protected:

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
};


UCLASS(BlueprintType, Const)
class UCharonAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UCharonAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.

	void GiveToAbilitySystem(UCharonAbilitySystemComponent* CharonASC, FCharonAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

protected:

	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FCharonAbilitySet_GameplayAbility> GrantedGameplayAbilities;
	//
	// // Gameplay effects to grant when this ability set is granted.
	// UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	// TArray<FLyraAbilitySet_GameplayEffect> GrantedGameplayEffects;
	//
	// // Attribute sets to grant when this ability set is granted.
	// UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	// TArray<FLyraAbilitySet_AttributeSet> GrantedAttributes;
};

