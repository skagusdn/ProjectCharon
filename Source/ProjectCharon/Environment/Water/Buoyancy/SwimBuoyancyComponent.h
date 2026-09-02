// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonBuoyancyComponent.h"
#include "SwimBuoyancyComponent.generated.h"


class UBoxComponent;
class USphereComponent;
/*
 * 무브먼트 컴포넌트와 연계해 수영 관련 기능을 구현할 컴포넌트.
 * 부력 컴포넌트에서 현재 물 깊이 등을 얻기 위해 상속.
 * 추후 몬스터 등에서 수영을 구현한다면 기능 확장할 수 있음.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API USwimBuoyancyComponent : public UCharonBuoyancyComponent
{
	GENERATED_BODY()

public:

	USwimBuoyancyComponent(const FObjectInitializer& ObjectInitializer);

protected:
	
	virtual void BeginPlay() override;

	bool InitCheckPontoons();
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable)
	void HandleDebugPontoonEnteredWater(const FSphericalPontoon& Pontoon);

	UFUNCTION(BlueprintCallable)
	void HandleDebugPontoonExitedWater(const FSphericalPontoon& Pontoon);

	

public:
	UFUNCTION(BlueprintCallable)
	bool GetIsSwimming() const;
	/* OwnerActor가 얼마나 물에 잠겨있는지 리턴. 1이면 물 안, 0이면 물 밖.*/
	UFUNCTION(BlueprintCallable)
	float GetImmersionDepth();
	
	// 아마 임시?
	float GetWaterHeightAtLocation(const FVector& WorldLocation);
	
	/*  물에 잠겨있는 깊이 체크용 폰툰 인덱스. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WaterCheckPontoonIndex;
	/* 수영 시작 폰툰 인덱스, 이 폰툰이 물에 닿으면 수영 모드로 바꾸기 -> 구현은 각 액터 클래스에서 알아서. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StartSwimPontoonIndex;
	/* 수영 끝 폰툰 인덱스. 이 폰툰과 수영 시작 폰툰 모두 물에서 빠져 나가면 수영 모드 종료. -> 역시 구현은 각 액터에서 알아서. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 EndSwimPontoonIndex;
	
	
	// 디버그 용 폰튼 구체를 그릴건지
	UPROPERTY(EditDefaultsOnly)
	bool bDrawDebugPontoonShapes;
	UPROPERTY(VisibleAnywhere)
	TArray<USphereComponent*> DebugSphereComponents;
	UPROPERTY(VisibleAnywhere)
	TArray<UBoxComponent*> DebugAxisBoxComponents;

	UPROPERTY(BlueprintAssignable)
	FOnPontoonEnteredWater OnSwimStarterEnteredWater;
	UPROPERTY(BlueprintAssignable)
	FOnPontoonExitedWater OnSwimEnderExitedWater;

	UFUNCTION()
	void CheckSwimPontoonEnteredWater(const FSphericalPontoon& Pontoon);
	UFUNCTION()
	void CheckSwimPontoonExitedWater(const FSphericalPontoon& Pontoon);

	FSphericalPontoon SwimStarterPontoon;
	FSphericalPontoon SwimEnderPontoon;
	FSphericalPontoon WaterCheckPontoon;
};
