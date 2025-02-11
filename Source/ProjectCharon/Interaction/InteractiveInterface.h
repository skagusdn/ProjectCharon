// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractiveInterface.generated.h"

UENUM(Blueprintable)
enum class InteractResult : uint8
{
	InteractFail,
	InteractEnd,
	InteractInProgress
};



// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractiveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTCHARON_API IInteractiveInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Charon|Interaction")
	InteractResult Interact(ACharacter* Character);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Charon|Interaction")
	void CancelInteraction(ACharacter* Character);
};
