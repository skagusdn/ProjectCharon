// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyReceiverLibrary.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "AnimNotifyDispatcherComponent.generated.h"


class IAnimNotifyReceiverInterface;

class UFXEventLibrary;


USTRUCT(BlueprintType)
struct FTaggedNotifyReceiver
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EffectTag;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// TScriptInterface<IAnimNotifyReceiverInterface> NotifyReceiver;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAnimNotifyReceiverLibrary> NotifyReceiverLibraryClass;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UAnimNotifyDispatcherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UAnimNotifyDispatcherComponent();

protected:
	
	virtual void BeginPlay() override;

public:
	
	UFUNCTION(BlueprintCallable)
	void DispatchNotify(const FName Bone, const FGameplayTag EffectTag, const UAnimSequenceBase* AnimationSequence,
		USkeletalMeshComponent* MeshComp, FVector VFXScale = FVector(1), float AudioVolume = 1, float AudioPitch = 1);

	UFUNCTION(BlueprintCallable)
	void RegisterNotifyReceiver(const FGameplayTag EffectTag, TScriptInterface<IAnimNotifyReceiverInterface> NotifyReceiver);
	

	// UPROPERTY(EditAnywhere)
	// TArray<FTaggedNotifyReceiver> DefaultNotifyReceivers;
	UPROPERTY(EditAnywhere)
	TArray<FTaggedNotifyReceiver> DefaultNotifyReceiverLibraries; 
	

protected:

	//void LoadDefaultFXEventLibraries();
	//TMap<const FGameplayTag, TArray<TScriptInterface<IAnimNotifyReceiverInterface>>> ActiveNotifyReceiverMap;
	
	TMap<const FGameplayTag, TArray<TScriptInterface<IAnimNotifyReceiverInterface>>> ActiveNotifyReceiverMap;
	
};

