// Fill out your copyright notice in the Description page of Project Settings.


#include "TestAdvancedWaterMaker.h"
#include "Engine/TextureRenderTarget2DArray.h"
#include "NiagaraComponent.h"
#include "WaterBodyActor.h"
#include "WaterBodyComponent.h"


// Sets default values
ATestAdvancedWaterMaker::ATestAdvancedWaterMaker()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WaterSimSystem = nullptr;
}

void ATestAdvancedWaterMaker::Rebuild()
{
	if(!TargetWaterBody.IsValid())
	{
		return;
	}

	if(!TestSimWaterMaterial)
	{
		return;
	}


	if(TObjectPtr<UWaterBodyComponent> WaterBodyComp = TargetWaterBody->GetWaterBodyComponent())
	{
		WaterBodyComp->SetWaterMaterial(TestSimWaterMaterial);
	}

	if(WaterSimSystem && TargetWaterBody)
	{

		if(AWaterZone* WaterZone =  TargetWaterBody->GetWaterBodyComponent()->GetWaterZone())
		{
			if(UTextureRenderTarget2DArray* WaterInfoTextureArray =  WaterZone->WaterInfoTextureArray)
			{
				if(UTexture* WITTextureArray = Cast<UTexture>(WaterInfoTextureArray))
				{
					WaterSimSystem->SetVariableTexture("WaterHeightTexture", WITTextureArray);
				}
			}
		}
		
		
		
		WaterSimSystem->ActivateSystem();
	}

	TestPostRebuild();
	
}

