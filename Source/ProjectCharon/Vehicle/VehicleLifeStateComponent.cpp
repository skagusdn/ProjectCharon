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
		TArray<ACharacter*> RidersToExit;
		// 강제적으로 탑승자 내리게하기. RideVehicle 어빌리티 쪽에서 처리하긴하는데 이렇게 중복으로 만들어둬도 되나?
		// 바로 Riders 에서 루프 돌면서 ExitVehicle하면 루프 내에서 Riders를 삭제하는 꼴이라고 경고줌.
		for(TTuple<int32, ACharacter*> Tuple : OwnerVehicle->Riders)
		{
			if(ACharacter* Rider = Tuple.Value)
			{
				RidersToExit.Add(Rider);	
			}
		}

		for(ACharacter* Rider : RidersToExit)
		{
			OwnerVehicle->ExitVehicle(Rider);
		}
	}
	UninitializeFromAbilitySystem();

	Super::EndPlay(EndPlayReason);
}


