// Fill out your copyright notice in the Description page of Project Settings.


#include "WaterFluidSimulator.h"

#include "WaterInteractiveComponent.h"
#include "WaterSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/TextureRenderTarget2D.h"


// Sets default values
AWaterFluidSimulator::AWaterFluidSimulator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWaterFluidSimulator::RegisterTarget(UWaterInteractiveComponent* Target)
{
	const uint32 ID = Target->GetUniqueID();
	if(TouchingSurfaceTargets.Contains(ID))
	{
		return;
	}
	TouchingSurfaceTargets.Add(ID, Target);
	TargetsPrevLocation.Add(ID, Target->GetComponentLocation());
	TargetsPrevInWater.Add(ID, Target->GetIsInWater());
}

void AWaterFluidSimulator::UnregisterTarget(USceneComponent* Target)
{
	if(!Target)
	{
		return;
	}
	
	const uint32 ID = Target->GetUniqueID();
	if(!TouchingSurfaceTargets.Contains(ID))
	{
		return;
	}
	TouchingSurfaceTargets.Remove(ID);
	TargetsPrevLocation.Remove(ID);
	TargetsPrevInWater.Remove(ID);
	
}

void AWaterFluidSimulator::UpdateOverlappingWaterZone()
{
	
	const FVector Location = GetActorLocation();
	const FBox Bounds(Location - 500.0, Location + 500.0);

	TSoftObjectPtr<AWaterZone> FoundWaterZoneSoftPtr = UWaterSubsystem::FindWaterZone3D(GetWorld(), Bounds, GetLevel());
	if(FoundWaterZoneSoftPtr.IsValid())
	{
		if(OverlappingWaterZone != FoundWaterZoneSoftPtr.Get())
		{
			OverlappingWaterZone = FoundWaterZoneSoftPtr.Get();
			WaterInfoTextureArray = OverlappingWaterZone->WaterInfoTextureArray;
		}
	}
	else if(!FoundWaterZoneSoftPtr.IsPending())
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		Streamable.RequestAsyncLoad(FoundWaterZoneSoftPtr.ToSoftObjectPath(),
			FStreamableDelegate::CreateLambda([FoundWaterZoneSoftPtr, this]()
		{
			if(FoundWaterZoneSoftPtr.IsValid())
			{
				OverlappingWaterZone = FoundWaterZoneSoftPtr.Get();
			}
		}));
	}
}


void AWaterFluidSimulator::BeginPlay()
{
	Super::BeginPlay();
	
}

// bool AWaterFluidSimulator::ProcessTimeStep(const float DeltaTime)
// {
// 	TimeStacked += DeltaTime;
// 	UpdateRate = FMath::Min(UpdateRate, 120);
// 	const double TimeForUpdate = 1 / UpdateRate;
//
// 	if(TimeStacked >= TimeForUpdate)
// 	{
// 		TimeStacked -= TimeForUpdate;
// 		return true;
// 	}
// 	return false;
// }

// void AWaterFluidSimulator::InitRTs()
// {
//
// 	HeightRTs.Empty();
// 	
// 	for(int i = 0; i < 3; i++)
// 	{
// 		UTextureRenderTarget2D* NewRT = NewObject<UTextureRenderTarget2D>();
// 		NewRT->InitAutoFormat(GridResolution, GridResolution);
// 		NewRT->RenderTargetFormat = RTF_RGBA16f;
// 		NewRT->AddressX = TextureAddress::TA_Clamp;
// 		NewRT->AddressY = TextureAddress::TA_Clamp;
// 		NewRT->ClearColor = FColor::Black;
// 		
// 		HeightRTs.Add(NewRT);
// 	}
// }

UTextureRenderTarget2D* AWaterFluidSimulator::GetHeightRT(const int PrevIdx)
{
	const int Idx = (HeightState + 3 - PrevIdx) % 3;
	return HeightRTs[Idx];
}


void AWaterFluidSimulator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

