// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleGameStateComponent.h"

#include "Logging.h"
#include "GameFramework/GameStateBase.h"



UVehicleGameStateComponent::UVehicleGameStateComponent()
{
}

bool UVehicleGameStateComponent::CantRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass)
{
	//임시
	return true;
}

void UVehicleGameStateComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	
	if (!Owner || !Owner->IsA(AGameStateBase::StaticClass()))
	{
		UE_LOG(LogCharon, Warning, TEXT("UMyRestrictedComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
 
		// 컴포넌트 비활성화: Tick 및 기능 차단
		ensureAlwaysMsgf((false), TEXT("UMyRestrictedComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
		SetActive(false);
	}
}

void UVehicleGameStateComponent::BeginPlay()
{
	Super::BeginPlay();

	for(TIterator<ACharonDock> DockIterator; DockIterator; ++DockIterator)
	{
		Docks.Add(DockIterator);
	}
}


