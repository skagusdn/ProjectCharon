// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyDispatcherComponent.h"

#include "AnimNotifyReceiverInterface.h"
#include "FXEventLibrary.h"
#include "Logging.h"


UAnimNotifyDispatcherComponent::UAnimNotifyDispatcherComponent()
{
	
	//PrimaryComponentTick.bCanEverTick = true;


}



void UAnimNotifyDispatcherComponent::BeginPlay()
{
	Super::BeginPlay();

	if(UWorld* World = GetWorld())
	{
		for(FTaggedNotifyReceiver Receiver : DefaultNotifyReceiverLibraries)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			
			AAnimNotifyReceiverLibrary* Library =  World->SpawnActor<AAnimNotifyReceiverLibrary>(Receiver.NotifyReceiverLibraryClass, SpawnParams); 
			RegisterNotifyReceiver(Receiver.EffectTag, Library);
		}
	}
	
	
}

void UAnimNotifyDispatcherComponent::DispatchNotify(const FName Bone, const FGameplayTag EffectTag,
	const UAnimSequenceBase* AnimationSequence, USkeletalMeshComponent* MeshComp, FVector VFXScale, float AudioVolume, float AudioPitch)
{
	// if(ActiveNotifyReceiverMap.Contains(EffectTag))
	// {
	// 	for(TScriptInterface<IAnimNotifyReceiverInterface> NotifyReceiver : ActiveNotifyReceiverMap[EffectTag])
	// 	{
	// 		if(NotifyReceiver)
	// 		{
	// 			NotifyReceiver->ReceiveNotify(Bone, EffectTag, AnimationSequence, MeshComp, VFXScale, AudioVolume, AudioPitch);
	// 		}
	// 	}
	// }
	if(ActiveNotifyReceiverMap.Contains(EffectTag))
	{
		for(TScriptInterface<IAnimNotifyReceiverInterface> NotifyReceiver : ActiveNotifyReceiverMap[EffectTag])
		{
			if(NotifyReceiver)
			{
				UE_LOG(LogCharon, Display, TEXT("DispatchNotify - %s"), GetOwner()->HasAuthority()?TEXT("Server"):TEXT("Client"));///////
				NotifyReceiver->Execute_ReceiveNotify(NotifyReceiver.GetObject(), Bone, EffectTag, AnimationSequence, MeshComp, VFXScale, AudioVolume, AudioPitch);
			}
		}
	}

	
}

void UAnimNotifyDispatcherComponent::RegisterNotifyReceiver(const FGameplayTag EffectTag, TScriptInterface<IAnimNotifyReceiverInterface> NotifyReceiver)
{
	if(!ActiveNotifyReceiverMap.Contains(EffectTag))
	{
		ActiveNotifyReceiverMap.Add(EffectTag, TArray<TScriptInterface<IAnimNotifyReceiverInterface>>());
	}

	ActiveNotifyReceiverMap[EffectTag].Add(NotifyReceiver);
}

// void UAnimNotifyDispatcherComponent::LoadDefaultFXEventLibraries()
// {
// 	for(UFXEventLibrary* Library : DefaultFXEventLibraries)
// 	{
// 		Library->LoadEffects();
// 	}
// }


