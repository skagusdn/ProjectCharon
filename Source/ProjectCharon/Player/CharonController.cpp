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
		// if(UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		// {
		// 	ASC->InitAbilityActorInfo(PS, InPawn);
		// }

		// 플에이어 스테이트에 캐릭터 메시 저장 -> 베히클 탈때 쓸꺼. 추후 로직 바꿀 수 있음.
		if(ACharacter* InCharacter = Cast<ACharacter>(InPawn))
		{
			PS->SetCharacterMesh(InCharacter->GetMesh());
		}
	}

	if(UCharonLocalPlayer* CharonLocalPlayer = Cast<UCharonLocalPlayer>(Player))
	{
		CharonLocalPlayer->OnPlayerPawnSet.Broadcast(CharonLocalPlayer, InPawn);
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
			OnPlayerStateSet.Broadcast(this, PlayerState);
		}

		// UI Config 등록
		if(UCharonUISubsystem* UISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCharonUISubsystem>())
		{
			if(UIConfig)
			{
				UISubsystem->RegisterUIConfigToPlayer(CharonLocalPlayer, UIConfig);
			}
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
			OnPlayerStateSet.Broadcast(this, PlayerState);
		}
	}
}

// void ACharonController::InitPlayerState()
// {
// 	Super::InitPlayerState();
//
// 	if(PlayerState)
// 	{
// 		OnPlayerStateInitiated.Broadcast();
// 	}
// }

void ACharonController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharonController, UIConfig);
}

void ACharonController::SetUIConfig(UCharonUIConfig* InUIConfig)
{
	if(HasAuthority())
	{
		UCharonUIConfig* OldUIConfig = UIConfig;
		UIConfig = InUIConfig;

		// 리슨서버일 경우.
		if(HasAuthority() && !IsRunningDedicatedServer())
		{
			OnRep_UIConfig(OldUIConfig);
		}
	}
}

// void ACharonController::UpdateUIConfigFromGameMode()
// {
// 	if(GetWorld() && GetWorld()->GetAuthGameMode())
// 	{
// 		if(ACharonGameMode* CharonGameMode = Cast<ACharonGameMode>(GetWorld()->GetAuthGameMode()))
// 		{
// 			UCharonUIConfig* OldUIConfig = UIConfig;
// 			UIConfig = CharonGameMode->GetDefaultUIConfig();
// 			if(HasAuthority() && !IsRunningDedicatedServer())
// 			{
// 				OnRep_UIConfig(OldUIConfig);
// 			}
// 		}
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
			//LocalPlayer->InitUIConfig();

			//
			if(UCharonUISubsystem* UISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCharonUISubsystem>())
			{
				UISubsystem->RegisterUIConfigToPlayer(LocalPlayer, UIConfig);
			}
			
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