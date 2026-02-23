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
protected:

	USwimBuoyancyComponent* GetSwimBuoyancyComponent();
	TObjectPtr<USwimBuoyancyComponent> SwimBuoyancyComponent;
	
	virtual void PhysSwimming(float deltaTime, int32 Iterations) override;

public:
	
};
