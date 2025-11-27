// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TestingSomethingLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARONEDITOR_API UTestingSomethingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable)
	static bool TestChangeMaterial(UMaterialInterface* InMaterial, UStaticMeshComponent* InStaticMeshComponent);
};
