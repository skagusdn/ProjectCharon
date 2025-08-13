// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_FXEvent.h"

#include "AnimNotifyDispatcherComponent.h"
#include "Logging.h"
#include "Math/UnitConversion.h"

UAnimNotify_FXEvent::UAnimNotify_FXEvent()
{
}

void UAnimNotify_FXEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if(MeshComp)
	{
		if(AActor* OwningActor = MeshComp->GetOwner())
		{
			if(UAnimNotifyDispatcherComponent* EventReceiverComp = OwningActor->GetComponentByClass<UAnimNotifyDispatcherComponent>())
			{
				UE_LOG(LogCharon, Display, TEXT("AnimNotify - %s"), OwningActor->HasAuthority()?TEXT("Server"):TEXT("Client"));///////
				EventReceiverComp->DispatchNotify(BoneName, EffectTag, Animation, MeshComp);
			}
		}
		
		
	}
}

void UAnimNotify_FXEvent::SetNotifyParams(FGameplayTag InEffectTag, FName InBoneName)
{
	EffectTag = InEffectTag;
	BoneName = InBoneName;
}


