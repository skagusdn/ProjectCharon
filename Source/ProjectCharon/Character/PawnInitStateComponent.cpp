// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnInitStateComponent.h"

#include "CharonGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"

const FName UPawnInitStateComponent::NAME_ActorFeatureName("PawnInitState");

bool UPawnInitStateComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == CharonGameplayTags::InitState_Spawned)
	{
		// As long as we are on a valid pawn, we count as spawned
		if (Pawn)
		{
			return true;
		}
	}
	if (CurrentState == CharonGameplayTags::InitState_Spawned && DesiredState == CharonGameplayTags::InitState_DataAvailable)
	{
		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

		if (bHasAuthority || bIsLocallyControlled)
		{
			// Check for being possessed by a controller.
			if (!GetController<AController>())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == CharonGameplayTags::InitState_DataAvailable && DesiredState == CharonGameplayTags::InitState_DataInitialized)
	{
		// Transition to initialize if all features have their data available
		return Manager->HaveAllFeaturesReachedInitState(Pawn, CharonGameplayTags::InitState_DataAvailable);
	}
	else if (CurrentState == CharonGameplayTags::InitState_DataInitialized && DesiredState == CharonGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
	
}

void UPawnInitStateComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (DesiredState == CharonGameplayTags::InitState_DataInitialized)
	{
		// This is currently all handled by other components listening to this state change
	}
}

void UPawnInitStateComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature is now in DataAvailable, see if we should transition to DataInitialized
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == CharonGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

// Sets default values for this component's properties
UPawnInitStateComponent::UPawnInitStateComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}


void UPawnInitStateComponent::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = { CharonGameplayTags::InitState_Spawned, CharonGameplayTags::InitState_DataAvailable, CharonGameplayTags::InitState_DataInitialized, CharonGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UPawnInitStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for changes to all features
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
	
	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(CharonGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UPawnInitStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

void UPawnInitStateComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("UPawnInitStateComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnInitStateComponents;
	Pawn->GetComponents(UPawnInitStateComponent::StaticClass(), PawnInitStateComponents);
	ensureAlwaysMsgf((PawnInitStateComponents.Num() == 1), TEXT("Only one PawnInitStateComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// Register with the init state system early, this will only work if this is a game world
	RegisterInitStateFeature();
	
}



