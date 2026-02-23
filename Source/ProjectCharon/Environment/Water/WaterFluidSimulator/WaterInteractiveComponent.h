// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "WaterInteractiveComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable, Abstract)
class PROJECTCHARON_API UWaterInteractiveComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	
	UWaterInteractiveComponent();

	UFUNCTION(BlueprintCallable)
	bool GetIsInWater() const {return bIsInWater;};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DrawSize = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DrawStrength = 1.0f;
	

	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	bool bIsInWater;

	
	
	
public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
