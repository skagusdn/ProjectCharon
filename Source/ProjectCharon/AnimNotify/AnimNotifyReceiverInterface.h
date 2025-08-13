// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "AnimNotifyReceiverInterface.generated.h"


// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UAnimNotifyReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTCHARON_API IAnimNotifyReceiverInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceiveNotify(const FName Bone, const FGameplayTag EffectTag, const UAnimSequenceBase* AnimationSequence,
		USkeletalMeshComponent* MeshComp, FVector VFXScale = FVector(1), float AudioVolume = 1, float AudioPitch = 1);
};
