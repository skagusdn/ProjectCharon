// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleAbility.h"

#include "GameFramework/Character.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Vehicle/Vehicle.h"

UVehicleAbility::UVehicleAbility()
{
	bShouldCheckTriggerData = false;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

// void UVehicleAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
// {
// 	Super::OnGiveAbility(ActorInfo, Spec);
//
// 	if(AActor* AvatarActor = GetAvatarActorFromActorInfo())
// 	{
// 		AvatarVehicle = Cast<AVehicle>(AvatarActor);
// 	}
// }

void UVehicleAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if(AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		AvatarVehicle = Cast<AVehicle>(AvatarActor);
	}
	
}

void UVehicleAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	// 필터링.
	if (TriggerEventData && bHasBlueprintActivateFromEvent && bShouldCheckTriggerData)
	{
		// 이벤트 데이터 체크. (호출자가 베히클의 탑승자인지 여부라던가)
		if(!CheckEventData(*TriggerEventData))
		{
			return;
		}
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

// bool UVehicleAbility::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
// 	const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
// {
// 	bool Ret = Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
// 	if(Ret)
// 	{
// 		FVehicleAbilityMessage Message;
// 		Message.VehicleAbilityClass = GetClass();
// 		Message.
// 		
// 		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
// 		MessageSystem.BroadcastMessage()
// 	}
// 	return Ret;
// }

bool UVehicleAbility::CheckEventData_Implementation(const FGameplayEventData& EventData)
{
	if(EventData.Instigator && AvatarVehicle)
	{
		if(const ACharacter* InstigatorCharacter = Cast<ACharacter>(EventData.Instigator))
		{
			int RiderIdx = AvatarVehicle->FindRiderIdx(InstigatorCharacter);
			if(RiderIdx >= 0)
			{
				return true;
			}
		}
	}
	
	return false;
}
