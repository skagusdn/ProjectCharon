// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CharonAbilitySystemComponent.h"

#include "Abilities/CharonGameplayAbility.h"
#include "Engine/SpecularProfile.h"

UCharonAbilitySystemComponent::UCharonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UCharonAbilitySystemComponent::TestGiveAbility(TSubclassOf<UGameplayAbility> AbilityClass,
                                                    FGameplayTag InputTag)
{
	if(FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(K2_GiveAbility(AbilityClass, 0, -1)))
	{
		Spec->DynamicAbilityTags.AddTag(InputTag);
	}
}

void UCharonAbilitySystemComponent::AbilityLocalInputTagPressed(FGameplayTag InputTag)
{
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			if (Spec.Ability)
			{
				Spec.InputPressed = true;
				if (Spec.IsActive())
				{
					if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
					{
						ServerSetInputPressed(Spec.Handle);
					}

					AbilitySpecInputPressed(Spec);

					// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());					
				}
				else
				{
					// Ability is not active, so try to activate it
					TryActivateAbility(Spec.Handle);
				}
			}
		}
	}
}

void UCharonAbilitySystemComponent::AbilityLocalInputTagReleased(FGameplayTag InputTag)
{
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			Spec.InputPressed = false;
			if (Spec.Ability && Spec.IsActive())
			{
				if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
				{
					ServerSetInputReleased(Spec.Handle);
				}

				AbilitySpecInputReleased(Spec);

				// 누르고 있을때만 발동하는 어빌리티는 떼면 취소하기.
				if(UCharonGameplayAbility* CharonAbilityCDO = Cast<UCharonGameplayAbility>(Spec.Ability))
				{
					if(CharonAbilityCDO->GetActivationPolicy() == ECharonAbilityActivationPolicy::WhileInputActive)
					{
						CancelAbilityHandle(Spec.Handle);
					}
				}
				
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}
