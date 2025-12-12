// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleGameStateComponent.generated.h"


class ACharonDock;
class AVehicle;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UVehicleGameStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UVehicleGameStateComponent();

	bool CantRentThisVehicle(int32 CrewId, TSubclassOf<AVehicle> VehicleClass);
	
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<ACharonDock*> Docks;
	
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, AVehicle*> CrewVehicles;
public:
	
};
