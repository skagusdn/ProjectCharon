// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Info.h"
#include "NewInputFunctionSet.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FInputActionDelegate, const FInputActionValue&, InputActionValue, ACharacter*, Initiator);

USTRUCT(BlueprintType)
struct FInputActionDelegateWithTriggerEvent
{
	GENERATED_BODY()
	
	FInputActionDelegate Delegate;
	ETriggerEvent TriggerEvent = ETriggerEvent::Triggered;
};


UCLASS(BlueprintType, Blueprintable)
class PROJECTCHARON_API ANewInputFunctionSet : public AInfo
{
	GENERATED_BODY()

public:
	
	ANewInputFunctionSet();

	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayTag, FInputActionDelegateWithTriggerEvent> InputFunctionDelegateMap;
	
	UFUNCTION(BlueprintCallable)
	void AddEvent(FGameplayTag Tag, FInputActionDelegate Delegate, ETriggerEvent TriggerEvent = ETriggerEvent::Triggered);
};
