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

// FGameplayAbilitySpecHandle UCharonAbility_Interaction::GrantAbilityAndActivate(TSubclassOf<UGameplayAbility> AbilityForInteraction, AActor* InteractionObject, const bool HasOptionalEventData,
// 	FGameplayEventData OptionalEventData)
// {
// 	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
// 	
// 	if(InteractionTarget != InteractionObject)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Wrong Interaction Target"));
// 		return FGameplayAbilitySpecHandle();
// 	}
//
// 	if(!AbilitySystemComponent)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent Is Not VALID"));
// 		return FGameplayAbilitySpecHandle();
// 	}
//
// 	if(HasAuthority(&CurrentActivationInfo))
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Authority is Necessary for Granting Ability"));
// 		return FGameplayAbilitySpecHandle();
// 	}
// 	
// 	AActor* Instigator = GetAvatarActorFromActorInfo();
// 	FGameplayEventData Payload = HasOptionalEventData ? OptionalEventData : FGameplayEventData();
//
// 	if(!HasOptionalEventData)
// 	{
// 		Payload.Instigator = Instigator;
// 		Payload.Target = InteractionTarget;
// 	}
//
// 	FGameplayAbilityActorInfo ActorInfo;
// 	ActorInfo.InitFromActor(AbilitySystemComponent->GetOwnerActor(), AbilitySystemComponent->GetAvatarActor(), AbilitySystemComponent);
//
// 	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityForInteraction);
// 	FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
// 	
// 	bool bSuccess = AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
// 	AbilitySpecHandle,
// 		&ActorInfo,
// 	CharonGameplayTags::Tag_Ability_Event_Interaction,
// 		&Payload,
// 		*AbilitySystemComponent
// 		);
//
// 	return AbilitySpecHandle;
// }



void UCharonAbility_Interaction::UpdateInteractionTarget(AActor* InInteractionTarget)
{
	if(!InInteractionTarget || !InInteractionTarget->Implements<UInteractiveInterface>())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Update Interaction Target Failed : Not Interactable"));
		DescriptionData = {};
		InteractionTarget = nullptr;
		return;
	}
	TargetInteractionInfo = IInteractiveInterface::Execute_GetInteractionInfo(InInteractionTarget);
	DescriptionData = TargetInteractionInfo.DescriptionData;
	InteractionTarget = InInteractionTarget;
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
	
	const IAbilitySystemInterface* Instigator = Cast<IAbilitySystemInterface>(EventData.Instigator);
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

// void UCharonAbility_Interaction::PressInteractionInputOnceMore()
// {
// 	if(!InteractionTarget)
// 	{
// 		return;
// 	}
// 	if(ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
// 	{
// 		IInteractiveInterface::Execute_PressInputOnceMore(InteractionTarget, Avatar);
// 	}
// 	
// }

// void UCharonAbility_Interaction::InputPressed(const FGameplayAbilitySpecHandle Handle,
// 	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
// {
// 	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
//
// 	UE_LOG(LogTemp, Display, TEXT("TESTING~~~~~~~~ Interaction InputPressed"));
// 		
// 	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo(); 
// 	FSimpleMulticastDelegate& TargetDelegate = ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, Handle, ActivationInfo.GetActivationPredictionKey());
// 	UE_LOG(LogTemp, Display, TEXT("TESTING~~~~~~~~ SIZE :  %llu"), TargetDelegate.GetAllocatedSize());
// }



// void UCharonAbility_Interaction::Interact_GrantAbilityAndActivate()
// {
// 	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
// 	if()
// }
