// Fill out your copyright notice in the Description page of Project Settings.


#include "RiderAbility.h"

#include "VehicleAbility.h"
#include "GameFramework/Character.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleRiderComponent.h"

URiderAbility::URiderAbility()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	
	bShouldRegisterToSlot = false;
	bDoesShareCooldownWithTargetVehicleAbility = false;
}

// void URiderAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
// {
// 	Super::OnGiveAbility(ActorInfo, Spec);
//
// 	//InitVehicleInfo();
//
// 	//TargetVehicleAbilityClass->GetDefaultObject()->;
// }


bool URiderAbility::DoesVehicleImplementsTargetClass() const
{
	if (AVehicle* Vehicle = FindTargetVehicle())
	{
		return Vehicle->IsA(TargetVehicleClass);
	}
	return false;
}

bool URiderAbility::DoesVehicleHasTargetAbility() const
{
	if(AVehicle* Vehicle = FindTargetVehicle())
	{
		if(UAbilitySystemComponent* TargetASC = Vehicle->GetAbilitySystemComponent())
		{
			for(FGameplayAbilitySpec& Spec : TargetASC->GetActivatableAbilities())
			{
				if(Spec.Ability && Spec.Ability->GetClass() == VehicleAbilityClassToTrigger)
				{
					return true;
				}
			}
		}
	}
	return false;
}

AVehicle* URiderAbility::FindTargetVehicle() const
{
	if(GetAvatarActorFromActorInfo())
	{
		if(ACharacter* AvatarRider = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			if(UVehicleRiderComponent* RiderComponent = AvatarRider->FindComponentByClass<UVehicleRiderComponent>())
			{
				return RiderComponent->GetRidingVehicle();
			}
		}
	}
	return nullptr;
}

