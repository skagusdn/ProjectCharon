// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionExtensionComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CharonGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "Interaction/InteractiveInterface.h"


UInteractionExtensionComponent::UInteractionExtensionComponent()
{

	//PrimaryComponentTick.bCanEverTick = true;

}

FGameplayAbilitySpecHandle UInteractionExtensionComponent::GrantAbilityAndActivate(
	UAbilitySystemComponent* AbilitySystemComponent, UGameplayAbility* InteractAbility,
	TSubclassOf<UGameplayAbility> AbilityForInteraction, bool HasOptionalEventData,
	FGameplayEventData OptionalEventData)
{
	if(!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent Is Not VALID"));
		return FGameplayAbilitySpecHandle();
	}

	AActor* Owner = GetOwner();
	if(!Owner || !Owner->GetClass()->ImplementsInterface(UInteractiveInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor Is Not VALID or did NOT Implement demanding Interface."));
		return FGameplayAbilitySpecHandle();
	}
	
	if(!Owner->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Authority is Necessary for Granting Ability"));
		return FGameplayAbilitySpecHandle();
	}
	
	AActor* Instigator = InteractAbility->GetAvatarActorFromActorInfo();
	FGameplayEventData Payload = HasOptionalEventData ? OptionalEventData : FGameplayEventData();

	if(!HasOptionalEventData)
	{
		Payload.Instigator = Instigator;
		Payload.Target = Owner;
	}

	FGameplayAbilityActorInfo ActorInfo;
	ActorInfo.InitFromActor(InteractAbility->GetOwningActorFromActorInfo(), Instigator, AbilitySystemComponent);

	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityForInteraction);
	FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	
	///////////////
	
	FGameplayAbilitySpec* AbilitySpec2 = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle, EConsiderPending::PendingAdd);
	if(!AbilitySpec2)
	{
		UE_LOG(LogTemp, Error, TEXT("Temp - AbilitySpec2 Is InValid"));
	}
	//////////////////

	//발동하지 말아볼까?
	// bool bSuccess = AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
	// AbilitySpecHandle,
	// 	&ActorInfo,
	// CharonGameplayTags::Tag_Ability_Event_Interaction,
	// 	&Payload,
	// 	*AbilitySystemComponent
	// 	);

	
	


	// //
	// if(!bSuccess)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Triggering Ability For Interaction Failed"));
	// }
	//
	
	// 부여한 어빌리티의 해제는 일단은 어빌리티 자체에서. bRemoveAbilityOnEnd.
	// TODO 좀 더 명시적으로 어빌리티를 부여할때 bRemovedAbilityOnEnd를 True로 하고 싶음.
	
	return AbilitySpecHandle;
}


void UInteractionExtensionComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UInteractionExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const AActor* OwningActor = GetOwner();
	if(!OwningActor->GetClass()->ImplementsInterface(UInteractiveInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("Owning Actor of This %s Should Implements %s"), *ThisClass::GetName(), *UInteractiveInterface::StaticClass()->GetName() );

#if WITH_EDITOR
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Warning : This Component Needs Implementing Specific Interface!")));
#endif
	}
}




