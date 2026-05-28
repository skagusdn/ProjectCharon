// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestAdvancedWaterMaker.generated.h"

class UNiagaraComponent;
class AWaterBody;

UCLASS()
class PROJECTCHARON_API ATestAdvancedWaterMaker : public AActor
{
	GENERATED_BODY()

public:
	
	ATestAdvancedWaterMaker();

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void TestPostRebuild();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<AWaterBody> TargetWaterBody;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UNiagaraComponent> WaterSimSystem;

	
public:

	UPROPERTY(EditAnywhere, Category = "Rendering")
	TObjectPtr <class UMaterialInstance> TestSimWaterMaterial;
	
#if WITH_EDITOR
	UFUNCTION(CallInEditor)
	void Rebuild();
#endif
};
