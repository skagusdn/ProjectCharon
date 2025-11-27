// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CharonGameplayStatics.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public :

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetServerWorldTime(UObject* WorldContextObject);
	UFUNCTION(BlueprintCallable,BlueprintPure)
	static float GetServerWorldTimeDelta(UObject* WorldContextObject);
};
