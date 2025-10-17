// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharonWaterManager.generated.h"


/*
 * 워터 플러그인 보조용 매니저 클래스
 */

class UTextureRenderTarget2DArray;
class AWaterZone;

UCLASS()
class PROJECTCHARON_API ACharonWaterManager : public AActor
{
	GENERATED_BODY()

public:
	
	ACharonWaterManager();

#if WITH_EDITOR	
	void RegisterWaterZone(AWaterZone *WaterZone);
#endif
protected:

	
	virtual void BeginPlay() override;

	// UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	// TMap<int, UTextureRenderTarget2D*> WaterXYZVelocityTexture;

#if WITH_EDITOR
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWaterZVelocity(UTextureRenderTarget2DArray* WaterInfoTextureArray);
#endif
	
	TSet<TSoftObjectPtr<AWaterZone*>> WaterZones; 
public:

	
	
	
	
};
