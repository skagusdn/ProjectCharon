// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CharonGameMode.h"

#include "CharonGameState.h"
#include "Player/CharonPlayerState.h"

//#include "Data/GameStartingData.h"

// UClass* ACharonGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
// {
// 	if(DefaultPawnData)
// 	{
// 		return DefaultPawnData->DefaultPawn;
// 	}
// 	
// 	return Super::GetDefaultPawnClassForController_Implementation(InController);
// }



// void ACharonGameMode::OnPostLogin(AController* NewPlayer)
// {
// 	Super::OnPostLogin(NewPlayer);
// 	
// 	if(APlayerController* PC = Cast<APlayerController>(NewPlayer))
// 	{
// 		RestartPlayer(PC);
// 	}
// 	
// }
void ACharonGameMode::ReportDeath(AController* Player)
{
	if(!Player)
	{
		return;
	}

	// 임시로, Spectator로 바꿔볼까. 
	if(APlayerController* PlayerController = Cast<APlayerController>(Player))
	{
		
		PlayerController->ChangeState(NAME_Spectating);
		//GetWorldTimerManager().SetTimerForNextTick(PlayerController, &APlayerController::ServerRestartPlayer_Implementation);
		// FTimerDelegate TimeDelegate;
		// TimeDelegate.BindUObject(PlayerController, &APlayerController::ChangeState, NAME_Spectating

		FTimerDelegate TimerDel;
		FTimerHandle TimerHandle;

		TimerDel.BindUObject(this, &ThisClass::RestartPlayer, Player);
		
		GetWorldTimerManager().SetTimer(TimerHandle, TimerDel, 5.0f, false);
	}
	
	
}

void ACharonGameMode::RegisterPlayer(APlayerController* Player, FCharonPlayerInfo PlayerInfo)
{
	check(Player);
	
	if(ACharonGameState* CharonGameState =  Cast<ACharonGameState>(GameState))
	{
		CharonGameState->RegisterCrew(Player, PlayerInfo.CrewID);
	}
	if(ACharonPlayerState* CharonPlayerState =  Cast<ACharonPlayerState>(Player->PlayerState))
	{
		CharonPlayerState->CrewId = PlayerInfo.CrewID;	
	}
}
