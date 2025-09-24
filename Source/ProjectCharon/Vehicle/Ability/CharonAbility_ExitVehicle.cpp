// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonAbility_ExitVehicle.h"

#include "Logging.h"
#include "Character/CharonCharacter.h"
#include "Vehicle/Vehicle.h"

UCharonAbility_ExitVehicle::UCharonAbility_ExitVehicle(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool UCharonAbility_ExitVehicle::TryExitVehicle(AVehicle* VehicleToExit, ACharacter* InRider)
{
	if(!VehicleToExit || !InRider)
	{
		UE_LOG(LogCharon,Warning,TEXT("TryExitVehicle Failed : Vehicle or Rider is not valid."));
		return false;
	}

	int RiderIdx = VehicleToExit->FindRiderIdx(InRider);
	
	if(RiderIdx < 0)
	{
		UE_LOG(LogCharon,Warning,TEXT("TryRideVehicle Failed : Rider is not riding this Vehicle"));
		return false;
	}
	
	if(!VehicleToExit->ExitVehicle(InRider))
	{
		UE_LOG(LogCharon,Warning,TEXT("TryRideVehicle Failed : Rider has denied to leave Vehicle"));
		return false;
	}

	if(ACharonCharacter* CharonRider = Cast<ACharonCharacter>(InRider))
	{
		CharonRider->ResetAbilityConfig();
	}

	return true;
}
