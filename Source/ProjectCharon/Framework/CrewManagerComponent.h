// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonGameState.h"
#include "Components/ActorComponent.h"
#include "CrewManagerComponent.generated.h"

USTRUCT()
struct FCrewEntry
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 CrewId;

	UPROPERTY()
	FCharonCrew Crew;
	
};

USTRUCT(BlueprintType)
struct FRepTestStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int TestValue;

	UPROPERTY(BlueprintReadOnly)
	TArray<int> TestArray;
};


UCLASS(Blueprintable,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UCrewManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UCrewManagerComponent();

	void RegisterCrew(APlayerController* Player, const int32 CrewID);

	UFUNCTION(BlueprintCallable)
	FCharonCrew FindCrew(const int32 CrewId) const;
	
	UFUNCTION(BlueprintCallable)
	int GetCrewID(APlayerController* Player) const;

	UFUNCTION(BlueprintNativeEvent)
	void UpdateAccessibleDockInfo(int32 CrewId, ACharonDock* Dock, bool NewAccess);

protected:
	
	//virtual void BeginPlay() override;
	virtual void OnRegister() override;

	// virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// UFUNCTION()
	// void OnRep_CrewEntries(const TArray<FCrewEntry>& OldValue);
	//
	// UPROPERTY(ReplicatedUsing = "OnRep_CrewEntries")
	// TArray<FCrewEntry> CrewEntries;
	
	// // CrewMap 리플리케이션 함수.
	// void Server_UpdateCrewMap();
	// UFUNCTION(NetMulticast, Reliable)
	// void Multicast_UpdateCrewMap(const TArray<FCrewEntry>& CrewEntries);
	// 	
	// UPROPERTY(BlueprintReadWrite)
	// TMap<int32, FCharonCrew> CrewMap;

	// UFUNCTION()
	// void OnRep_Crews(const TArray<FCharonCrew>& OldValue);
	
	//UPROPERTY(BlueprintReadOnly, ReplicatedUsing="OnRep_Crews")
	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<FCharonCrew> Crews;

	
	
	
};
