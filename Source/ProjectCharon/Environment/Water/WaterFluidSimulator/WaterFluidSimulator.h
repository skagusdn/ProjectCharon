// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterFluidSimulator.generated.h"


class UWaterInteractiveComponent;
/*
 * 플레이어를 따라다니며 물의 물결을 그려내는 Grid
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class PROJECTCHARON_API AWaterFluidSimulator : public AActor
{
	GENERATED_BODY()

public:
	
	AWaterFluidSimulator();

	UFUNCTION(BlueprintCallable)
	void RegisterTarget(UWaterInteractiveComponent* Target);
	UFUNCTION(BlueprintCallable)
	void UnregisterTarget(USceneComponent* Target);

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// bool bDrawDebugGrid;
	//
	// UPROPERTY()
	// TObjectPtr<UStaticMeshComponent> DebugGridMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bFollowPlayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double GridWidth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 GridResolution;

	
	
	
protected:
	
	virtual void BeginPlay() override;

	// /*
	//  * 주어진 시간이 남았으면 true, 아니면 false.
	//  * FPS에 영향 받지 않고 고정된 시간만큼 틱을 반복하기 위한 함수
	//  */
	// UFUNCTION(BlueprintCallable)
	// bool ProcessTimeStep(float DeltaTime);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double UpdateRate = 60.0f;
	UPROPERTY(BlueprintReadWrite)
	double TimeStacked = 0.0f; 

	
	//void SimulateRippleHeight();
	UPROPERTY(BlueprintReadWrite)
	int HeightState ;

	UFUNCTION(BlueprintCallable)
	void InitRTs();
	UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* GetHeightRT(int PrevIdx = 0);
	UPROPERTY(BlueprintReadWrite)
	TArray<UTextureRenderTarget2D*> HeightRTs;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UTextureRenderTarget2D> HeightNormalRT;
	// 다른 워터바디에 높이값을 전달할 렌더타깃. 
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UTextureRenderTarget2D> PresentHeightRT;

	
	UPROPERTY(BlueprintReadWrite)
	TMap<int32 , UWaterInteractiveComponent*> TouchingSurfaceTargets;
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, FVector> TargetsPrevLocation;
	
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APawn> PlayerToFollow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> PaintBrushMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> HeightCalculationMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> HeightNormalCalculationMaterial;
public:
	
	virtual void Tick(float DeltaTime) override;
};
