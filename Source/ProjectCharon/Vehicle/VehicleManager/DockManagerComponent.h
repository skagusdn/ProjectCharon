// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DockManagerComponent.generated.h"


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

/**
 * Dock 및 베히클 스폰 관리
 */
UCLASS(Blueprintable,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UDockManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UDockManagerComponent();

	UFUNCTION(BlueprintCallable)
	bool CanRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	AVehicle* RentVehicle(const int32 CrewId, TSubclassOf<AVehicle> VehicleClass, const FVector Location, const FRotator Rotation);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ReturnRentalVehicle(const int32 CrewId);
	
	UFUNCTION(BlueprintCallable)
	AVehicle* FindCrewVehicle(int32 CrewId);

	virtual void UpdateDockAccess(int32 CrewId, ACharonDock* Dock, bool NewAccess);

	// UFUNCTION(BlueprintImplementableEvent)
	// void UpdateCrewStartDock(int32 CrewId, ACharonDock* Dock);
	
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	
	void Server_UpdateCrewVehicles();
	UFUNCTION(Client, Reliable)
	void Client_UpdateCrewVehicles(const TArray<FVehicleEntry>& VehicleEntries);
	
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, AVehicle*> CrewVehicles;

	UPROPERTY(BlueprintReadOnly)
	TArray<ACharonDock*> Docks;
public:
	
};
