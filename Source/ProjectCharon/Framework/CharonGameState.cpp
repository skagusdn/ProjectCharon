// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonGameState.h"

#include "EngineUtils.h"
#include "Vehicle/VehicleManager/DockManagerComponent.h"


ACharonGameState::ACharonGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// VehicleRentalManager = CreateDefaultSubobject<UDockManagerComponent>(TEXT("VehicleGameState"));
}

// void ACharonGameState::RegisterCrew(APlayerController* Player, const int32 CrewID)
// {
// 	if(!CrewMap.Contains(CrewID))
// 	{
// 		CrewMap.Add(CrewID, FCharonCrew(CrewID, TArray<APlayerController*>()));		
// 	}
//
// 	FCharonCrew Crew = CrewMap[CrewID];
// 	Crew.CrewMembers.Add(Player);
// }
//
// FCharonCrew ACharonGameState::FindCrew(const int32 CrewId) const
// {
// 	if(CrewMap.Contains(CrewId))
// 	{
// 		return CrewMap[CrewId];
// 	}
// 	
// 	return FCharonCrew();
// }

// void ACharonGameState::NotifyCrewGotAccessForDock(int32 CrewId, ACharonDock* Dock)
// {
// 	if(!HasAuthority() || !Dock)
// 	{
// 		return;
// 	}
//
// 	//유효하지 않은 크루id라면 FindCrew로 리턴된 크루는 크루id가 -1.
// 	FCharonCrew Crew = FindCrew(CrewId);
// 	if(Crew.CrewId == CrewId)
// 	{
// 		Crew.AccessibleDocks.Add(Dock);
// 	}
//
// 	
// 	
// }

// int ACharonGameState::GetCrewID(APlayerController* Player) const
// {
// 	for(const auto& Tuple : CrewMap)
// 	{
// 		FCharonCrew Crew = Tuple.Value;
// 		if(Crew.CrewMembers.Contains(Player))
// 		{
// 			return Tuple.Key;
// 		}
// 	}
//
// 	return -1;
// }

void ACharonGameState::BeginPlay()
{
	Super::BeginPlay();
	
}
