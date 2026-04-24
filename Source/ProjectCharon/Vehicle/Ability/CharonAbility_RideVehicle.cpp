// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonAbility_RideVehicle.h"

#include "Logging.h"

#include "Character/CharonCharacter.h"
#include "GameFramework/Character.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleRiderComponent.h"

UCharonAbility_RideVehicle::UCharonAbility_RideVehicle(const FObjectInitializer& ObjectInitializer)
{
	//ActivationPolicy = ECharonAbilityActivationPolicy::OnInputTriggered
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UCharonAbility_RideVehicle::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	AActor* Avatar = GetAvatarActorFromActorInfo();
	Rider = Cast<ACharacter>(Avatar);
	ensureAlwaysMsgf((Rider != nullptr), TEXT("CharonAbility_RideVehicle on [%s] can only be given to Character actors."), *GetNameSafe(Avatar));
}

void UCharonAbility_RideVehicle::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if(RidingVehicle != nullptr)
	{
		TryExitVehicle(true);
	}
	
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UCharonAbility_RideVehicle::OnRidingVehicleDestroyed(AActor* DestroyedActor)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);

}


bool UCharonAbility_RideVehicle::TryEnterVehicle(AVehicle* VehicleToEnter)
{
	if(!VehicleToEnter || !Rider)
	{
		UE_LOG(LogCharon,Warning,TEXT("TryRideVehicle Failed : Vehicle or Rider is not valid."));
		return false;
	}
	
	ACharacter* AvatarActor = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if(!AvatarActor || AvatarActor != Rider)
	{
		UE_LOG(LogCharon,Warning,TEXT("TryRideVehicle Failed : Avatar of ability is not same with Rider."));
		return false;
	}
	
	if(!VehicleToEnter->EnterVehicle(Rider))
	{
		UE_LOG(LogCharon,Warning,TEXT("TryRideVehicle Failed : Vehicle denied rider."));
		return false;
	}

	RidingVehicle = VehicleToEnter;

	const int RiderIdx = RidingVehicle->FindRiderIdx(Rider);

	if(RiderIdx < 0)
	{
		UE_LOG(LogCharon,Error,TEXT("TryRideVehicle ERROR : SOMETHING WRONG!"));
		return false;
	}
	
	if(UVehicleRiderComponent* RiderComponent = UVehicleRiderComponent::FindRiderComponent(Rider))
	{
		RiderComponent->SetRidingVehicle(RidingVehicle);
	}
	
	RidingVehicle->OnDestroyed.AddDynamic(this, &UCharonAbility_RideVehicle::OnRidingVehicleDestroyed);
	
	return true;
}

bool UCharonAbility_RideVehicle::TryExitVehicle(bool bForcedExit)
{
	if(RidingVehicle && Rider)
	{
		int RiderIdx = RidingVehicle->FindRiderIdx(Rider);
	
		if(RiderIdx >= 0)
		{
			if(!RidingVehicle->ExitVehicle(Rider, bForcedExit))
			{
				UE_LOG(LogCharon,Warning,TEXT("TryExitVehicle Failed : Rider has denied to leave Vehicle"));
				return false;
			}
		}
		else
		{
			UE_LOG(LogCharon,Warning,TEXT("TryExitVehicle Failed : Rider is not riding this Vehicle"));
			//return false;
		}
		
	}

	if(Rider)
	{
		if(UVehicleRiderComponent* RiderComponent = UVehicleRiderComponent::FindRiderComponent(Rider))
		{
			RiderComponent->SetRidingVehicle(nullptr);
		}
	}
	
	if(RidingVehicle)
	{
		RidingVehicle->OnDestroyed.RemoveAll(this);
		RidingVehicle = nullptr;
	}
	
	return true;
}

