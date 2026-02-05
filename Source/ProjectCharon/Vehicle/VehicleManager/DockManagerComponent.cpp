// Fill out your copyright notice in the Description page of Project Settings.


#include "DockManagerComponent.h"

#include "Logging.h"
#include "Animation/MovieScene2DTransformSection.h"
#include "Chaos/PBDSuspensionConstraintData.h"
#include "Vehicle/Dock/CharonDock.h"
#include "GameFramework/GameStateBase.h"
#include "EngineUtils.h"
#include "Framework/CrewManagerComponent.h"
#include "Vehicle/Vehicle.h"


UDockManagerComponent::UDockManagerComponent()
{
}

bool UDockManagerComponent::CanRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass)
{
	//임시
	return true;
}

AVehicle* UDockManagerComponent::RentVehicle(const int32 CrewId, TSubclassOf<AVehicle> VehicleClass, const FVector Location, const FRotator Rotation)
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

void UDockManagerComponent::ReturnRentalVehicle(const int32 CrewId)
{
	if(!GetOwner()->HasAuthority())
	{
		return;
	}
	
	if(AVehicle* RentalVehicle = FindCrewVehicle(CrewId))
	{
		
	}
}

AVehicle* UDockManagerComponent::FindCrewVehicle(int32 CrewId)
{
	if(CrewVehicles.Contains(CrewId))
	{
		return CrewVehicles[CrewId];
	}
	return nullptr;
}

void UDockManagerComponent::UpdateDockAccess(int32 CrewId, ACharonDock* Dock, bool NewAccess)
{
	if(!GetOwner()->HasAuthority())
	{
		UE_LOG(LogCharon, Warning, TEXT("UDockManagerComponent::UpdateDockAccess - Need Authority"));
		return;
	}

	if(!Dock || !Docks.Contains(Dock))
	{
		UE_LOG(LogCharon, Warning, TEXT("UDockManagerComponent::UpdateDockAccess - Dock is not valid or not registered."));
		return;
	}
	
	if(AGameStateBase* AGameState = GetWorld()->GetGameState())
	{
		if(UCrewManagerComponent* CrewManager = AGameState->FindComponentByClass<UCrewManagerComponent>())
		{
			CrewManager->UpdateAccessibleDockInfo(CrewId, Dock, NewAccess);
		}
	}

	// // 일단 임시, 플레이어 스타트 포인트 갱신. 
	// if(NewAccess)
	// {
	// 	UpdateCrewStartDock(CrewId, Dock);	
	// }
	
}

void UDockManagerComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	
	if (!Owner || !Owner->IsA(AGameStateBase::StaticClass()))
	{
		UE_LOG(LogCharon, Warning, TEXT("UDockManagerComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
 
		// 컴포넌트 비활성화: Tick 및 기능 차단
		ensureAlwaysMsgf((false), TEXT("UDockManagerComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
		SetActive(false);
	}
}

void UDockManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	for(const auto& Dock : TActorRange<ACharonDock>(GetWorld()) )
	{
		Docks.Add(Dock);
	}
	
}

void UDockManagerComponent::Server_UpdateCrewVehicles()
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

void UDockManagerComponent::Client_UpdateCrewVehicles_Implementation(const TArray<FVehicleEntry>&  VehicleEntries)
{
	// 클라이언트에서 TMap 갱신
	CrewVehicles.Empty();
	for (const auto& [CrewId, Vehicle] : VehicleEntries)
	{
		CrewVehicles.Add(CrewId, Vehicle);
	}
}


