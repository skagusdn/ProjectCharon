// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleManagerSubsystem.h"

//#include "CollectionManagerTypes.h"
#include "Logging.h"
//#include "GameFramework/Character.h"
//#include "GameFramework/GameModeBase.h"
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



USkeletalMeshComponent* UVehicleManagerSubsystem::RentRiderMesh( AActor* Renter, APlayerState* PlayerState, const USkeletalMeshComponent* SourceMeshCompForCheck)
{
	if(!Renter)
	{
		UE_LOG(LogCharon, Warning, TEXT("RentRiderMesh: Renter actor is invalid"));
		return nullptr;
	}
	
	USkeletalMeshComponent** RiderMeshPtr = RiderMeshes.Find(PlayerState);
	// USkeletalMeshComponent** RiderMeshPtr = RiderMeshes.Find(PlayerState);
	//
	// if (!RiderMeshPtr || !(*RiderMeshPtr))
	// {
	// 	UE_LOG(LogCharon, Error, TEXT("RentRiderMesh: Cant Find RiderMesh of this Player"));
	// 	return nullptr;
	// }
	//
	// USkeletalMeshComponent* RiderMesh = *RiderMeshPtr;
	
	// 메시 에셋이 다를 경우
	// (게임 시작 후 바로 렌트를 시도할 경우 메시가 아직 리플리케이트 되지 않는 경우 발견)
	// 강제로 업데이트. 
	if (SourceMeshCompForCheck)
	{
		if (USkeletalMesh* SourceMesh = SourceMeshCompForCheck->GetSkeletalMeshAsset())
		{
			if (!RiderMeshPtr || SourceMesh != (*RiderMeshPtr)->GetSkeletalMeshAsset())
			{
				UpdateRiderMesh(PlayerState, SourceMeshCompForCheck);
				RiderMeshPtr = RiderMeshes.Find(PlayerState);
				
			}
		}
	}
	
	if(!RiderMeshes.Contains(PlayerState))
	{
		UE_LOG(LogCharon, Warning, TEXT("RentRiderMesh: There is no Rider Mesh matching with this player"));
		return nullptr;
	}
	
	if (!RiderMeshPtr)
	{
		UE_LOG(LogCharon, Error, TEXT("RentRiderMesh: Cant Find RiderMesh of this Player, Update Failed"));
		return nullptr;
	}
	
	USkeletalMeshComponent* RiderMesh = *RiderMeshPtr;

	const FRentKey RentKey = {Renter, PlayerState};
	
	// 이미 빌려준 메시인지 체크. 
	if (LentRiderMeshes.Contains(RentKey))
	{
		UE_LOG(LogCharon, Error, TEXT("RentRiderMesh: RiderMesh had been lent already. Something is wrong"));
		return nullptr;
	}
	
	// for(const TTuple<AActor*, USkeletalMeshComponent*> Tuple : LentRiderMeshes)
	// {
	// 	if(Tuple.Value == RiderMesh)
	// 	{
	// 		UE_LOG(LogCharon, Error, TEXT("RentRiderMesh: RiderMesh had been lent already. Something is wrong"));
	// 		//ReturnRentedRiderMesh(Tuple.Key);
	// 		return nullptr;
	// 	}
	// }
	
	
	
	if(!RiderMesh->IsRegistered())
	{
		RiderMesh->RegisterComponent();
	}
	RiderMesh->SetVisibility(true);
	RiderMesh->SetComponentTickEnabled(true);
	// 콜리전 세팅은 대여자가 알아서 하라고 하고.
	
	//LentRiderMeshes.Add(Renter, RiderMesh);
	LentRiderMeshes.Add(RentKey, RiderMesh);
	Renter->OnDestroyed.AddDynamic(this, &ThisClass::ReturnAllMeshOfRentor);
	return RiderMesh;
}

void UVehicleManagerSubsystem::ReturnRentedRiderMesh(AActor* Renter, APlayerState* PlayerState)
{
	if(!Renter)
	{
		UE_LOG(LogCharon, Warning, TEXT("ReturnRentedRiderMesh: Renter is invalid"));
		return;
	}

	// FRentMeshData* FoundData = LentRiderMeshes.FindByPredicate([Renter, PlayerState](const FRentMeshData& Data)
	// {
	// 	return Data.Renter == Renter && Data.PlayerState == PlayerState;
	// });
	//
	// if (FoundData == nullptr)
	// {
	// 	UE_LOG(LogCharon, Warning, TEXT("ReturnRentedRiderMesh: This Renter didn't rent RiderMesh"));
	// 	return;
	// }
	
	
	FRentKey RentKey = {Renter, PlayerState};
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
		ReturnRentedRiderMesh(RentKey.Renter, RentKey.PlayerState);
	}
	
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


