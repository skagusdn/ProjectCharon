// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "FXEventLibrary.generated.h"

class UNiagaraSystem;

UENUM()
enum class EEffectsLoadState : uint8
{
	Unloaded = 0,
	Loading = 1,
	Loaded = 2
};

USTRUCT(BlueprintType)
struct FSpecialEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EffectTag;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//FGameplayTagContainer Context;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Engine.SoundBase"))
	TArray<FSoftObjectPath> SoundEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Niagara.NiagaraSystem"))
	TArray<FSoftObjectPath> VisualEffects;
};

UCLASS(BlueprintType)
class PROJECTCHARON_API UActiveSpecialEffects : public UObject
{
	GENERATED_BODY()

public :
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EffectTag;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//FGameplayTagContainer Context;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Engine.SoundBase"))
	TArray<TObjectPtr<USoundBase>> SoundEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Niagara.NiagaraSystem"))
	TArray<TObjectPtr<UNiagaraSystem>> VisualEffects;
	
};

/**
 * FX 이벤트와 관련된 소리, 시각 특수효과 셋
 */
UCLASS(BlueprintType)
class PROJECTCHARON_API UFXEventLibrary : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSpecialEffects> EffectsToLoad;

	UFUNCTION(BlueprintCallable)
	void GetEffects(const FGameplayTag EffectTag, TArray<USoundBase*>& OutSounds, TArray<UNiagaraSystem*>& OutNiagaraSystems);

	UFUNCTION(BlueprintCallable)
	void LoadEffects();

protected:

	UPROPERTY(Transient)
	TArray<TObjectPtr<UActiveSpecialEffects>> ActiveEffects;

	UPROPERTY(Transient)
	EEffectsLoadState LoadState = EEffectsLoadState::Unloaded;
};
