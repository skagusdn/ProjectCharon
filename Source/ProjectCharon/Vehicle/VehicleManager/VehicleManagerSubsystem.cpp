// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleManagerSubsystem.h"

#include "CollectionManagerTypes.h"
#include "Logging.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"

void UVehicleManagerSubsystem::UpdateRiderMesh(APlayerState* PlayerState, const USkeletalMeshComponent* SourceMeshComp)
{
	check(PlayerState);
	check(SourceMeshComp);
	
	USkeletalMesh* SourceMesh = SourceMeshComp->GetSkeletalMeshAsset();

	if(!SourceMesh)
	{
		UE_LOG(LogCharon, Warning, TEXT("UpdatePlayerCharacterMesh: Mesh has no skeletal mesh asset"));
		return;
	}

	if(!MeshTempContainer)
	{
		MeshTempContainer = GetWorld()->SpawnActor(AActor::StaticClass());
		MeshTempContainer->Rename(TEXT("MeshTempContainer"));
	}
	
	if(USkeletalMeshComponent* NewComponent = NewObject<USkeletalMeshComponent>(MeshTempContainer))
	{
		NewComponent->SetSkeletalMesh(SourceMesh);
		NewComponent->SetVisibility(false);
		NewComponent->SetComponentTickEnabled(false);
		NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		RiderMeshes.Add(PlayerState, NewComponent);
	}
	
}



USkeletalMeshComponent* UVehicleManagerSubsystem::RentRiderMesh(APlayerState* PlayerState, AActor* Renter)
{
	if(!RiderMeshes.Contains(PlayerState))
	{
		UE_LOG(LogCharon, Warning, TEXT("RentRiderMesh: There is no Rider Mesh matching with this player"));
		return nullptr;
	}

	if(!Renter)
	{
		UE_LOG(LogCharon, Warning, TEXT("RentRiderMesh: Renter actor is invalid"));
		return nullptr;
	}
	
	USkeletalMeshComponent* RiderMesh = *RiderMeshes.Find(PlayerState);

	if(!RiderMesh)
	{
		UE_LOG(LogCharon, Error, TEXT("RentRiderMesh: RiderMesh somehow disappeared. check it"));
		return nullptr;
	}

	// 이미 빌려준 메시인지 체크. 
	for(const TTuple<AActor*, USkeletalMeshComponent*> Tuple : LentRiderMeshes)
	{
		if(Tuple.Value == RiderMesh)
		{
			UE_LOG(LogCharon, Warning, TEXT("RentRiderMesh: RiderMesh had been lent already. Something is wrong"));
			ReturnRentedRiderMesh(Tuple.Key);
		}
	}
	
	if(!RiderMesh->IsRegistered())
	{
		RiderMesh->RegisterComponent();
	}
	RiderMesh->SetVisibility(true);
	RiderMesh->SetComponentTickEnabled(true);
	// 콜리전 세팅은 대여자가 알아서 하라고 하고.
	
	LentRiderMeshes.Add(Renter, RiderMesh);
	Renter->OnDestroyed.AddDynamic(this, &ThisClass::ReturnRentedRiderMesh);
	return RiderMesh;
}

void UVehicleManagerSubsystem::ReturnRentedRiderMesh(AActor* Renter)
{
	if(!Renter)
	{
		UE_LOG(LogCharon, Warning, TEXT("ReturnRentedRiderMesh: Renter is invalid"));
		return;
	}

	if(!LentRiderMeshes.Contains(Renter))
	{
		UE_LOG(LogCharon, Warning, TEXT("ReturnRentedRiderMesh: This Renter didn't rent RiderMesh"));
		return;
	}

	USkeletalMeshComponent* RentedRiderMesh = *LentRiderMeshes.Find(Renter); 

	RentedRiderMesh->SetVisibility(false);
	RentedRiderMesh->SetComponentTickEnabled(false);
	RentedRiderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RentedRiderMesh->SetLeaderPoseComponent(nullptr);
	
	RentedRiderMesh->AttachToComponent(MeshTempContainer->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	LentRiderMeshes.Remove(Renter);
	Renter->OnDestroyed.RemoveAll(this);
	
}

// void UVehicleManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
// {
// 	Super::OnWorldBeginPlay(InWorld);
//
// 	if(GIsServer)
// 	{
// 		VehicleManager = GetWorld()->SpawnActor<AVehicleManager>(AVehicleManager::StaticClass());	
// 	}
// }


