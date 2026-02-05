// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonDock.h"

#include "Framework/CharonGameState.h"
#include "GameFramework/GameStateBase.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleManager/DockManagerComponent.h"


// Sets default values
ACharonDock::ACharonDock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsSpawningVehicle = false;
}

AVehicle* ACharonDock::RentVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass, FVector Location,
	FRotator Rotation)
{
	check(DockManager);
	
	if(!HasAuthority()|| !CanRentThisVehicle(CrewId, VehicleClass) || bIsSpawningVehicle)
	{
		return nullptr;
	}

	bIsSpawningVehicle = true;

	TWeakObjectPtr<ACharonDock> WeakThis(this);
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		[WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis.Get()->bIsSpawningVehicle = false;
			}
		},
		0.5f,
		false
	);
	
	return DockManager->RentVehicle(CrewId, VehicleClass, Location, Rotation);
}

void ACharonDock::ReturnRentalVehicle(int32 CrewId)
{
	
}

void ACharonDock::BeginPlay()
{
	Super::BeginPlay();

	if(AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		if(UDockManagerComponent* DockManagerComponent = GameState->FindComponentByClass<UDockManagerComponent>())
		{
			DockManager = DockManagerComponent;
		}
	}
	
	
}

bool ACharonDock::CanRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass)
{
	return DockManager->CanRentThisVehicle(CrewId, VehicleClass);
}

void ACharonDock::SetCrewAccess(int32 CrewId, bool NewAccess)
{
	if(!HasAuthority())
	{
		return;
	}
	
	if(!CrewAccessMap.Contains(CrewId) || CrewAccessMap[CrewId] != NewAccess)
	{
		if(DockManager)
		{
			DockManager->UpdateDockAccess(CrewId, this, NewAccess);
		}
	}
	CrewAccessMap.Add(CrewId, NewAccess);
}

