// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharonWaterManager.generated.h"


/*
 * 워터 플러그인 보조용 매니저 클래스
 */

UCLASS()
class PROJECTCHARON_API ACharonWaterManager : public AActor
{
	GENERATED_BODY()

public:
	
	ACharonWaterManager();

protected:

	
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	TMap<int, UTextureRenderTarget2D*> WaterXYZVelocityTexture;

#if WITH_EDITOR	
	virtual void PostEditMove(bool bFinished) override;
#endif
	
public:

	UPROPERTY(EditAnywhere)
	int TestNumber;
	
	
};
