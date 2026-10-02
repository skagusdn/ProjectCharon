// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaterBodyRiverActor.h"
#include "CharonWaterBodyRiver.generated.h"

class UCharonWaterNavCollisionComponent;
class UNavAreaBase;

UCLASS()
class PROJECTCHARON_API ACharonWaterBodyRiver : public AWaterBodyRiver
{
	GENERATED_BODY()

public:

	ACharonWaterBodyRiver(const FObjectInitializer& ObjectInitializer);


	UFUNCTION(BlueprintCallable)
	AWaterZone* GetOwningWaterZone();

	// Bake 후 눌러서 내비게이션 전용 콜리전을 수면 높이에 맞춰 다시 만든다.
	UFUNCTION(CallInEditor, Category = "Charon|Water|Navigation")
	void RebuildNavCollisionFromBakedSim();

protected:

	virtual void BeginPlay() override;

	// 윗면 정점에서 물을 찾을 최대 반경(cm). 강둑 위 정점이 물을 못 찾는 걸 방지.
	UPROPERTY(EditAnywhere, Category = "Charon|Water|Navigation")
	float NavTopSearchRadius = 300.f;

	// 수면 위로 둘 여유(cm).
	UPROPERTY(EditAnywhere, Category = "Charon|Water|Navigation")
	float NavTopPadding = 20.f;

	// 내비 컨벡스 최소 두께(cm). 너무 얇으면 Recast가 면을 못 잡는다.
	UPROPERTY(EditAnywhere, Category = "Charon|Water|Navigation")
	float NavMinConvexThickness = 50.f;

	// 내비 전용 콜리전에 적용할 영역 클래스. 내비게이션 영역은 여기서만 설정한다.
	// 워터바디의 WaterNavAreaClass는 protected라 읽을 수 없고 워터바디 내비게이션은 꺼지므로,
	// Rebuild 시 이 값을 워터바디에도 써서 두 값을 같게 유지한다.
	UPROPERTY(EditAnywhere, Category = "Charon|Water|Navigation")
	TSubclassOf<UNavAreaBase> NavAreaClass;

	UPROPERTY()
	TObjectPtr<UCharonWaterNavCollisionComponent> NavCollisionComponent;

	// ////////테스트용
	// UPROPERTY(VisibleAnywhere)
	// TObjectPtr<class UCharonWaterNavCollisionComponent> TestComp;
	
public:

};
