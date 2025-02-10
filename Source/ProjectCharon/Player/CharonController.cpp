// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonController.h"
#include "CharonPlayerState.h"
#include "Character/CharonCharacter.h"
#include "Input/CharonInputComponent.h"

UCharonAbilitySystemComponent* ACharonController::GetCharonAbilitySystemComponent() const
{
	ACharonPlayerState* CharonPS = Cast<ACharonPlayerState>(PlayerState);
		
	return CharonPS? CharonPS->GetCharonAbilitySystemComponent() : nullptr;
}

void ACharonController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (ACharonPlayerState* PS = GetPlayerState<ACharonPlayerState>())
	{
		if(UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->InitAbilityActorInfo(PS, InPawn);
		}
	}

	// if(ACharonCharacter* CharonCharacter = Cast<ACharonCharacter>(InPawn))
	// {
	// 	CharonCharacter->InitCharonCharacter();
	// }
}

