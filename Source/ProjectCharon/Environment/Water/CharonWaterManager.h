// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharonWaterManager.generated.h"


/*
 * 워터 플러그인 보조용 매니저 클래스.
 * 물 정보 텍스처(WaterInfoTexture)를 수집한 뒤 가공해서 카론 워터 바디의 MID에 주입.
 * 이는 거품 등을 만드는 데 사용.
 */

class UTextureRenderTarget2DArray;
class AWaterZone;

UCLASS()
class PROJECTCHARON_API ACharonWaterManager : public AActor
{
	GENERATED_BODY()

public:
	
	ACharonWaterManager();

	UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* GetWaterXYZVelocityTexture(int WaterZoneIndex);
protected:

	
	virtual void BeginPlay() override;

	// UFUNCTION(BlueprintCallable)
	// void PutUpdatedParameters();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<int, UTextureRenderTarget2DArray*> WaterInfoTextures;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<int, UTextureRenderTarget2D*> WaterXYZVelocityTextures;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<int, UTextureRenderTarget2D*> WaterVelocityChangeTextures;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<int, UTextureRenderTarget2D*> WaterSpeedChangeTextures;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<int, UTextureRenderTarget2D*> WaterOutlineTextures;
	
#if WITH_EDITOR
	
	//에디터에서 임의로 워터 텍스처 업데이트 명령.
	UFUNCTION(CallInEditor)
	void UpdateWaterTexturesInEditor();
	
#endif

	// WaterInfoTexture를 모두 수집. 에셋이 로드되지 않았고 로딩중도 아니라면 에셋 로드 명령.
	// 모두 수집했으면 true, 아니면 false 리턴.
	// TODO : 나중엔 게임 시작할때마다 텍스처를 수집하고 가공하는게 아닌, 에디터에서 뭔가 바뀔 때마다 가공 후 에셋 형태로 저장하기. 
	UFUNCTION(BlueprintCallable)
	bool CollectWaterInfoTextures();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UpdateWaterTextures();
	
	// UFUNCTION(BlueprintCallable)
	// void InitWaterInfoTextures();
	
	UFUNCTION(BlueprintCallable)
	void MarkObjectDirty(AActor* Actor);
	
	TSet<TSoftObjectPtr<AWaterZone*>> WaterZones; 
public:

	
	
	
	
};
