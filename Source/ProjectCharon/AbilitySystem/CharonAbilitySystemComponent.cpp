// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CharonAbilitySystemComponent.h"

#include "Abilities/CharonGameplayAbility.h"
#include "Engine/SpecularProfile.h"
#include "ProjectCharon.h"
#include "Animation/AnimTrace.h"

UCharonAbilitySystemComponent::UCharonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}


FDelegateHandleWrapper UCharonAbilitySystemComponent::K2_BindEventOnAttributeChange(UObject* EventSource, FGameplayAttribute AttributeToBind,
                                                               FOnCharonAttributeChanged_Dynamic Event)
{
	if(!IsValid(EventSource))
	{
		UE_LOG(LogCharon, Error, TEXT("K2_BindEventOnAttributeChange Failed : EventSource is Not VALID"));
		return FDelegateHandleWrapper();
	}
	
	FGameplayAttribute FoundAttribute = FindAttribute(AttributeToBind).Get(nullptr);
	
	if(FoundAttribute == nullptr)
	{
		UE_LOG(LogCharon, Error, TEXT("Attribute not found"));
		return FDelegateHandleWrapper();
	}
	
	// 원하는 어트리뷰트 값 변화 델리게이트에 이벤트 실행 함수 바인드. EventSoruce가 Valid하지 않으면 관련 함수 언바인드.
	TObjectPtr<UObject> EventSourcePtr = EventSource; 
	FDelegateHandle Handle = GetGameplayAttributeValueChangeDelegate(FoundAttribute).
	AddWeakLambda(EventSource, [This=this, Event, EventSourcePtr](const FOnAttributeChangeData& Data) ->
	void{
		if(IsValid(EventSourcePtr))
		{
			Event.Execute(Data.OldValue, Data.NewValue);	
		}
		else
		{
			if(This->AttributeBindHandlesOfSource.Contains(EventSourcePtr))
			{
				for(const FDelegateHandleWrapper BindHandle : This->AttributeBindHandlesOfSource[EventSourcePtr])
				{
					This->UnBindEventOnAttributeChange(BindHandle);
				}
				This->AttributeBindHandlesOfSource.Remove(EventSourcePtr);
			}			
		}		
	});
	
	if(!Handle.IsValid())
	{
		UE_LOG(LogCharon, Error, TEXT("Delegate Binding Failed"));
		return FDelegateHandleWrapper();
	}


	if(!AttributeBindHandlesOfSource.Contains(EventSourcePtr))
	{
		AttributeBindHandlesOfSource.Add({EventSourcePtr, TArray<FDelegateHandleWrapper>()});
	}
	AttributeBindHandlesOfSource[EventSourcePtr].Add({Handle, EventSourcePtr});
	AttributeBindHandles.Add({Handle, FoundAttribute});

	return FDelegateHandleWrapper(Handle, EventSourcePtr);
}

FDelegateHandleWrapper UCharonAbilitySystemComponent::BindEventOnAttributeChange(UObject* EventSource,
	FGameplayAttribute AttributeToBind, FOnCharonAttributeChanged Event)
{
	FGameplayAttribute FoundAttribute = FindAttribute(AttributeToBind).Get(nullptr);
	
	if(FoundAttribute == nullptr)
	{
		UE_LOG(LogCharon, Error, TEXT("Attribute not found"));
		return FDelegateHandleWrapper();
	}

	// 원하는 어트리뷰트 값 변화 델리게이트에 이벤트 실행 함수 바인드. EventSoruce가 Valid하지 않으면 관련 함수 언바인드.
	TObjectPtr<UObject> EventSourcePtr = EventSource; 
	FDelegateHandle Handle = GetGameplayAttributeValueChangeDelegate(FoundAttribute).
	AddWeakLambda(EventSource, [This=this, Event, EventSourcePtr](const FOnAttributeChangeData& Data) ->
	void{
		if(IsValid(EventSourcePtr))
		{
			Event.Broadcast(Data.OldValue, Data.NewValue);	
		}
		else
		{
			if(This->AttributeBindHandlesOfSource.Contains(EventSourcePtr))
			{
				for(const FDelegateHandleWrapper BindHandle : This->AttributeBindHandlesOfSource[EventSourcePtr])
				{
					This->UnBindEventOnAttributeChange(BindHandle);
				}
				This->AttributeBindHandlesOfSource.Remove(EventSourcePtr);
			}			
		}		
	});
	
	if(!Handle.IsValid())
	{
		UE_LOG(LogCharon, Error, TEXT("Delegate Binding Failed"));
		return FDelegateHandleWrapper();
	}


	if(!AttributeBindHandlesOfSource.Contains(EventSourcePtr))
	{
		AttributeBindHandlesOfSource.Add({EventSourcePtr, TArray<FDelegateHandleWrapper>()});
	}
	AttributeBindHandlesOfSource[EventSourcePtr].Add({Handle, EventSourcePtr});
	AttributeBindHandles.Add({Handle, FoundAttribute});

	return FDelegateHandleWrapper(Handle, EventSourcePtr);
}

void UCharonAbilitySystemComponent::K2_UnBindEventOnAttributeChange(const FDelegateHandleWrapper EventHandle)
{
	UnBindEventOnAttributeChange(EventHandle);
}

void UCharonAbilitySystemComponent::UnBindEventOnAttributeChange(const FDelegateHandleWrapper EventHandle)
{
	if(AttributeBindHandles.Contains(EventHandle.Handle))
	{
		if(AttributeBindHandlesOfSource.Contains(EventHandle.EventSourcePtr))
		{
			AttributeBindHandlesOfSource[EventHandle.EventSourcePtr].Remove(EventHandle);
		}
		
		const FGameplayAttribute Attribute = AttributeBindHandles[EventHandle.Handle];
		
		GetGameplayAttributeValueChangeDelegate(Attribute).Remove(EventHandle.Handle);
		AttributeBindHandles.Remove(EventHandle.Handle);
	}
}

void UCharonAbilitySystemComponent::CheckAttributeBinds()
{
	for(TTuple<FDelegateHandle, FGameplayAttribute> Tuple : AttributeBindHandles)
	{
		UE_LOG(LogCharon, Warning, TEXT("Attribute Binds With %s"),*Tuple.Value.GetName() );
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
