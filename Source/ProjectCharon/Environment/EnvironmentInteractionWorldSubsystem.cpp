// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentInteractionWorldSubsystem.h"

#include "EnvironmentDeveloperSettings.h"
#include "Water/CharonWaterManager.h"
#include "Water/WaterFluidSimulator/WaterFluidSimulator.h"

void UEnvironmentInteractionWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
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

void UEnvironmentInteractionWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
			if(EnvironmentInteractionSettings->CharonWaterManagerClass)
			{
				CharonWaterManagerClass = EnvironmentInteractionSettings->CharonWaterManagerClass;
			}
		}
	}

	if(UWorld* World = GetWorld())
	{
		if(WaterFluidSimulatorClass)
		{
			SpawnWaterFluidSimulator();
		}

		if(CharonWaterManagerClass)
		{
			
			CharonWaterManager = World->SpawnActor<ACharonWaterManager>(CharonWaterManagerClass);
			if(CharonWaterManager)
			{
				UE_LOG(LogTemp, Warning, TEXT("Charon Water Manager Spawned"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Charon Water Manager Spawn Failed"));
			}
		}
	}
	
	
}

void UEnvironmentInteractionWorldSubsystem::SpawnWaterFluidSimulator()
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
