// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonController.h"

#include "CharonLocalPlayer.h"
#include "CharonPlayerState.h"
#include "Character/CharonCharacter.h"
#include "Framework/CharonGameMode.h"
#include "Input/CharonInputComponent.h"
#include "UI/CharonUISubsystem.h"
#include "Vehicle/VehicleManager/VehicleManagerSubsystem.h"


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

	if(UCharonLocalPlayer* CharonLocalPlayer = Cast<UCharonLocalPlayer>(Player))
	{
		CharonLocalPlayer->OnPlayerPawnSet.Broadcast(CharonLocalPlayer, InPawn);
	}
	
	// // VehicleManager에게 폰이 바꼈다고 알려주기 - 임시. 수정한다면 OnRep_Pawn도 고치기.
	// // TODO : 플레이어 외형 관련 로직이 확정되면 이 부분 로직 바꾸기. 
	// if (UVehicleManagerSubsystem* VehicleManager = GetWorld()->GetSubsystem<UVehicleManagerSubsystem>())
	// {
	// 	VehicleManager->UpdateRiderMesh(GetPlayerState<APlayerState>());
	// }

	if(ACharonPlayerState* PS = GetPlayerState<ACharonPlayerState>())
	{
		if(ACharacter* InCharacter = Cast<ACharacter>(InPawn))
		{
			PS->SetCharacterMesh(InCharacter->GetMesh());
		}
		
	}
	
}

void ACharonController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if(UCharonLocalPlayer* CharonLocalPlayer = Cast<UCharonLocalPlayer>(Player))
	{
		CharonLocalPlayer->OnPlayerControllerSet.Broadcast(CharonLocalPlayer, this);

		if (PlayerState)
		{
			CharonLocalPlayer->OnPlayerStateSet.Broadcast(CharonLocalPlayer, PlayerState);
		}
	}
}

void ACharonController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	
	if (UCharonLocalPlayer* LocalPlayer = Cast<UCharonLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, InPawn);
	}
}

void ACharonController::OnUnPossess()
{
	Super::OnUnPossess();
	
	if (UCharonLocalPlayer* LocalPlayer = Cast<UCharonLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, nullptr);
	}
}

void ACharonController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (PlayerState)
	{
		if (UCharonLocalPlayer* LocalPlayer = Cast<UCharonLocalPlayer>(Player))
		{
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}
}

void ACharonController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharonController, UIConfig);
}

void ACharonController::UpdateUIConfigFromGameMode()
{
	if(GetWorld() && GetWorld()->GetAuthGameMode())
	{
		if(ACharonGameMode* CharonGameMode = Cast<ACharonGameMode>(GetWorld()->GetAuthGameMode()))
		{
			UCharonUIConfig* OldUIConfig = UIConfig;
			UIConfig = CharonGameMode->GetDefaultUIConfig();
			if(HasAuthority() && !IsRunningDedicatedServer())
			{
				OnRep_UIConfig(OldUIConfig);
			}
		}
	}
}

// void ACharonController::OnRep_Pawn()
// {
// 	Super::OnRep_Pawn();
//
// 	if (UVehicleManagerSubsystem* VehicleManager = GetWorld()->GetSubsystem<UVehicleManagerSubsystem>())
// 	{
// 		VehicleManager->UpdateRiderMesh(GetPlayerState<APlayerState>());
// 	}
// }

//// 여기서부터 저 아래까지 긁어온 네트워크 시계 구현.

float ACharonController::GetServerWorldTimeDelta() const
{
	return ServerWorldTimeDelta;
}

float ACharonController::GetServerWorldTime() const
{
	return GetWorld()->GetTimeSeconds() + ServerWorldTimeDelta;
}

void ACharonController::PostNetInit()
{
	Super::PostNetInit();
	if (GetLocalRole() != ROLE_Authority)
	{
		RequestWorldTime_Internal();
		if (NetworkClockUpdateFrequency > 0.f)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::RequestWorldTime_Internal, NetworkClockUpdateFrequency, true);
		}
	}
}

void ACharonController::RequestWorldTime_Internal()
{
	ServerRequestWorldTime(GetWorld()->GetTimeSeconds());
}

void ACharonController::OnRep_UIConfig(const UCharonUIConfig* OldUIConfig)
{
	if(GetGameInstance())
	{
		if(UCharonLocalPlayer* LocalPlayer = Cast<UCharonLocalPlayer>(GetLocalPlayer()))
		{
			LocalPlayer->InitUIConfig();
		}
	}
}

void ACharonController::ClientUpdateWorldTime_Implementation(float ClientTimestamp, float ServerTimestamp)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientTimestamp;
	if (RoundTripTime < ShortestRoundTripTime)
	{
		ShortestRoundTripTime = RoundTripTime;
		ServerWorldTimeDelta = ServerTimestamp - ClientTimestamp - ShortestRoundTripTime / 2.f;
	}
}

void ACharonController::ServerRequestWorldTime_Implementation(float ClientTimestamp)
{
	const float Timestamp = GetWorld()->GetTimeSeconds();
	ClientUpdateWorldTime(ClientTimestamp, Timestamp);
}

//// ~~~~~~~~~~~~~ 네트워크 시계 구현 끝.