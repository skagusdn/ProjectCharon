// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_FindInteractables.h"
#include "CharonAbility_Interaction.h"
#include "Interaction/InteractiveInterface.h"


UAbilityTask_FindInteractables* UAbilityTask_FindInteractables::AbilityTask_FindInteractables(
	UCharonGameplayAbility* InteractionAbility, FCollisionProfileName TraceProfile, USceneComponent* TraceAim,
	float TraceRange, float TraceRate, bool ShowDebug)
{
	UAbilityTask_FindInteractables* AbilityTask =  NewAbilityTask<UAbilityTask_FindInteractables>(InteractionAbility);

	AbilityTask->TraceAim = TraceAim;
	AbilityTask->TraceRange = TraceRange;
	AbilityTask->TraceProfile = TraceProfile;
	AbilityTask->TraceRate = TraceRate;
	AbilityTask->bShowDebug = ShowDebug;
	
	return AbilityTask;
}

void UAbilityTask_FindInteractables::PauseTrace()
{
	GetWorld()->GetTimerManager().PauseTimer( TraceTimerHandle);
}

void UAbilityTask_FindInteractables::UnpauseTrace()
{
	GetWorld()->GetTimerManager().UnPauseTimer( TraceTimerHandle);
}

FString UAbilityTask_FindInteractables::Test_CheckInteractionTarget()
{
	if(!InteractionTarget)
	{
		return FString("NULL");
	}
	return InteractionTarget->GetName();
}

void UAbilityTask_FindInteractables::Activate()
{
	Super::Activate();
	
	GetWorld()->GetTimerManager().SetTimer(TraceTimerHandle, this, &ThisClass::PerformTrace, TraceRate, true); 
}

void UAbilityTask_FindInteractables::OnDestroy(bool bInOwnerFinished)
{
	GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_FindInteractables::PerformTrace()
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		return;
	}

	UWorld* World = GetWorld();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);

	
	FCollisionQueryParams Params;
	Params.AddIgnoredActors(ActorsToIgnore);
	
	FHitResult OutHitResult;

	FVector StartLocation = AvatarActor->GetActorLocation();
	FVector EndLocation = StartLocation + TraceAim->GetForwardVector() * TraceRange;
	World->LineTraceSingleByProfile(OutHitResult, StartLocation, EndLocation, TraceProfile.Name, Params);
	
	AActor* NewInteractionTarget = nullptr;
	if(OutHitResult.GetActor() && OutHitResult.GetActor()->Implements<UInteractiveInterface>())
	{
		NewInteractionTarget = OutHitResult.GetActor();
	}
	 
	if(InteractionTarget != NewInteractionTarget)
	{
		OnInteractionTargetChanged.Broadcast(NewInteractionTarget);
	}

	InteractionTarget = NewInteractionTarget;
	

#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		FColor DebugColor = OutHitResult.bBlockingHit ? FColor::Red : FColor::Green;
		if (OutHitResult.bBlockingHit)
		{
			DrawDebugLine(World, StartLocation, OutHitResult.Location, DebugColor, false, TraceRate);
			DrawDebugSphere(World, OutHitResult.Location, 5, 16, DebugColor, false, TraceRate);
			
		}
		else
		{
			DrawDebugLine(World, StartLocation, EndLocation, DebugColor, false, TraceRate);
		}
	}
#endif // ENABLE_DRAW_DEBUG
}
