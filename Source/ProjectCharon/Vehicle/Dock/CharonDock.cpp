// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonDock.h"

#include "GameFramework/GameStateBase.h"
#include "Vehicle/Vehicle.h"


// Sets default values
ACharonDock::ACharonDock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsSpawningVehicle = false;
}

// AVehicle* ACharonDock::FindCrewVehicle(int32 CrewId)
// {
// 	if(CrewVehicles.Contains(CrewId))
// 	{
// 		return CrewVehicles[CrewId];
// 	}
// 	return nullptr;
// }

AVehicle* ACharonDock::RentVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass, FVector Location,
	FRotator Rotation)
{
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
	return VehicleGameStateComp->RentVehicle(CrewId, VehicleClass, Location, Rotation);
	
	
}

// AVehicle* ACharonDock::RentVehicle(const int32 CrewId, const TSubclassOf<AVehicle> VehicleClass, const FVector Location)
// {
// 	if(!HasAuthority()|| !CanRentThisVehicle(CrewId, VehicleClass) || bIsSpawningVehicle)
// 	{
// 		return nullptr;
// 	}
//
// 	bIsSpawningVehicle = true;
// 	
// 	if(CrewVehicles.Contains(CrewId))
// 	{
// 		if(AVehicle* RentedVehicle = FindCrewVehicle(CrewId))
// 		{
// 			RentedVehicle->Destroy();
// 			//베히클이 파괴될 때 Ride? Enter? 어빌리티 측에서 탑승자의 하차를 처리하도록.
// 			CrewVehicles.Remove(CrewId);
// 		}
// 	}
//
// 	AVehicle* RetVehicle = Cast<AVehicle>(GetWorld()->SpawnActor(VehicleClass, &Location));
// 	CrewVehicles.Add(CrewId, RetVehicle);
// 	bIsSpawningVehicle = false;
// 	
// 	return RetVehicle;
// }


void ACharonDock::BeginPlay()
{
	Super::BeginPlay();

	if(AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		if(UVehicleGameStateComponent* VehicleGameStateComponent = GameState->FindComponentByClass<UVehicleGameStateComponent>())
		{
			VehicleGameStateComp = VehicleGameStateComponent;
		}
	}
	
	
}

bool ACharonDock::CanRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass)
{
	return VehicleGameStateComp->CantRentThisVehicle(CrewId, VehicleClass);
}

