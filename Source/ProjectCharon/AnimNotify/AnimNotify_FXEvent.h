// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_FXEvent.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTCHARON_API UAnimNotify_FXEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_FXEvent();
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UFUNCTION(BlueprintCallable)
	void SetNotifyParams(FGameplayTag InEffectTag, FName InBoneName);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FGameplayTag EffectTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FName BoneName = NAME_None;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// bool bPerformTrace = false;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = "bPerformTrace"))
	// bool bIgnoreOwningActor = true;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = "bPerformTrace"))
	// FVector TraceStartOffset = FVector::ZeroVector;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = "bPerformTrace"))
	// FVector TraceEndOffset = FVector::ZeroVector;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// bool bDebugMode = false;
	
protected:
	
};
