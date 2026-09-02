// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VehicleManagerSubsystem.generated.h"

class AVehicleManager;

USTRUCT()
struct FRentKey
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<AActor> Renter;
	UPROPERTY()
	TObjectPtr<ACharacter> Rider;
	// UPROPERTY()
	// TObjectPtr<USkeletalMeshComponent> Mesh;
	
	// bool operator==(const FRentMeshData& Other) const
	// {
	// 	if (Other.Renter == Renter && Other.PlayerState == PlayerState)
	// 	{
	// 		return true;
	// 	}
	// 	
	// 	return false;
	// }
	
	bool operator==(const FRentKey& Other) const
	{
		return Renter == Other.Renter && Rider == Other.Rider;
	}
	
	friend uint32 GetTypeHash(const FRentKey& Key)
	{
		// 두 멤버의 해시 값을 조합하여 고유한 해시 값을 생성합니다.
		// GetTypeHash는 포인터에 대해서도 잘 동작합니다.
		return HashCombine(GetTypeHash(Key.Renter), GetTypeHash(Key.Rider));
	}
};



/**
 *  일단 지금은 베히클 탑승 시 베히클에 부착할 캐릭터 메시만 관리.
 */
UCLASS()
class PROJECTCHARON_API UVehicleManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public :
	void UpdateRiderMesh(ACharacter* Rider, USkeletalMesh* CharacterMesh);
	
	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* RentRiderMesh(AActor* Renter, ACharacter* Rider);
	
	
	//UFUNCTION(BlueprintCallable)
	//void ReturnRentedRiderMesh(USkeletalMeshComponent* RentedRiderMesh);
	UFUNCTION(BlueprintCallable)
	void ReturnRentedRiderMesh(AActor* Renter, ACharacter* Rider);
	UFUNCTION()
	void ReturnAllMeshOfRentor(AActor* Renter);
	// UFUNCTION()
	// void ReturnAllMeshOfPlayer(APlayerState* PlayerState);

	// 생성한 라이더 메시를 임시 보관할 액터. 
	UPROPERTY()
	TObjectPtr<AActor> MeshTempContainer;

	// UPROPERTY(BlueprintReadOnly)
	// TMap<AActor*, USkeletalMeshComponent*> LentRiderMeshes;
	
	
	UPROPERTY()
	TMap<FRentKey , USkeletalMeshComponent*> LentRiderMeshes;
	// UPROPERTY()
	// TArray<FRentMeshData> LentRiderMeshes;
	

protected:
	//virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	UPROPERTY(BlueprintReadOnly)
	//TMap<APlayerController*, USkeletalMeshComponent*> RiderMeshes;
	TMap<ACharacter*, USkeletalMeshComponent*> RiderMeshes;

	// TObjectPtr<AVehicleManager> VehicleManager;

	
	
};
