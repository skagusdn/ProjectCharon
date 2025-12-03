// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonGameState.h"

ACharonGameState::ACharonGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
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
