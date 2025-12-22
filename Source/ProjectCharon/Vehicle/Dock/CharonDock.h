// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vehicle/VehicleManager/VehicleGameStateComponent.h"
#include "CharonDock.generated.h"

class AVehicle;

UCLASS()
class PROJECTCHARON_API ACharonDock : public AActor
{
	GENERATED_BODY()

public:
	
	ACharonDock();

	// UFUNCTION(BlueprintCallable)
	// AVehicle* FindCrewVehicle(int32 CrewId);

	// UFUNCTION(BlueprintCallable)
	// AVehicle* RentVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass, FVector Location);
	UFUNCTION(BlueprintCallable)
	AVehicle* RentVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass, FVector Location, FRotator Rotation);
protected:
	
	virtual void BeginPlay() override;
	
	// 이 크루가 해당 타입의 탈 것을 빌릴 수 있는가? 일단 걍 리턴 true 하게.
	UFUNCTION(BlueprintCallable)
	bool CanRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass);
	
	TObjectPtr<UVehicleGameStateComponent> VehicleGameStateComp;
	
	// // <크루ID, 크루에서 렌탈한 탈것>
	// UPROPERTY(BlueprintReadWrite)
	// TMap<int32, AVehicle*> CrewVehicles;

	bool bIsSpawningVehicle;
public:
	
};
