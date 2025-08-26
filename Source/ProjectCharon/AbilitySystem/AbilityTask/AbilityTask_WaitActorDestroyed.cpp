// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_WaitActorDestroyed.h"

UAbilityTask_WaitActorDestroyed* UAbilityTask_WaitActorDestroyed::WaitActorDestroyed(UGameplayAbility* OwningAbility,
	AActor* TargetActor)
{
	UAbilityTask_WaitActorDestroyed* AbilityTask = NewAbilityTask<UAbilityTask_WaitActorDestroyed>(OwningAbility);
	if(AbilityTask && TargetActor)
	{
		AbilityTask->TargetActor = TargetActor;
	}

	return AbilityTask;
}

void UAbilityTask_WaitActorDestroyed::Activate()
{
	Super::Activate();

	AActor* Actor = TargetActor.Get();
	if(Actor)
	{
		Actor->OnDestroyed.AddDynamic(this, &ThisClass::HandleTargetActorDestroyed);
	}
	else
	{
		HandleTargetActorDestroyed(nullptr);
	}
}

void UAbilityTask_WaitActorDestroyed::HandleTargetActorDestroyed(AActor* DestroyedActor)
{
	OnTargetActorDestroyed.Broadcast();

	EndTask();
}
