// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "GameFramework/Character.h"


// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	
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

void UInteractionComponent::TryInteraction(TScriptInterface<IInteractiveInterface> Target)
{
	if(Target == nullptr || !OwnerCharacter)
	{
		return;
	}

	if(!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	switch(Target->Execute_Interact(Target.GetObject(), OwnerCharacter))
	{
	case InteractResult::InteractInProgress :
		InteractingTarget = Target;
		bIsInteracting = true;
		break;

	case InteractResult::InteractFail :
		break;
		
	case InteractResult::InteractEnd :
		bIsInteracting = false;
		break;
	}
}

void UInteractionComponent::TryCancelInteraction()
{
	if(!bIsInteracting || InteractingTarget == nullptr)
	{
		bIsInteracting = false;
		return;
	}
	
	InteractingTarget->Execute_CancelInteraction(InteractingTarget.GetObject(), OwnerCharacter);
}



