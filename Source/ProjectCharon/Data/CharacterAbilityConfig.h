// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterAbilityConfig.generated.h"

class UCharonInputConfig;
class UCharonAbilitySet;
class AInputFunctionSet;


/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTCHARON_API UCharacterAbilityConfig : public UDataAsset
{
	GENERATED_BODY()

public :
	//UCharacterAbilityConfig(){InputFunctions = CreateDefaultSubobject<UInputFunctionSet>(TEXT("InputFunctions"));}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSet<UCharonAbilitySet*> Abilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UCharonInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AInputFunctionSet> InputFunctionSetClass;
};
