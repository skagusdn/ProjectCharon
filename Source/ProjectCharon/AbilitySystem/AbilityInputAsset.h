// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityInputAsset.generated.h"

class UInputAction;


USTRUCT(BlueprintType)
struct FAbilityInputMapping
{
	GENERATED_BODY()

public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UGameplayAbility> Ability = nullptr;
};



/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTCHARON_API UAbilityInputAsset : public UDataAsset
{
	GENERATED_BODY()

public : 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAbilityInputMapping> AbilityInputMapping;


};
