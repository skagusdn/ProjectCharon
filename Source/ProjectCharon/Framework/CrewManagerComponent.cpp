// Fill out your copyright notice in the Description page of Project Settings.


#include "CrewManagerComponent.h"

#include "Logging.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"


// Sets default values for this component's properties
UCrewManagerComponent::UCrewManagerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCrewManagerComponent::RegisterCrew(APlayerController* Player, const int32 CrewID)
{
	check(CrewID >= 0);
	
	if(!GetOwner()->HasAuthority())
	{
		return;
	}
	
	// if(!CrewMap.Contains(CrewID))
	// {
	// 	CrewMap.Add(CrewID, FCharonCrew(CrewID, TArray<APlayerController*>()));		
	// }
	//
	// FCharonCrew& Crew = CrewMap[CrewID];
	// Crew.CrewMembers.Add(Player);
	//
	// Server_UpdateCrewMap();

	// FCharonCrew* FoundCrew = nullptr;
	// for(FCharonCrew Crew : Crews)
	// {
	// 	if(Crew.CrewId == CrewID)
	// 	{
	// 		FoundCrew = &Crew;
	// 		break;
	// 	}
	// }
	//
	// if(!FoundCrew)
	// {
	// 	FoundCrew = &Crews[Crews.Add(FCharonCrew(CrewID, TArray<APlayerState*>()))];
	// }
	//
	//
	// FoundCrew->CrewMembers.AddUnique(Player->PlayerState);

	int FoundCrewIdx = -1;
	for(int i = 0; i < Crews.Num(); i++)
	{
		if(Crews[i].CrewId == CrewID)
		{
			FoundCrewIdx = i;
			break;
		}
	}
	
	if(FoundCrewIdx < 0)
	{
		FoundCrewIdx = Crews.Add(FCharonCrew(CrewID, TArray<APlayerState*>()));
	}
	
	Crews[FoundCrewIdx].CrewMembers.AddUnique(Player->PlayerState);
}



FCharonCrew UCrewManagerComponent::FindCrew(const int32 CrewId) const
{
	// if(CrewMap.Contains(CrewId))
	// {
	// 	return CrewMap[CrewId];
	// }
	for(FCharonCrew Crew : Crews)
	{
		if(Crew.CrewId == CrewId)
		{
			return Crew;
		}
	}
	
	return FCharonCrew();
}

int UCrewManagerComponent::GetCrewID(APlayerController* Player) const
{
	// for(const auto& [CrewId, Crew] : CrewMap)
	// {
	// 	if(Crew.CrewMembers.Contains(Player))
	// 	{
	// 		return CrewId;
	// 	}
	// }

	for(FCharonCrew Crew : Crews)
	{
		if(Crew.CrewMembers.Contains(Player->PlayerState))
		{
			return Crew.CrewId;
		}
	}

	return -1;
}

void UCrewManagerComponent::UpdateAccessibleDockInfo_Implementation(int32 CrewId, ACharonDock* Dock, bool NewAccess)
{
	// if(!CrewMap.Contains(CrewId))
	// {
	// 	return;
	// }
	//
	// if(NewAccess)
	// {
	// 	CrewMap.Find(CrewId)->AccessibleDocks.AddUnique(Dock);
	// }
	// else
	// {
	// 	CrewMap.Find(CrewId)->AccessibleDocks.Remove(Dock);
	// }

	FCharonCrew* FoundCrew = nullptr;
	for(FCharonCrew Crew : Crews)
	{
		if(Crew.CrewId == CrewId)
		{
			FoundCrew = &Crew;
			break;
		}
	}

	if(!FoundCrew || FoundCrew->CrewId < 0)
	{
		return;
	}
	
	if(NewAccess)
	{
		FoundCrew->AccessibleDocks.AddUnique(Dock);
	}
	else if(FoundCrew->AccessibleDocks.Contains(Dock))
	{
		FoundCrew->AccessibleDocks.Remove(Dock);
	}
}

void UCrewManagerComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	
	if (!Owner || !Owner->IsA(AGameStateBase::StaticClass()))
	{
		UE_LOG(LogCharon, Warning, TEXT("UCrewManagerComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
 
		// 컴포넌트 비활성화: Tick 및 기능 차단
		ensureAlwaysMsgf((false), TEXT("UCrewManagerComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
		SetActive(false);
	}
}

// void UCrewManagerComponent::OnRep_Crews(const TArray<FCharonCrew>& OldValue)
// {
// 	UE_LOG(LogTemp, Display, TEXT("Crews Replicated"));
// }

void UCrewManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UCrewManagerComponent, CrewEntries)
	//FDoRepLifetimeParams Params;
	//Params.bIsPushBased = true;
	
	//DOREPLIFETIME_WITH_PARAMS(UCrewManagerComponent, Crews, Params);
	DOREPLIFETIME(UCrewManagerComponent, Crews);

}
//
// void UCrewManagerComponent::OnRep_CrewEntries(const TArray<FCrewEntry>& OldValue)
// {
// 	
// }


// void UCrewManagerComponent::Server_UpdateCrewMap()
// {
// 	if(!GetOwner()->HasAuthority())
// 	{
// 		return;
// 	}
//
// 	TArray<FCrewEntry> CrewEntries;
// 	for(const auto& [CrewId, Crew] : CrewMap)
// 	{
// 		CrewEntries.Add({ CrewId, Crew });
// 	}
//
// 	Multicast_UpdateCrewMap(CrewEntries);
// }
//
// void UCrewManagerComponent::Multicast_UpdateCrewMap_Implementation(const TArray<FCrewEntry>& CrewEntries)
// {
// 	if(GetOwner()->HasAuthority())
// 	{
// 		return;
// 	}
// 	
// 	CrewMap.Empty();
// 	for(const auto& [CrewId, Crew] : CrewEntries)
// 	{
// 		CrewMap.Add(CrewId, Crew);
// 	}
// }


