// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputTriggers.h"
#include "Engine/DataAsset.h"
#include "CharonInputConfig.generated.h"

class UInputMappingContext;
class UInputAction;


/* 어빌리티나 함수 - InputAction의 매핑 정보.
 * Lyra에서 가져옴.
 */
USTRUCT(BlueprintType)
struct FCharonInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	//
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// ETriggerEvent TriggerEvent = ETriggerEvent::Triggered;
};

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTCHARON_API UCharonInputConfig : public UDataAsset
{
	GENERATED_BODY()

public :

	UCharonInputConfig(const FObjectInitializer& ObjectInitializer){};

	UFUNCTION(BlueprintCallable)
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	// UFUNCTION(BlueprintCallable)
	// const FCharonInputAction FindNativeCharonInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable)
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
	
	// List of input actions used by the owner.  These input actions are mapped to a gameplay tag and must be manually bound. - 라이라에서 가져온거.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FCharonInputAction> NativeInputActions;

	// List of input actions used by the owner.  These input actions are mapped to a gameplay tag and are automatically bound to abilities with matching input tags.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FCharonInputAction> AbilityInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSet<UInputMappingContext*> DefaultMappingContexts;
	
};
