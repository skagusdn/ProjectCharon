// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/VehicleRiderComponent.h"

#include "AbilitySystemComponent.h"
#include "Logging.h"
#include "Abilities/GameplayAbility.h"
#include "Ability/VehicleAbility.h"
#include "Character/CharonCharacter.h"
#include "GameFramework/Character.h"

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

void UVehicleRiderComponent::Server_HandleVehicleAbilityCommitted(UGameplayAbility* Ability)
{
	if(!GetOwner()->HasAuthority())
	{
		return;
	}

	FAbilityCommitInfo AbilityCommitInfo;
	AbilityCommitInfo.Ability = Ability;
	// float OutRemainingCooldown  = 0.0f;
	// float OutCooldownDuration = 0.0f;
	const FGameplayAbilityActorInfo ActorInfo = Ability->GetActorInfo();
	Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(), &ActorInfo , AbilityCommitInfo.CooldownDuration, AbilityCommitInfo.RemainingCooldown );
	
	
	HandleVehicleAbilityCommit(AbilityCommitInfo);
	Client_HandleVehicleAbilityCommitted(AbilityCommitInfo);
}

void UVehicleRiderComponent::Client_HandleVehicleAbilityCommitted_Implementation(FAbilityCommitInfo AbilityCommitInfo)
{
	if(!GetOwner()->HasAuthority())
	{
		HandleVehicleAbilityCommit(AbilityCommitInfo);
	}
}

void UVehicleRiderComponent::HandleVehicleAbilityCommit(FAbilityCommitInfo AbilityCommitInfo)
{
	OnVehicleAbilityCommitted.Broadcast(AbilityCommitInfo);

	// ///
	// if(GetOwner()->HasAuthority())
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("VehicleAbility Committed TEST SERVER %s %f %f"), *AbilityCommitInfo.Ability.GetName(), AbilityCommitInfo.CooldownDuration, AbilityCommitInfo.RemainingCooldown);
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("VehicleAbility Committed TEST Client %s %f %f"), *AbilityCommitInfo.Ability.GetName(), AbilityCommitInfo.CooldownDuration, AbilityCommitInfo.RemainingCooldown);
	// }
	// ////
}

// void UVehicleRiderComponent::Server_OnVehicleAbilityActivated(UGameplayAbility* Ability)
// {
// 	if(!GetOwner()->HasAuthority())
// 	{
// 		return;
// 	}
// 	
// 	Client_OnVehicleAbilityActivated(Ability);
// }
//
// void UVehicleRiderComponent::Client_OnVehicleAbilityActivated_Implementation(UGameplayAbility* Ability)
// {
// 	OnVehicleAbilityActivated(Ability);
// }

void UVehicleRiderComponent::OnRep_RidingVehicle(AVehicle* OldVehicle)
{
	if(RidingVehicle != nullptr)
	{
		bIsRidingVehicle = true;

		if(OwnerCharacter && OwnerCharacter->GetMesh())
		{
			OwnerCharacter->GetMesh()->SetHiddenInGame(true);
		}
		
		// 서버인지 권한체크해야되나? 이 주석 시점에선 베히클의 Riders 목록이 리플리케이션 안되기 때문에.. 
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

				// 베히클 어빌리티 커밋 콜백 바인드
				if(UAbilitySystemComponent* VehicleASC = RidingVehicle->GetAbilitySystemComponent())
				{
					VehicleASC->AbilityCommittedCallbacks.AddUObject(this, &ThisClass::Server_HandleVehicleAbilityCommitted);
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

		//베히클이 어빌리티 발동시켰을 때 호출되게 한 콜백함수 바인딩 해제
		if(OldVehicle)
		{
			if(UAbilitySystemComponent* VehicleASC = OldVehicle->GetAbilitySystemComponent())
			{
				VehicleASC->AbilityCommittedCallbacks.RemoveAll(this);
			}
		}
		
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

	if(OldVehicle != RidingVehicle)
	{
		OnRidingVehicleChanged.Broadcast(RidingVehicle);	
	}
	
}





