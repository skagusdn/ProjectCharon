// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DevComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTestValueDelegate2, const float&, OldVlaue);


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTCHARON_API UDevComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDevComponent();

protected:

	UFUNCTION()
	void OnRep_TestValue(const float& OldValue);

	// Called when the game starts
	//virtual void BeginPlay() override;
	
	void OnTestValueUpdate(const float& OldValue);

	

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//UFUNCTION(BlueprintCallable)
	//TArray<FVector> FillBalls(FVector leftDown, FVector rightUp, int32 radius);

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = "OnRep_TestValue")
	float TestValue = 0.0f;
	
	UPROPERTY(BlueprintAssignable)
	FOnTestValueDelegate2 TestDelegate;
};
