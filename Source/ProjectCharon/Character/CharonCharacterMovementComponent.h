// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharonCharacterMovementComponent.generated.h"


class USwimBuoyancyComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UCharonCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UCharonCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual float ImmersionDepth() const override;

	// 테스트중~~~~~~
	UPROPERTY(BlueprintReadOnly)
	float Slope = 0.0001f;
	UPROPERTY(BlueprintReadOnly)
	float SlopeAccelZ = 0.f;
	UPROPERTY(BlueprintReadOnly)
	FVector HorizontalVelocity;
	UPROPERTY(BlueprintReadOnly)
	FVector TotalTestVelocity;
	// ~~~~~~~~테스트중

protected:

	///// 캐릭터의 수영 및 부력 관련 계수들. 
	
	UPROPERTY(EditAnywhere, Category="Charon|Swimming")
	float BuoyancyDamp = 2.f;
	UPROPERTY(EditAnywhere, Category="Charon|Swimming")
	float BuoyancyDamp2 = 0.f;

	// 수면이 굴곡져 있으면 수영시 따라가기.
	
	UPROPERTY(EditAnywhere, Category="Charon|Swimming|SlopeFollow")
	float SlopeFollowMinHorizontalSpeed = 10.f;
	UPROPERTY(EditAnywhere, Category="Charon|Swimming|SlopeFollow")
	float SlopeSampleTimeAhead = 0.08f;
	UPROPERTY(EditAnywhere, Category="Charon|Swimming|SlopeFollow")
	float MinSlopeSampleDistance = 15.f;
	UPROPERTY(EditAnywhere, Category="Charon|Swimming|SlopeFollow")
	float MaxSlopeSampleDistance = 50.f;
	UPROPERTY(EditAnywhere, Category="Charon|Swimming|SlopeFollow")
	float MaxSlopeFollowVerticalSpeed = 600.f;
	// UPROPERTY(EditAnywhere, Category="Charon|Swimming|SlopeFollow")
	// float SlopeFollowResponseSpeed = 5.f;   
	UPROPERTY(EditAnywhere, Category="Charon|Swimming|SlopeFollow")
	float DepthCorrectionGain = 1.f;
	
	USwimBuoyancyComponent* GetSwimBuoyancyComponent();
	TObjectPtr<USwimBuoyancyComponent> SwimBuoyancyComponent;
	
	virtual void PhysSwimming(float deltaTime, int32 Iterations) override;

	FVector SmoothedBuoyAccel = FVector::ZeroVector;
public:
	
};
