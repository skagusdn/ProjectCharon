// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleManagerSubsystem.h"

//#include "CollectionManagerTypes.h"
#include "Logging.h"
//#include "GameFramework/Character.h"
//#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"

void UVehicleManagerSubsystem::UpdateRiderMesh(ACharacter* Rider, USkeletalMesh* CharacterMesh)
{
	
	if(!MeshTempContainer)
	{
		MeshTempContainer = GetWorld()->SpawnActor(AActor::StaticClass());
		MeshTempContainer->Rename(TEXT("MeshTempContainer"));
	}
	
	if (!RiderMeshes.Contains(Rider))
	{
		if(USkeletalMeshComponent* NewComponent = NewObject<USkeletalMeshComponent>(MeshTempContainer))
		{
			NewComponent->SetVisibility(false);
			NewComponent->SetComponentTickEnabled(false);
			NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			RiderMeshes.Add(Rider, NewComponent);
		}
	}
	
	USkeletalMeshComponent* RiderMeshComponent = *RiderMeshes.Find(Rider);
	RiderMeshComponent->SetSkeletalMeshAsset(CharacterMesh);
	
}



USkeletalMeshComponent* UVehicleManagerSubsystem::RentRiderMesh( AActor* Renter, ACharacter* Rider)
{
	if(!Renter)
	{
		UE_LOG(LogCharon, Warning, TEXT("RentRiderMesh: Renter actor is invalid"));
		return nullptr;
	}
	
	//USkeletalMeshComponent** RiderMeshPtr = RiderMeshes.Find(PlayerState);
	
	// // 메시 에셋이 다를 경우
	// // (게임 시작 후 바로 렌트를 시도할 경우 메시가 아직 리플리케이트 되지 않는 경우 발견)
	// // 강제로 업데이트. 
	// if (SourceMeshCompForCheck)
	// {
	// 	if (USkeletalMesh* SourceMesh = SourceMeshCompForCheck->GetSkeletalMeshAsset())
	// 	{
	// 		if (!RiderMeshPtr || SourceMesh != (*RiderMeshPtr)->GetSkeletalMeshAsset())
	// 		{
	// 			UpdateRiderMesh(PlayerState, SourceMeshCompForCheck);
	// 			RiderMeshPtr = RiderMeshes.Find(PlayerState);
	// 			
	// 		}
	// 	}
	// }
	
	if(!RiderMeshes.Contains(Rider))
	{
		UpdateRiderMesh(Rider, Rider->GetMesh()->GetSkeletalMeshAsset());
	}
	
	check(RiderMeshes.Find(Rider));
	
	USkeletalMeshComponent* RiderMesh = *RiderMeshes.Find(Rider);

	const FRentKey RentKey = {Renter, Rider};
	
	// 이미 빌려준 메시인지 체크. 
	if (LentRiderMeshes.Contains(RentKey))
	{
		UE_LOG(LogCharon, Error, TEXT("RentRiderMesh: RiderMesh had been lent already. Something is wrong"));
		return nullptr;
	}
	
	if(!RiderMesh->IsRegistered())
	{
		RiderMesh->RegisterComponent();
	}
	RiderMesh->SetVisibility(true);
	RiderMesh->SetComponentTickEnabled(true);
	// 콜리전 세팅은 대여자가 알아서 하라고 하고.
	
	//LentRiderMeshes.Add(Renter, RiderMesh);
	// TODO : LentRiderMeshes 없애고 그냥 한곳에서 관리해. 이게 뭐여. 
	LentRiderMeshes.Add(RentKey, RiderMesh);
	Renter->OnDestroyed.AddDynamic(this, &ThisClass::ReturnAllMeshOfRentor);
	// 라이더가 파괴되는 경우 알아서 베히클에서 내리게 하겟지 뭐
	return RiderMesh;
}

void UVehicleManagerSubsystem::ReturnRentedRiderMesh(AActor* Renter, ACharacter* Rider)
{
	if(!Renter)
	{
		UE_LOG(LogCharon, Warning, TEXT("ReturnRentedRiderMesh: Renter is invalid"));
		return;
	}
	
	
	FRentKey RentKey = {Renter, Rider};
	if(!LentRiderMeshes.Contains(RentKey))
	{
		UE_LOG(LogCharon, Warning, TEXT("ReturnRentedRiderMesh: This Renter didn't rent RiderMesh"));
		return;
	}

	// USkeletalMeshComponent* RentedRiderMesh = *LentRiderMeshes.Find(RentKey); 
	//
	// RentedRiderMesh->SetVisibility(false);
	// RentedRiderMesh->SetComponentTickEnabled(false);
	// RentedRiderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// RentedRiderMesh->SetLeaderPoseComponent(nullptr);
	//
	// RentedRiderMesh->AttachToComponent(MeshTempContainer->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	//
	// LentRiderMeshes.Remove(RentKey);
	//
	// Renter->OnDestroyed.RemoveAll(this);
	
	USkeletalMeshComponent* RentedRiderMesh = *LentRiderMeshes.Find(RentKey); 

	RentedRiderMesh->SetVisibility(false);
	RentedRiderMesh->SetComponentTickEnabled(false);
	RentedRiderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RentedRiderMesh->SetLeaderPoseComponent(nullptr);
	
	RentedRiderMesh->AttachToComponent(MeshTempContainer->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	LentRiderMeshes.Remove(RentKey);
	
	Renter->OnDestroyed.RemoveAll(this);
	
}

void UVehicleManagerSubsystem::ReturnAllMeshOfRentor(AActor* Renter)
{
	TArray<FRentKey> Keys;
	
	for (TTuple<FRentKey, USkeletalMeshComponent*> &Tuple : LentRiderMeshes)
	{
		if(Tuple.Key.Renter == Renter)
		{
			Keys.Add(Tuple.Key);
		}
	}
	
	for (FRentKey RentKey : Keys)
	{
		ReturnRentedRiderMesh(RentKey.Renter, RentKey.Rider);
	}
	
}



