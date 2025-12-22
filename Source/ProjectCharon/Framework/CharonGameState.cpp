// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonGameState.h"

#include "Vehicle/VehicleManager/VehicleGameStateComponent.h"

ACharonGameState::ACharonGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VehicleGameStateComponent = CreateDefaultSubobject<UVehicleGameStateComponent>(TEXT("VehicleGameState"));
}

void ACharonGameState::RegisterCrew(APlayerController* Player, const int32 CrewID)
{
	if(!CrewMap.Contains(CrewID))
	{
		CrewMap.Add(CrewID, FCharonCrew(CrewID, TArray<APlayerController*>()));		
	}

	FCharonCrew Crew = CrewMap[CrewID];
	Crew.CrewMembers.Add(Player);
}
