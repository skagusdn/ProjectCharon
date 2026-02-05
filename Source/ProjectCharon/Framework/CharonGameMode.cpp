// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CharonGameMode.h"

#include "CharonGameState.h"
//#include "CharonPlayerSpawnManagerComponent.h"
#include "CrewManagerComponent.h"
#include "Player/CharonController.h"
//#include "Player/CharonLocalPlayer.h"
#include "Player/CharonPlayerState.h"
#include "UI/CharonUISubsystem.h"

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
// 	if(APlayerController* Player = Cast<APlayerController>(NewPlayer))
// 	{
// 		if(UCharonLocalPlayer* LocalPlayer = Cast<UCharonLocalPlayer>(Player->GetLocalPlayer()))
// 		{
// 					
// 		}
// 	}	
// }

// void ACharonGameMode::StartPlay()
// {
// 	Super::StartPlay();
//
// 	if(UCharonUISubsystem* CharonUISubsystem =  GetGameInstance()->GetSubsystem<UCharonUISubsystem>())
// 	{
// 		CharonUISubsystem->InitUIPolicy();
// 	}
// }

// AActor* ACharonGameMode::ChoosePlayerStart_Implementation(AController* Player)
// {
// 	if(UCharonPlayerSpawnManagerComponent* SpawnManager = GameState->GetComponentByClass<UCharonPlayerSpawnManagerComponent>())
// 	{
// 		if(AActor* PlayerStart = SpawnManager->ChoosePlayerStart(Player))
// 		{
// 			return PlayerStart;
// 		}
// 	}
// 	
// 	return Super::ChoosePlayerStart_Implementation(Player);
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

		// 플레이어의 StartSpot을 초기화하지 않으면 오버라이드한 ChoosePlayerStart 함수가 호출되지 않음. 
		Player->StartSpot = nullptr;
		
		TimerDel.BindUObject(this, &ThisClass::RestartPlayer, Player);
		
		GetWorldTimerManager().SetTimer(TimerHandle, TimerDel, 5.0f, false);
	}
	
	
}

void ACharonGameMode::InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::InitializeHUDForPlayer_Implementation(NewPlayer);

	if(ACharonController* CharonPlayer = Cast<ACharonController>(NewPlayer))
	{
		CharonPlayer->UpdateUIConfigFromGameMode();
	}	
}

void ACharonGameMode::RegisterPlayer(APlayerController* Player, FCharonPlayerInfo PlayerInfo)
{
	check(Player);
	
	if(UCrewManagerComponent* CrewManager = GameState->GetComponentByClass<UCrewManagerComponent>())
	{
		CrewManager->RegisterCrew(Player, PlayerInfo.CrewID);
	}
	if(ACharonPlayerState* CharonPlayerState =  Cast<ACharonPlayerState>(Player->PlayerState))
	{
		CharonPlayerState->CrewId = PlayerInfo.CrewID;	
	}
}
