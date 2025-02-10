// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DelegateTester.generated.h"

DECLARE_DYNAMIC_DELEGATE(FTestDelegate);

UCLASS()
class PROJECTCHARON_API ADelegateTester : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADelegateTester();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FTestDelegate TestDelegate;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetDelegate(FTestDelegate InDelegate);

	UFUNCTION(BlueprintCallable)
	void FireDelegate();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTestDelegate PublicDelegate;
};
