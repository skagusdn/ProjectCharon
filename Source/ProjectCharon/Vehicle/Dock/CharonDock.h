// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CharonDock.generated.h"

class UDockManagerComponent;
class AVehicle;

// 나중엔 여기에 뭐 오더 종류라던가 더 복잡한 구조체가 될 수도.
USTRUCT(BlueprintType)
struct FOrderInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<AVehicle> VehicleToRent;
};



UCLASS()
class PROJECTCHARON_API ACharonDock : public AActor
{
	GENERATED_BODY()

public:
	
	ACharonDock();
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	AVehicle* RentVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass, FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ReturnRentalVehicle(int32 CrewId);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ReturnRentalVehicle(AVehicle* VehicleToReturn);

	// 이 크루가 해당 타입의 탈 것을 빌릴 수 있는가? 일단 걍 리턴 true 하게.
	UFUNCTION(BlueprintCallable)
	bool CanRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetCrewAccess(int32 CrewId, bool NewAccess);
	
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, bool> CrewAccessMap;
	
	TObjectPtr<UDockManagerComponent> DockManager;

	bool bIsSpawningVehicle;
public:
	
};
