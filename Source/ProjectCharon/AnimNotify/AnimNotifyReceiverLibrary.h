// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyReceiverInterface.h"
#include "GameFramework/Info.h"
#include "AnimNotifyReceiverLibrary.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECTCHARON_API AAnimNotifyReceiverLibrary : public AInfo, public IAnimNotifyReceiverInterface
{
	GENERATED_BODY()

public:
	
	AAnimNotifyReceiverLibrary();

protected:
	
	virtual void BeginPlay() override;

public:

	// UFUNCTION(BlueprintCallable)
	// virtual void ReceiveNotify_Implementation(const FName Bone, const FGameplayTag EffectTag, const UAnimSequenceBase* AnimationSequence,
	// 	USkeletalMeshComponent* MeshComp, FVector VFXScale = FVector(1), float AudioVolume = 1, float AudioPitch = 1);
	

	
};
