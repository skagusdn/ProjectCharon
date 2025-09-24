// Fill out your copyright notice in the Description page of Project Settings.


#include "WaterFluidSimulator.h"

#include "WaterInteractiveComponent.h"
#include "Engine/TextureRenderTarget2D.h"


// Sets default values
AWaterFluidSimulator::AWaterFluidSimulator()
{

	PrimaryActorTick.bCanEverTick = true;

	// DebugGridMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugGridMesh"));
	// if(!bDrawDebugGrid)
	// {
	// 	DebugGridMesh->SetHiddenInGame(true);
	// }
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
}

void AWaterFluidSimulator::UnregisterTarget(USceneComponent* Target)
{
	const uint32 ID = Target->GetUniqueID();
	if(!TouchingSurfaceTargets.Contains(ID))
	{
		return;
	}
	TouchingSurfaceTargets.Remove(ID);
	TargetsPrevLocation.Remove(ID);
	
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

void AWaterFluidSimulator::InitRTs()
{

	HeightRTs.Empty();
	
	for(int i = 0; i < 3; i++)
	{
		UTextureRenderTarget2D* NewRT = NewObject<UTextureRenderTarget2D>();
		NewRT->InitAutoFormat(GridResolution, GridResolution);
		NewRT->RenderTargetFormat = RTF_RGBA16f;
		NewRT->AddressX = TextureAddress::TA_Clamp;
		NewRT->AddressY = TextureAddress::TA_Clamp;
		NewRT->ClearColor = FColor::Black;
		
		HeightRTs.Add(NewRT);
	}
}

UTextureRenderTarget2D* AWaterFluidSimulator::GetHeightRT(const int PrevIdx)
{
	const int Idx = (HeightState + 3 - PrevIdx) % 3;
	return HeightRTs[Idx];
}


void AWaterFluidSimulator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

