// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentWorldSubsystem.h"

#include "EnvironmentDeveloperSettings.h"
#include "Water/CharonWaterManager.h"
#include "Water/WaterFluidSimulator/WaterFluidSimulator.h"

void UEnvironmentWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EnvironmentInteractionSettings = GetDefault<UEnvironmentDeveloperSettings>();
	if(EnvironmentInteractionSettings)
	{
		if(EnvironmentInteractionSettings->WaterFluidSimulatorClass)
		{
			WaterFluidSimulatorClass = EnvironmentInteractionSettings->WaterFluidSimulatorClass;
		}
	}
}

void UEnvironmentWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if(!EnvironmentInteractionSettings)
	{
		EnvironmentInteractionSettings = GetDefault<UEnvironmentDeveloperSettings>();
		if(EnvironmentInteractionSettings)
		{
			if(EnvironmentInteractionSettings->WaterFluidSimulatorClass)
			{
				WaterFluidSimulatorClass = EnvironmentInteractionSettings->WaterFluidSimulatorClass;
			}
		}
	}

	if(UWorld* World = GetWorld())
	{
		if(WaterFluidSimulatorClass)
		{
			SpawnWaterFluidSimulator();
		}
	}
	
	
}

void UEnvironmentWorldSubsystem::SpawnWaterFluidSimulator()
{
	if(WaterFluidSimulatorClass && GetWorld()) 
	{
		WaterFluidSimulator = GetWorld()->SpawnActor<AWaterFluidSimulator>(WaterFluidSimulatorClass);
		if(WaterFluidSimulator)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawned WaterFluidSimulator2"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawn Failed WaterFluidSimulator2"));
		}
	}
}
