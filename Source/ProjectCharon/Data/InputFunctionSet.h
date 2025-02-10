// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
//#include "GameFramework/Info.h"

#include "InputFunctionSet.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FInputActionDelegate, const FInputActionValue&, InputActionValue, ACharacter*, Initiator);

USTRUCT(BlueprintType)
struct FInputActionDelegateWithTriggerEvent
{
	GENERATED_BODY()
	
	FInputActionDelegate Delegate;
	ETriggerEvent TriggerEvent = ETriggerEvent::Triggered;
};

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTCHARON_API UInputFunctionSet : public UObject
{
	GENERATED_BODY()

public:
	UInputFunctionSet() = default;

	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayTag, FInputActionDelegateWithTriggerEvent> InputFunctionDelegateMap;
	//void RegisterNativeFunction(FGameplayTag Tag, )

	UFUNCTION(BlueprintCallable)
	void AddEvent(FGameplayTag Tag, FInputActionDelegate Delegate, ETriggerEvent TriggerEvent = ETriggerEvent::Triggered);

	// 	void RegisterFunctionSetToInputComponent(UCharonInputComponent* CharonIC);
	// 	
	// protected:
	
	
};
