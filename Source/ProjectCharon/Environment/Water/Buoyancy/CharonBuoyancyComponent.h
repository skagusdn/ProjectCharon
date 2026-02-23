// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuoyancyComponent.h"
#include "CharonBuoyancyComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Config = Game, meta = (BlueprintSpawnableComponent))
class PROJECTCHARON_API UCharonBuoyancyComponent : public UBuoyancyComponent
{
	GENERATED_BODY()
	

public :
	
	UFUNCTION(BlueprintCallable)
	void AddPontoons(float Radius, TArray<FVector> Locations);

	/*
	 * 벡터 두개를 양 끝으로 하는 직육면체 공간에서 폰툰 구체를 넣을 위치 배열 계산.
	 * MaxPontoon 값 안 넣으면 알아서 꽉차게.  
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FVector> CalculateBallsLocation(FVector UpLeftFront, FVector DownRightBack, int32 Radius, int MaxPontoonNum = 0);

	UFUNCTION(BlueprintCallable)
	void InitPontoons(FVector UpLeftFront, FVector DownRightBack, int32 Radius, int MaxPontoonNum = 0);

	UFUNCTION(BlueprintCallable)
	double GetPercentOfPontoonsInWater() const;
	
};
