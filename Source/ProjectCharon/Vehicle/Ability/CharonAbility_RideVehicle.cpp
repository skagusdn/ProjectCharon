// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonAbility_RideVehicle.h"

#include "Logging.h"

#include "Character/CharonCharacter.h"
#include "GameFramework/Character.h"
#include "Vehicle/Vehicle.h"

UCharonAbility_RideVehicle::UCharonAbility_RideVehicle(const FObjectInitializer& ObjectInitializer)
{
	//ActivationPolicy = ECharonAbilityActivationPolicy::OnInputTriggered
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

// void UCharonAbility_EnterVehicle::EndAbility(const FGameplayAbilitySpecHandle Handle,
// 	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
// 	bool bReplicateEndAbility, bool bWasCancelled)
// {
// 	if(ACharonCharacter* CharonRider = Cast<ACharonCharacter>(Rider))
// 	{
// 		CharonRider->ResetAbilityConfig();
// 	}
// 	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
// }

bool UCharonAbility_RideVehicle::TryEnterVehicle(AVehicle* VehicleToEnter, ACharacter* InRider)
{
	if(!VehicleToEnter || !InRider)
	{
		UE_LOG(LogCharon,Warning,TEXT("TryRideVehicle Failed : Vehicle or Rider is not valid."));
		return false;
	}

	//아바타 액터와 라이더가 다른지 테스트. 이 부분이나 Rider 파라미터가 꼭 필요한가 싶긴함. 
	ACharacter* AvatarActor = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if(!AvatarActor || AvatarActor != InRider)
	{
		UE_LOG(LogCharon,Warning,TEXT("TryRideVehicle Failed : Avatar of ability is not same with Rider."));
		return false;
	}

	Rider = InRider;
	
	if(!VehicleToEnter->EnterVehicle(InRider))
	{
		UE_LOG(LogCharon,Warning,TEXT("TryRideVehicle Failed : Vehicle denied rider."));
		return false;
	}

	RidingVehicle = VehicleToEnter;

	int RiderIdx = RidingVehicle->FindRiderIdx(InRider);

	if(RiderIdx < 0)
	{
		UE_LOG(LogCharon,Error,TEXT("TryRideVehicle ERROR : SOMETHING WRONG!"));
		return false;
	}
	
	FRiderSpecData RiderSpecData = RidingVehicle->GetRiderSpecData(RiderIdx);

	if(ACharonCharacter* CharonRider = Cast<ACharonCharacter>(InRider))
	{
		CharonRider->SwitchAbilityConfig(RiderSpecData.AbilityConfig, RiderSpecData.InputFunctionSet);
	}

	return true;
}

bool UCharonAbility_RideVehicle::TryExitVehicle(AVehicle* VehicleToExit, ACharacter* InRider)
{
	if(!VehicleToExit || !InRider)
	{
		UE_LOG(LogCharon,Warning,TEXT("TryExitVehicle Failed : Vehicle or Rider is not valid."));
		return false;
	}

	int RiderIdx = VehicleToExit->FindRiderIdx(InRider);
	
	if(RiderIdx < 0)
	{
		UE_LOG(LogCharon,Warning,TEXT("TryExitVehicle Failed : Rider is not riding this Vehicle"));
		return false;
	}
	
	if(!VehicleToExit->ExitVehicle(InRider))
	{
		UE_LOG(LogCharon,Warning,TEXT("TryExitVehicle Failed : Rider has denied to leave Vehicle"));
		return false;
	}

	if(ACharonCharacter* CharonRider = Cast<ACharonCharacter>(InRider))
	{
		CharonRider->ResetAbilityConfig();
	}

	return true;
}
