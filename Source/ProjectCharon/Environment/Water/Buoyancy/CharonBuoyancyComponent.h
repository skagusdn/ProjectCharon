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
	UCharonBuoyancyComponent(const FObjectInitializer& ObjectInitializer);
	
	// 기존껀 워터바디를 Overlapping 하고 있어야 UpdatePontoons가 제대로 기능함
	// 성능을 좀 포기하고 조건문 없애기. 나중에 여유 있으면 워터바디 근처에 있을 때도 키게 한다거나 그렇게 하면 되겠다.
	virtual int32 UpdatePontoons(float DeltaTime, float ForwardSpeed, float ForwardSpeedKmh, UPrimitiveComponent* PrimitiveComponent) override;
	
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

protected:
	UPROPERTY(EditDefaultsOnly)
	bool bUpdatePontoonsOutOfWater;
};
