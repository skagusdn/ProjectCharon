// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleGameStateComponent.generated.h"


class ACharonDock;
class AVehicle;

USTRUCT()
struct FVehicleEntry
{
	GENERATED_BODY()
 
	UPROPERTY()
	int32 CrewId;
 
	UPROPERTY()
	AVehicle* Vehicle;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UVehicleGameStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UVehicleGameStateComponent();

	UFUNCTION(BlueprintCallable)
	bool CantRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	AVehicle* RentVehicle(const int32 CrewId, TSubclassOf<AVehicle> VehicleClass, const FVector Location, const FRotator Rotation);

	UFUNCTION(BlueprintCallable)
	AVehicle* FindCrewVehicle(int32 CrewId);
	
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	
	void Server_UpdateCrewVehicles();
	UFUNCTION(Client, Reliable)
	void Client_UpdateCrewVehicles(const TArray<FVehicleEntry>& VehicleEntries);
	
	UPROPERTY(BlueprintReadOnly)
	TArray<ACharonDock*> Docks;
	
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, AVehicle*> CrewVehicles;
public:
	
};
