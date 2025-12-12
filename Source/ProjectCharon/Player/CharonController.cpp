// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonController.h"
#include "CharonPlayerState.h"
#include "Character/CharonCharacter.h"
#include "Input/CharonInputComponent.h"
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
		// GetWorldTimerManager().SetTimerForNextTick([PS]()-> void
		// {
		// 	
		// 	PS->Multicast_NotifyPawnChanged();
		// });
		
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