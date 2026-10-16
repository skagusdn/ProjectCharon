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

	// 탑승 등으로 어빌리티 세트를 교체해도 회수하지 않는 세트 (Death 등).
	// 캐릭터의 DefaultAbilityConfig에서만 쓰인다. 베히클 탑승자 설정에서는 무시된다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Abilities")
	TSet<UCharonAbilitySet*> PersistentAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Input")
	TObjectPtr<const UCharonInputConfig> InputConfig;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Abilities")
	TObjectPtr<UCharonAbilityTagRelationshipMapping> TagRelationshipMapping;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Input")
	TSubclassOf<AInputFunctionSet> InputFunctionSetClass;
};
