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