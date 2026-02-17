// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonLocalPlayer.h"

#include "CharonController.h"
#include "UI/CharonUISubsystem.h"

UCharonLocalPlayer::UCharonLocalPlayer() : Super(FObjectInitializer::Get())
{
	
	// CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate::CreateLambda(
	// 	[This=this](UCharonLocalPlayer* InLocalPlayer, APlayerController* InPlayerController)->void
	// {
	// 	This->InitUIConfig();
	// }));
}

FDelegateHandle UCharonLocalPlayer::CallAndRegister_OnPlayerControllerSet(
	FPlayerControllerSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());

	if (PC)
	{
		Delegate.Execute(this, PC);
	}

	return OnPlayerControllerSet.Add(Delegate);
}

FDelegateHandle UCharonLocalPlayer::CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APlayerState* PlayerState = PC ? PC->PlayerState : nullptr;

	if (PlayerState)
	{
		Delegate.Execute(this, PlayerState);
	}
	
	return OnPlayerStateSet.Add(Delegate);
}

FDelegateHandle UCharonLocalPlayer::CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	if (Pawn)
	{
		Delegate.Execute(this, Pawn);
	}

	return OnPlayerPawnSet.Add(Delegate);
}

// void UCharonLocalPlayer::InitUIConfig()
// {
// 	if(ACharonController* CharonController = Cast<ACharonController>(GetPlayerController(GetWorld())))
// 	{
// 		if(UCharonUIConfig* UIConfig = CharonController->GetUIConfig())
// 		{
// 			if(UCharonUISubsystem* UISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCharonUISubsystem>())
// 			{
// 				UISubsystem-> RegisterUIConfigToPlayer(this, UIConfig);
// 			}
// 			
// 		}
// 	}
// }
