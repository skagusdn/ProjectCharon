// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnvironmentInteractionWorldSubsystem.generated.h"

class AWaterFluidSimulator;
class UEnvironmentDeveloperSettings;
/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UEnvironmentInteractionWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	public :
	// USubsystem 시작
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	//virtual void Deinitialize() override;
	// USubsystem 끝

	// UWorldSubsystem 인터페이스~~
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	// UWorldSubsystem 인터페이스 끝~~
	
	protected :

	void SpawnWaterFluidSimulator();
	UPROPERTY()
	const UEnvironmentDeveloperSettings* EnvironmentInteractionSettings;
	TSubclassOf<AWaterFluidSimulator> WaterFluidSimulatorClass;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AWaterFluidSimulator> WaterFluidSimulator;
};
