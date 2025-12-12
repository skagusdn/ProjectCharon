// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/VehicleRiderComponent.h"

#include "AbilitySystemComponent.h"
#include "Logging.h"
#include "Abilities/GameplayAbility.h"
#include "Character/CharonCharacter.h"
#include "GameFramework/Character.h"
#include "UI/AttributeBoundWidget.h"
#include "Vehicle/Vehicle.h"

// Sets default values for this component's properties
UVehicleRiderComponent::UVehicleRiderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	RidingVehicle = nullptr;
	OwnerCharacter = nullptr;
}


// void UVehicleRiderComponent::BeginPlay()
// {
// 	Super::BeginPlay();
// 	
//
// }

void UVehicleRiderComponent::OnRegister()
{
	Super::OnRegister();
	AActor* Owner = GetOwner();
	
	OwnerCharacter = Cast<ACharacter>(Owner);
	ensureAlwaysMsgf((OwnerCharacter != nullptr), TEXT("VehicleRiderComponent on [%s] can only be added to Character actors."), *GetNameSafe(GetOwner()));

	if(const USkeletalMeshComponent* CharacterMesh =  OwnerCharacter->GetMesh())
	{
		OriginalRiderAnimClass = CharacterMesh->AnimClass;
	}
	
}

void UVehicleRiderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVehicleRiderComponent, RidingVehicle);
}

UVehicleRiderComponent* UVehicleRiderComponent::FindRiderComponent(ACharacter* Character)
{
	check(Character);
	return Character->FindComponentByClass<UVehicleRiderComponent>();
}

void UVehicleRiderComponent::SetRidingVehicle(AVehicle* VehicleToRide)
{
	if(GetOwner()->HasAuthority())
	{
		AVehicle* OldVehicle = RidingVehicle;
		RidingVehicle = VehicleToRide;
		OnRep_RidingVehicle(OldVehicle);
	}

	
}

void UVehicleRiderComponent::OnRep_RidingVehicle(AVehicle* OldVehicle)
{
	if(RidingVehicle != nullptr)
	{
		bIsRidingVehicle = true;

		if(OwnerCharacter && OwnerCharacter->GetMesh())
		{
			OwnerCharacter->GetMesh()->SetHiddenInGame(true);
		}

		// 서버인지 권한체크해야되나?
		if (GetOwner()->HasAuthority())
		{
			const int RiderIdx = RidingVehicle->FindRiderIdx(OwnerCharacter);
			if (RiderIdx >= 0)
			{
				if (ACharonCharacter* CharonRider = Cast<ACharonCharacter>(OwnerCharacter))
				{
					FRiderSpecData RiderSpecData = RidingVehicle->GetRiderSpecData(RiderIdx);
					CharonRider->SwitchAbilityConfig(RiderSpecData.AbilityConfig, RiderSpecData.InputFunctionSet);
				}
			}
			else
			{
				UE_LOG(LogCharon, Error, TEXT("UVehicleRiderComponent::OnRep_RidingVehicle Error"));
			}
		}
	}
	else
	{
		bIsRidingVehicle = false;
		// 애니메이션 원상 복구
		OwnerCharacter->GetMesh()->SetAnimInstanceClass(OriginalRiderAnimClass);
		if(OwnerCharacter && OwnerCharacter->GetMesh())
		{
			OwnerCharacter->GetMesh()->SetHiddenInGame(false);
		}
		if(ACharonCharacter* CharonRider = Cast<ACharonCharacter>(OwnerCharacter))
		{
			CharonRider->ResetAbilityConfig();
		}
	}
}





