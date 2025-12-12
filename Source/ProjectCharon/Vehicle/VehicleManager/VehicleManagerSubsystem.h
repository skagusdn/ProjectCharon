// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VehicleManagerSubsystem.generated.h"

class AVehicleManager;
/**
 *  베히클 관리 
 */
UCLASS()
class PROJECTCHARON_API UVehicleManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public :
	void UpdateRiderMesh(APlayerState* PlayerState, const USkeletalMeshComponent* SourceMeshComp);
	
	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* RentRiderMesh(APlayerState* PlayerState, AActor* Renter);
	
	//UFUNCTION(BlueprintCallable)
	//void ReturnRentedRiderMesh(USkeletalMeshComponent* RentedRiderMesh);
	UFUNCTION(BlueprintCallable)
	void ReturnRentedRiderMesh(AActor* Renter);

	// 생성한 라이더 메시를 임시 보관할 액터. 
	UPROPERTY()
	TObjectPtr<AActor> MeshTempContainer;

	UPROPERTY(BlueprintReadOnly)
	TMap<AActor*, USkeletalMeshComponent*> LentRiderMeshes;

protected:
	//virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	UPROPERTY(BlueprintReadOnly)
	//TMap<APlayerController*, USkeletalMeshComponent*> RiderMeshes;
	TMap<APlayerState*, USkeletalMeshComponent*> RiderMeshes;

	// TObjectPtr<AVehicleManager> VehicleManager;

	
	
};
