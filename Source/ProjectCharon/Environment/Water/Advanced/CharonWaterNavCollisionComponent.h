// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "CharonWaterNavCollisionComponent.generated.h"

class UBodySetup;
class UNavAreaBase;

/*
 * 내비게이션 전용 콜리전 컴포넌트.
 * 강 워터바디의 컨벡스를 복사한 뒤 윗면 정점 높이만 베이크된 수면 높이로 바꾼 형상을 콜리전으로 가진다.
 * 모든 채널을 무시하므로 게임플레이 쿼리와는 반응하지 않고, 내비메시 생성에만 형상을 제공한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UCharonWaterNavCollisionComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:

	UCharonWaterNavCollisionComponent(const FObjectInitializer& ObjectInitializer);

	// 콜리전 형상 교체. 각 원소는 월드 공간 정점 배열.
	void SetNavConvexes(const TArray<TArray<FVector>>& WorldSpaceConvexes);

	void SetNavAreaClass(TSubclassOf<UNavAreaBase> InNavAreaClass) { NavAreaClass = InNavAreaClass; }

	UFUNCTION(BlueprintPure, Category = "Charon|Water|Navigation")
	int32 GetNavConvexCount() const;

	//~ UPrimitiveComponent 시작
	virtual UBodySetup* GetBodySetup() override { return NavBodySetup; }
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ UPrimitiveComponent 끝

	//~ INavRelevantInterface 시작
	virtual void GetNavigationData(FNavigationRelevantData& Data) const override;
	virtual bool IsNavigationRelevant() const override;
	//~ INavRelevantInterface 끝

protected:

	// 내비메시에 적용할 영역 클래스.
	UPROPERTY(VisibleAnywhere, Category = "Charon|Water|Navigation")
	TSubclassOf<UNavAreaBase> NavAreaClass;

	// 콜리전 형상. 이 컴포넌트가 직접 소유하므로 강 스플라인을 수정해도 덮어써지지 않는다.
	UPROPERTY(VisibleAnywhere, Category = "Charon|Water|Navigation")
	TObjectPtr<UBodySetup> NavBodySetup;

	// 에디터 뷰포트에 콜리전 외곽선을 그릴지. 게임에서는 항상 숨겨진다.
	UPROPERTY(EditAnywhere, Category = "Charon|Water|Navigation")
	bool bDrawNavCollision = true;
};
