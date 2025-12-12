// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CharonGameState.generated.h"

// USTRUCT(BlueprintType)
// struct FCrewID
// {
// 	GENERATED_BODY()
// 	FCrewID() : Value(-1) {}
// 	explicit FCrewID(int32 InValue) : Value(InValue) {}
// 	
// 	UPROPERTY(BlueprintReadOnly)
// 	int32 Value;
//
// 	bool operator==(const FCrewID& Other) const { return Value == Other.Value; }
// 	bool operator!=(const FCrewID& Other) const { return Value != Other.Value; }
// 	
// };

USTRUCT(BlueprintType)
struct FCharonCrew
{
	GENERATED_BODY()

	FCharonCrew() : CrewId(-1){};

	explicit FCharonCrew(const int InCrewId, TArray<APlayerController*> InPlayers)
		: CrewId(InCrewId), CrewMembers(InPlayers)
	{}
	// explicit FCharonCrew(const int InCrewID, TArray<APlayerController*> InPlayers)
	// 	: CrewID(FCrewID(InCrewID)), CrewMembers(InPlayers)
	// {}
	// explicit FCharonCrew(const FCrewID InCrewID, TArray<APlayerController*> InPlayers)
	// 	: CrewID(InCrewID), CrewMembers(InPlayers)
	// {}
	
	UPROPERTY(BlueprintReadOnly)
	int32 CrewId;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<APlayerController*> CrewMembers;
	
	bool operator==(const FCharonCrew& Other) const { return CrewId == Other.CrewId; }
	bool operator!=(const FCharonCrew& Other) const { return CrewId != Other.CrewId; }
};

//플레이어 정보. 우선 임시로 여기서 정의. 나중에 필요한 정보가 추가되거나 다른 곳으로 옮겨질 수도.
USTRUCT(BlueprintType)
struct FCharonPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString PlayerName;
	UPROPERTY(BlueprintReadWrite)
	int32 CrewID = -1;
};


/**
 * 
 */
UCLASS()
class PROJECTCHARON_API ACharonGameState : public AGameState
{
	GENERATED_BODY()

public:
	ACharonGameState(const FObjectInitializer& ObjectInitializer);

	//void RegisterCrew(APlayerController* Player, const int CrewID);
	void RegisterCrew(APlayerController* Player, const int32 CrewID);
	
protected:
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, FCharonCrew> CrewMap;
};
