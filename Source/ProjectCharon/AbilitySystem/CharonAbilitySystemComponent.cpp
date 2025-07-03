// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CharonAbilitySystemComponent.h"

#include "Abilities/CharonGameplayAbility.h"
#include "Engine/SpecularProfile.h"
#include "ProjectCharon.h"

UCharonAbilitySystemComponent::UCharonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}


void UCharonAbilitySystemComponent::K2_BindEventOnAttributeChange(UObject* EventSource, FGameplayAttribute AttributeToBind,
                                                               FOnCharonAttributeChanged_Dynamic Event)
{
	FGameplayAttribute FoundAttribute = FindAttribute(AttributeToBind).Get(nullptr);
	
	if(FoundAttribute == nullptr)
	{
		UE_LOG(LogCharon, Error, TEXT("Attribute not found"));
		return;
	}
	
	FDelegateHandle Handle = GetGameplayAttributeValueChangeDelegate(FoundAttribute).
	AddWeakLambda(EventSource, [Event](const FOnAttributeChangeData& Data) ->
	void{
		Event.Execute(Data.OldValue, Data.NewValue); 
	});
	
	if(!Handle.IsValid())
	{
		UE_LOG(LogCharon, Error, TEXT("Delegate Binding Failed"));
		return;
	}
	
	AttributeChangeBindHandles_Dynamic.Add(Event, {Handle, FoundAttribute});
}

void UCharonAbilitySystemComponent::K2_UnBindEventOnAttributeChange(FOnCharonAttributeChanged_Dynamic Event)
{
	if(AttributeChangeBindHandles_Dynamic.Contains(Event))
	{
		const FGameplayAttribute Attribute = AttributeChangeBindHandles_Dynamic.Find(Event)->Value;
		const FDelegateHandle Handle = AttributeChangeBindHandles_Dynamic.Find(Event)->Key;
		
		GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);
		AttributeChangeBindHandles_Dynamic.Remove(Event);
	}
}

void UCharonAbilitySystemComponent::BindEventOnAttributeChange(FGameplayAttribute AttributeToBind,
	FOnCharonAttributeChanged Event)
{
	FGameplayAttribute FoundAttribute = FindAttribute(AttributeToBind).Get(nullptr);
	
	if(FoundAttribute == nullptr)
	{
		UE_LOG(LogCharon, Error, TEXT("Attribute not found"));
		return;
	}
	
	FDelegateHandle Handle = GetGameplayAttributeValueChangeDelegate(FoundAttribute).
	AddLambda([Event](const FOnAttributeChangeData& Data) ->
	void{
		Event.Broadcast(Data.OldValue, Data.NewValue); 
	});
	
	if(!Handle.IsValid())
	{
		UE_LOG(LogCharon, Error, TEXT("Delegate Binding Failed"));
		return;
	}
	
	AttributeChangeBindHandles.Add({Handle, FoundAttribute});
}

void UCharonAbilitySystemComponent::UnBindEventOnAttributeChange(FDelegateHandle EventHandle)
{
	if(AttributeChangeBindHandles.Contains(EventHandle))
	{
		const FGameplayAttribute * Attribute = AttributeChangeBindHandles.Find(EventHandle);
		GetGameplayAttributeValueChangeDelegate(*Attribute).Remove(EventHandle);

		AttributeChangeBindHandles.Remove(EventHandle);
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

TOptional<FGameplayAttribute> UCharonAbilitySystemComponent::FindAttribute(const FGameplayAttribute& AttributeToFind)
{
	TOptional<FGameplayAttribute> Ret;
	
	TArray<FGameplayAttribute> AllAttributes;
	GetAllAttributes(AllAttributes);
	
	for(FGameplayAttribute Attribute : AllAttributes)
	{
		if(Attribute.AttributeName.Equals(AttributeToFind.AttributeName))
		{
			Ret.Emplace(Attribute);
		}
	}
	
	return Ret;
}
