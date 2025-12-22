// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleGameStateComponent.h"

#include "Logging.h"
#include "Animation/MovieScene2DTransformSection.h"
#include "Chaos/PBDSuspensionConstraintData.h"
#include "Vehicle/Dock/CharonDock.h"
#include "GameFramework/GameStateBase.h"
#include "EngineUtils.h"
#include "Vehicle/Vehicle.h"


UVehicleGameStateComponent::UVehicleGameStateComponent()
{
}

bool UVehicleGameStateComponent::CantRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass)
{
	//임시
	return true;
}

AVehicle* UVehicleGameStateComponent::RentVehicle(const int32 CrewId, TSubclassOf<AVehicle> VehicleClass, const FVector Location, const FRotator Rotation)
{
	if(!GetOwner()->HasAuthority())
	{
		return nullptr;
	}

	if(CrewVehicles.Contains(CrewId))
	{
		if(AVehicle* RentedVehicle = FindCrewVehicle(CrewId))
		{
			RentedVehicle->Destroy();
			CrewVehicles.Remove(CrewId);
			Server_UpdateCrewVehicles();
		}
	}

	
	if(AVehicle* SpawnedVehicle = Cast<AVehicle>(GetWorld()->SpawnActor(VehicleClass, &Location, &Rotation)))
	{
		CrewVehicles.Add(CrewId, SpawnedVehicle);
		Server_UpdateCrewVehicles();
		return SpawnedVehicle;
	}
	
	return nullptr;
	
}

AVehicle* UVehicleGameStateComponent::FindCrewVehicle(int32 CrewId)
{
	if(CrewVehicles.Contains(CrewId))
	{
		return CrewVehicles[CrewId];
	}
	return nullptr;
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

	// for(TObjectIterator<ACharonDock> It(GetWorld()); It; ++It)
	// {
	// 	Docks.Add(*It);
	// }
	for(const auto& Dock : TActorRange<ACharonDock>(GetWorld()) )
	{
		Docks.Add(Dock);
	}
}

void UVehicleGameStateComponent::Server_UpdateCrewVehicles()
{
	if(!GetOwner()->HasAuthority())
	{
		return;
	}

	// TMap을 배열로 변환
	TArray<FVehicleEntry> VehicleEntries;
	for (const TTuple<int32, AVehicle*> Tuple : CrewVehicles)
	{
		FVehicleEntry Entry;
		Entry.CrewId = Tuple.Key;
		Entry.Vehicle = Tuple.Value;
		VehicleEntries.Add(Entry);
	}
 
	// 클라이언트 RPC 호출 (클라이언트마다 실행)
	Client_UpdateCrewVehicles_Implementation(VehicleEntries);
}

void UVehicleGameStateComponent::Client_UpdateCrewVehicles_Implementation(const TArray<FVehicleEntry>&  VehicleEntries)
{
	// 클라이언트에서 TMap 갱신
	CrewVehicles.Empty();
	for (const auto& [CrewId, Vehicle] : VehicleEntries)
	{
		CrewVehicles.Add(CrewId, Vehicle);
	}
}


