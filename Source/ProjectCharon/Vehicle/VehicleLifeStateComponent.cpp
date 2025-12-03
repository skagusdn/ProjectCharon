// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleLifeStateComponent.h"

#include "Vehicle.h"


UVehicleLifeStateComponent::UVehicleLifeStateComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Super 해놨으니까 상관없나?
	AbilitySystemComponent = nullptr;
	LifeState = ECharonLifeState::NotDead;
}

void UVehicleLifeStateComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	
	AVehicle* Vehicle = Cast<AVehicle>(Owner);
	ensureAlwaysMsgf((Vehicle != nullptr), TEXT("VehicleLifeStateComponent on [%s] can only be added to Vehicle actors."), *GetNameSafe(GetOwner()));

	OwnerVehicle = Vehicle;
}

void UVehicleLifeStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// DeathFinished 상태가 아니라면 정상적으로 액터가 파괴된게 아니라는 뜻. 
	if(LifeState != ECharonLifeState::DeathFinished)
	{
		// 강제적으로 탑승자 내리게하기. 원래는 어빌리티가 먼저 요청했어야 했다.
		// Riders는 nullptr로 초기화되어있음.
		for(TTuple<int32, ACharacter*> Tuple : OwnerVehicle->Riders)
		{
			if(ACharacter* Rider = Tuple.Value)
			{
				OwnerVehicle->ExitVehicle(Rider);	
			}
		}
	}
	UninitializeFromAbilitySystem();

	Super::EndPlay(EndPlayReason);
}


