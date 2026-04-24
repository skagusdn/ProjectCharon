// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterAbilityConfig.generated.h"

class UCharonInputConfig;
class UCharonAbilitySet;
class AInputFunctionSet;
class UCharonAbilityTagRelationshipMapping;

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTCHARON_API UCharacterAbilityConfig : public UDataAsset
{
	GENERATED_BODY()

public :
	//UCharacterAbilityConfig(){InputFunctions = CreateDefaultSubobject<UInputFunctionSet>(TEXT("InputFunctions"));}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Abilities")
	TSet<UCharonAbilitySet*> Abilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Input")
	TObjectPtr<const UCharonInputConfig> InputConfig;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Abilities")
	TObjectPtr<UCharonAbilityTagRelationshipMapping> TagRelationshipMapping;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Input")
	TSubclassOf<AInputFunctionSet> InputFunctionSetClass;
};
