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

void URiderAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	SetVehicleInfo();

	//TargetVehicleAbilityClass->GetDefaultObject()->;
}

void URiderAbility::SetVehicleInfo()
{
	if(GetAvatarActorFromActorInfo())
	{
		if(ACharacter* AvatarRider = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			if(UVehicleRiderComponent* RiderComponent = AvatarRider->FindComponentByClass<UVehicleRiderComponent>())
			{
				TargetVehicle = RiderComponent->GetRidingVehicle();
			}
		}
	}
}

bool URiderAbility::DoesVehicleImplementsTargetClass() const
{
	return TargetVehicle && TargetVehicle->IsA(TargetVehicleClass);
}

bool URiderAbility::DoesVehicleHasTargetAbility() const
{
	if(TargetVehicle)
	{
		if(UAbilitySystemComponent* TargetASC = TargetVehicle->GetAbilitySystemComponent())
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
