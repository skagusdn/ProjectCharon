// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/RiderComponent.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Character/CharonCharacter.h"
#include "GameFramework/Character.h"
#include "Vehicle/Vehicle.h"

// Sets default values for this component's properties
URiderComponent::URiderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	RidingVehicle = nullptr;
	OwnerCharacter = nullptr;
}


void URiderComponent::BeginPlay()
{
	Super::BeginPlay();
	

}

void URiderComponent::OnRegister()
{
	Super::OnRegister();
	AActor* Owner = GetOwner();
	
	OwnerCharacter = Cast<ACharacter>(Owner);
}

void URiderComponent::HandleRide(AVehicle* Vehicle, UCharacterAbilityConfig* AbilityConfig,
	ANewInputFunctionSet* InputFunctions)
{
	check(Vehicle);

	RidingVehicle = Vehicle;
	if(ACharonCharacter* CharonCharacter = Cast<ACharonCharacter>(OwnerCharacter))
	{
		if(AbilityConfig)
		{
			CharonCharacter->SwitchAbilityConfig(AbilityConfig, InputFunctions);	
		}
	}
}

void URiderComponent::HandleUnride()
{
	if(ACharonCharacter* CharonCharacter = Cast<ACharonCharacter>(OwnerCharacter))
	{
		CharonCharacter->ResetAbilityConfig();
	}

	RidingVehicle = nullptr;
}


URiderComponent* URiderComponent::FindRiderComponent(ACharacter* Character)
{
	return Character->FindComponentByClass<URiderComponent>();
}





