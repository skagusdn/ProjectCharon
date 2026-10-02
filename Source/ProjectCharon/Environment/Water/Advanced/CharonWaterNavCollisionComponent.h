// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "CharonWaterNavCollisionComponent.generated.h"

class UBodySetup;
class UNavAreaBase;

/*
 * 내비게이션 전용 콜리전 컴포넌트.
 * 물리 충돌에는 전혀 참여하지 않고(NoCollision), 내비메시 생성에만 형상을 제공한다.
 * 강 워터바디의 컨벡스를 복사한 뒤 윗면 정점 높이만 베이크된 수면 높이로 바꿔서 담는다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UCharonWaterNavCollisionComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:

	UCharonWaterNavCollisionComponent(const FObjectInitializer& ObjectInitializer);

	// 내비 형상 교체. 각 원소는 월드 공간 정점 배열(0~3 바닥 / 4~ 윗면).
	void SetNavConvexes(const TArray<TArray<FVector>>& WorldSpaceConvexes);

	void SetNavAreaClass(TSubclassOf<UNavAreaBase> InNavAreaClass) { NavAreaClass = InNavAreaClass; }

	UFUNCTION(BlueprintPure, Category = "Charon|Water|Navigation")
	int32 GetNavConvexCount() const;

	//~ UPrimitiveComponent 시작
	virtual UBodySetup* GetBodySetup() override { return NavBodySetup; }
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual bool DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const override;
	//~ UPrimitiveComponent 끝
	
	//~ INavRelevantInterface 시작
	virtual void GetNavigationData(FNavigationRelevantData& Data) const override;
	virtual bool IsNavigationRelevant() const override;
	//~ INavRelevantInterface 끝

protected:

	// 내비메시에 적용할 영역 클래스. 워터바디의 WaterNavAreaClass를 복사해서 쓴다.
	UPROPERTY(VisibleAnywhere, Category = "Charon|Water|Navigation")
	TSubclassOf<UNavAreaBase> NavAreaClass;

	// 내비 전용 형상. 물리 바디는 만들지 않는다.
	UPROPERTY(VisibleAnywhere, Category = "Charon|Water|Navigation")
	TObjectPtr<UBodySetup> NavBodySetup;
};
