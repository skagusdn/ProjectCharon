// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonAbility_Interaction.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CharonGameplayTags.h"
#include "Logging.h"
#include "GameFramework/Character.h"
#include "Interaction/InteractiveInterface.h"

UCharonAbility_Interaction::UCharonAbility_Interaction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = ECharonAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UCharonAbility_Interaction::UpdateInteractionTarget(AActor* InInteractionTarget)
{
	bool bTargetChanged = InteractionTarget != InInteractionTarget; 
	
	if(!InInteractionTarget || !InInteractionTarget->Implements<UInteractiveInterface>())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Update Interaction Target Failed : Not Interactable"));
		DescriptionData = {};
		InteractionTarget = nullptr;
		return bTargetChanged;
	}
	TargetInteractionInfo = IInteractiveInterface::Execute_GetInteractionInfo(InInteractionTarget);
	DescriptionData = TargetInteractionInfo.DescriptionData;
	InteractionTarget = InInteractionTarget;

	return bTargetChanged;
}

FInteractionResultWithAbilityData UCharonAbility_Interaction::TriggerInteraction()
{	
	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if(!IsValid(InteractionTarget) || !Avatar)
	{
		return {EInteractResult::InteractFail};
	}
	
	return IInteractiveInterface::Execute_Interact(InteractionTarget, Avatar, this);
}

bool UCharonAbility_Interaction::TriggerAbilityForInteraction(FGameplayAbilitySpecHandle AbilitySpecHandle,
	FGameplayEventData EventData)
{
	FGameplayAbilityActorInfo ActorInfo;
	
	// if(!EventData.Instigator->Implements<UAbilitySystemInterface>())
	// {
	// 	UE_LOG(LogCharon, Warning, TEXT("TriggerAbilityForInteraction:: Cant Find AbilitySystemComponent"));
	// 	return false;
	// }
	
	//const IAbilitySystemInterface* Instigator = Cast<IAbilitySystemInterface>(EventData.Instigator);
	//UAbilitySystemComponent* AbilitySystemComponent = Instigator->GetAbilitySystemComponent();
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EventData.Instigator);

	
	if(!AbilitySystemComponent)
	{
		UE_LOG(LogCharon, Warning, TEXT("TriggerAbilityForInteraction:: AbilitySystemComponent Is not Valid"));
		return false;
	}

	if(!AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle))
	{
		UE_LOG(LogCharon, Warning, TEXT("TriggerAbilityForInteraction:: AbilitySystemComponent Do not Have Ability for Interaction"));
		return false;
	}

	//Ability
	
	ActorInfo.InitFromActor(AbilitySystemComponent->GetOwnerActor(), AbilitySystemComponent->GetAvatarActor(), AbilitySystemComponent);
	
	bool Result = AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
		AbilitySpecHandle, &ActorInfo, CharonGameplayTags::GameplayEvent_Interaction, &EventData, *AbilitySystemComponent);

	return Result;
}

