// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CharonAbilitySystemComponent.h"

#include "Abilities/CharonGameplayAbility.h"
// "Engine/SpecularProfile.h"
#include "ProjectCharon.h"
//#include "Animation/AnimTrace.h"

UCharonAbilitySystemComponent::UCharonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UCharonAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (bHasNewPawnAvatar)
	{
		// ASC가 초기화 될 경우 어트리뷰트도 초기화.
		InitAttributesWithDefaultData();
	}
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

void UCharonAbilitySystemComponent::CheckAbilitySpecs(FGameplayAbilitySpecHandle Handle)
{
	//SCOPE_CYCLE_COUNTER(STAT_FindAbilitySpecFromHandle);

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		UE_LOG(LogTemp, Warning, TEXT("CheckAbilitySpecs : %s, Handle : %s"), *Spec.Ability.GetName(), *Spec.Handle.ToString());
		if (Spec.Handle == Handle)
		{
			UE_LOG(LogTemp, Warning, TEXT("CheckAbilitySpecs, FOUND!"));
			
		}
	}

	for (const FGameplayAbilitySpec& Spec : AbilityPendingAdds)
	{
		UE_LOG(LogTemp, Warning, TEXT("CheckAbilitySpecs - PendAdds : %s, Handle : %s"), *Spec.Ability.GetName(), *Spec.Handle.ToString());
		if (Spec.Handle == Handle)
		{
			UE_LOG(LogTemp, Warning, TEXT("CheckAbilitySpecs, FOUND!"));
		}
		
	}
}


void UCharonAbilitySystemComponent::AbilityLocalInputTagPressed(FGameplayTag InputTag)
{
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			if (Spec.Ability)
			{
				Spec.InputPressed = true;
				if (Spec.IsActive())
				{
					AbilitySpecInputPressed(Spec);
				}
				else
				{
					// 어빌리티 비활성 상태이면 액티비티 발동.
					TryActivateAbility(Spec.Handle);
					
					// UCharonGameplayAbility* CharonAbility = Cast<UCharonGameplayAbility>(Spec.Ability);
					// if(CharonAbility && (CharonAbility->GetActivationPolicy() == ECharonAbilityActivationPolicy::OnInputTriggered ||
					// 	CharonAbility->GetActivationPolicy() == ECharonAbilityActivationPolicy::WhileInputActive))
					// {
					// 	TryActivateAbility(Spec.Handle);
					// }								
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
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			Spec.InputPressed = false;
			if (Spec.Ability && Spec.IsActive())
			{
				AbilitySpecInputReleased(Spec);

				// 누르고 있을때만 발동하는 어빌리티는 떼면 취소하기.
				if(UCharonGameplayAbility* CharonAbilityCDO = Cast<UCharonGameplayAbility>(Spec.Ability))
				{
					if(CharonAbilityCDO->GetActivationPolicy() == ECharonAbilityActivationPolicy::WhileInputActive)
					{
						CancelAbilityHandle(Spec.Handle);
					}
				}
			}
		}
	}
}

void UCharonAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	UE_LOG(LogTemp, Display, TEXT("Ability - %s Has Granted"), *AbilitySpec.Ability.GetName());
}

void UCharonAbilitySystemComponent::InitAttributesWithDefaultData()
{
	// Init starting data
	for (int32 i=0; i < DefaultStartingData.Num(); ++i)
	{
		if (DefaultStartingData[i].Attributes && DefaultStartingData[i].DefaultStartingTable)
		{
			UAttributeSet* Attributes = const_cast<UAttributeSet*>(GetOrCreateAttributeSubobject(DefaultStartingData[i].Attributes));
			Attributes->InitFromMetaDataTable(DefaultStartingData[i].DefaultStartingTable);
		}
	}
}

void UCharonAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);
	
	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	// 라이라에서 가져온 코드. 5.4 -> 5.6 버젼 바꾸면서 달라진 부분.
	// 달라진 로직에선 PredictionKey가 뭐 리플리케이션 문제 같은것 때문에 뭔가 달라지나 봄. 
	if (Spec.IsActive())
	{
		
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();

		UE_LOG(LogTemp, Warning, TEXT("Prediction KEY1 : %s"), *OriginalPredictionKey.ToString());//
		UE_LOG(LogTemp, Warning, TEXT("Prediction KEY2 : %s"), *Spec.ActivationInfo.GetActivationPredictionKey().ToString());//
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
		
	}
}

void UCharonAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	// Pressed랑 마찬가지.
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
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
