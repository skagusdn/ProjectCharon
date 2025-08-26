// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UInteractionComponent::OnRegister()
{
	Super::OnRegister();

	if(OwnerCharacter && bIsInteracting)
	{
		InteractingTarget->CancelInteraction(OwnerCharacter);
	}
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UInteractionComponent::OnRep_IsInteracting()
{
	UE_LOG(LogTemp, Display, TEXT("bIsInteracting changed"));
}

void UInteractionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractionComponent, bIsInteracting);
}

void UInteractionComponent::TryInteraction(TScriptInterface<IInteractiveInterface> Target)
{
	if(!Target.GetObject() || !(Target.GetObject())->Implements<UInteractiveInterface>() || !OwnerCharacter)
	{
		return;
	}

	if(!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	switch(Target->Execute_Interact(Target.GetObject(), OwnerCharacter, nullptr))
	{
	case EInteractResult::InteractInProgress :
		InteractingTarget = Target;
		bIsInteracting = true;
		break;

	case EInteractResult::InteractFail :
		break;
		
	case EInteractResult::InteractEnd :
		bIsInteracting = false;
		break;
	}
}

void UInteractionComponent::TryCancelInteraction_Implementation()
{
	bIsInteracting = false;
	if(InteractingTarget != nullptr)
	{
		InteractingTarget->Execute_CancelInteraction(InteractingTarget.GetObject(), OwnerCharacter);
		InteractingTarget = nullptr;
	}
}



