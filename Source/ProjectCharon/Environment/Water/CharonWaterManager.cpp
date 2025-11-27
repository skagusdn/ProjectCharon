// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonWaterManager.h"

#include "EngineUtils.h"
#include "WaterBodyComponent.h"
#include "WaterZoneActor.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"


// Sets default values
ACharonWaterManager::ACharonWaterManager()
{
	
	
}

UTextureRenderTarget2D* ACharonWaterManager::GetWaterXYZVelocityTexture(int WaterZoneIndex)
{
	if(WaterXYZVelocityTextures.Find(WaterZoneIndex))
	{
		return WaterXYZVelocityTextures[WaterZoneIndex];
	}
	return nullptr;
}


// Called when the game starts or when spawned
void ACharonWaterManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// void ACharonWaterManager::PutUpdatedParameters()
// {
// 	
// 	for(TActorIterator<AWaterZone> WaterZoneItr(GetWorld()) ; WaterZoneItr; ++WaterZoneItr)
// 	{
// 		AWaterZone* WaterZone = *WaterZoneItr;
// 		for(WaterZone->ForEachWaterBodyComponent([&](UWaterBodyComponent* WaterBodyComponent)-> 
// 		bool {
// 			
// 		}));
// 	}
// }

#if WITH_EDITOR
void ACharonWaterManager::UpdateWaterTexturesInEditor()
{
	UpdateWaterTextures();
}
#endif


bool ACharonWaterManager::CollectWaterInfoTextures()
{
	bool bCollectedAll = true;
	for(TActorIterator<AWaterZone> WaterZoneItr(GetWorld()) ; WaterZoneItr; ++WaterZoneItr)
	{
		TSoftObjectPtr<AWaterZone> WaterZoneSoftPtr = *WaterZoneItr;
		if(WaterZoneSoftPtr.IsValid())
		{
			AWaterZone* WaterZone = WaterZoneSoftPtr.Get();
			WaterInfoTextures.Add(WaterZone->GetWaterZoneIndex(), WaterZone->WaterInfoTextureArray);
		}
		else if(!WaterZoneSoftPtr.IsPending())
		{
			bCollectedAll = false;
			
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			Streamable.RequestAsyncLoad(WaterZoneSoftPtr.ToSoftObjectPath(),
				FStreamableDelegate::CreateLambda([WaterZoneSoftPtr, this]()
			{
				if(WaterZoneSoftPtr.IsValid())
				{
					AWaterZone* WaterZone = WaterZoneSoftPtr.Get();
					WaterInfoTextures.Add(WaterZone->GetWaterZoneIndex(), WaterZone->WaterInfoTextureArray);
				}
			}));
		}
	}

	return bCollectedAll;
	
}


// void ACharonWaterManager::InitWaterInfoTextures()
// {
// 	for(TActorIterator<AWaterZone> WaterZoneItr(GetWorld()) ; WaterZoneItr; ++WaterZoneItr)
// 	{
// 		TSoftObjectPtr<AWaterZone> WaterZoneSoftPtr = *WaterZoneItr;
// 		if(WaterZoneSoftPtr.IsValid())
// 		{
// 			AWaterZone* WaterZone = WaterZoneSoftPtr.Get();
// 			WaterInfoTextures.Add(WaterZone->GetWaterZoneIndex(), WaterZone->WaterInfoTextureArray);
// 		}
// 		// 잉 Pending이 아닐때? 뭔가 코드 잘 못 가져온거 아님? -> 수정해봄
// 		else if(WaterZoneSoftPtr.IsPending())
// 		{
// 			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
// 			Streamable.RequestAsyncLoad(WaterZoneSoftPtr.ToSoftObjectPath(),
// 				FStreamableDelegate::CreateLambda([WaterZoneSoftPtr, this]()
// 			{
// 				if(WaterZoneSoftPtr.IsValid())
// 				{
// 					AWaterZone* WaterZone = WaterZoneSoftPtr.Get();
// 					WaterInfoTextures.Add(WaterZone->GetWaterZoneIndex(), WaterZone->WaterInfoTextureArray);
// 				}
// 			}));
// 		}
// 	}
// }

void ACharonWaterManager::MarkObjectDirty(AActor* Actor)
{
#if WITH_EDITOR
	if(Actor != nullptr)
	{
		Actor->Modify();	
	}
#endif
}




