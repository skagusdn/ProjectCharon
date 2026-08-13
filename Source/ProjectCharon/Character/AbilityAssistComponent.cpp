// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityAssistComponent.h"

#include "CharonCharacter.h"
#include "CharonGameplayTags.h"
#include "PawnInitStateComponent.h"
#include "AbilitySystem/CharonAbilityTagRelationshipMapping.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"
#include "Player/CharonPlayerState.h"


const FName UAbilityAssistComponent::NAME_ActorFeatureName("AbilityAssist");

bool UAbilityAssistComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == CharonGameplayTags::InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == CharonGameplayTags::InitState_Spawned && DesiredState == CharonGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<ACharonPlayerState>())
		{
			return false;
		}

		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == CharonGameplayTags::InitState_DataAvailable && DesiredState == CharonGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		ACharonPlayerState* CharonPS = GetPlayerState<ACharonPlayerState>();

		return CharonPS && Manager->HasFeatureReachedInitState(Pawn, UPawnInitStateComponent::NAME_ActorFeatureName, CharonGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == CharonGameplayTags::InitState_DataInitialized && DesiredState == CharonGameplayTags::InitState_GameplayReady)
	{
		// TODO add ability initialization checks?
		return true;
	}

	return false;
}

void UAbilityAssistComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (CurrentState == CharonGameplayTags::InitState_DataAvailable && DesiredState == CharonGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		ACharonPlayerState* CharonPS = GetPlayerState<ACharonPlayerState>();
		if (!ensure(Pawn && CharonPS))
		{
			return;
		}

		//const ULyraPawnData* PawnData = nullptr;

		if (ACharonCharacter* CharonCharacter = Cast<ACharonCharacter>(Pawn))
		{
			
			//
			if (Pawn->HasAuthority())
			{
				UE_LOG(LogTemp, Warning, TEXT("리자몽 Server HandleChangeInitState %s // %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("리자몽 Client HandleChangeInitState %s // %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));	
			}
			
			InitAbilityAssist(CharonPS->GetCharonAbilitySystemComponent(), CharonPS, CharonCharacter->DefaultAbilityConfig);
			
			//
			if (Pawn->HasAuthority())
			{
				UE_LOG(LogTemp, Warning, TEXT("리자몽 After Server HandleChangeInitState %s // %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("리자몽 After Client HandleChangeInitState %s // %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));	
			}
		}
		
	}
}

void UAbilityAssistComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UPawnInitStateComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == CharonGameplayTags::InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
}

void UAbilityAssistComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { CharonGameplayTags::InitState_Spawned, CharonGameplayTags::InitState_DataAvailable, CharonGameplayTags::InitState_DataInitialized, CharonGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UAbilityAssistComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		//
		if (GetOwner()->HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("OnAbilitySystemInitialized_RegisterAndCall 리자몽 Server %s // %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent) );	
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OnAbilitySystemInitialized_RegisterAndCall 리자몽 Client %s // %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent) );	
		}
		//
		
		Delegate.Execute();
	}
}

void UAbilityAssistComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

// Sets default values for this component's properties
UAbilityAssistComponent::UAbilityAssistComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	
	AbilitySystemComponent = nullptr;

	GrantedAbilityHandles = {};
}

void UAbilityAssistComponent::InitAbilityAssist(UCharonAbilitySystemComponent* InAsc, AActor* InOwnerActor,
	const TObjectPtr<UCharacterAbilityConfig>& InAbilityConfig)
{
	
	DefaultAbilityConfig = InAbilityConfig;

	//InitializeAttributes();
	if(GetOwner() && GetOwner()->HasAuthority())
	{
		InitializeAbilitySystem(InAsc, InOwnerActor);
		//TODO : 등록 하는 로직이 아직 애매함. 일부만 Ability를 등록 해제 한다거나 그런거 없음. 여기 명확하게 하기. 
		ClearAbilitySet();
		SwitchAbilitySet(DefaultAbilityConfig->Abilities);	
	}
	
}

void UAbilityAssistComponent::InitializeAbilitySystem(UCharonAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	//본의는 아니지만 AbilitySystemComponent를 리플리케이션 설정해놔서 클라이언트는 이 부분에서 return되네.
	//그럼 걍 Init은 서버에서만 하게 하자?
	if(InASC == AbilitySystemComponent)
	{
		return;
	}

	if(AbilitySystemComponent)
	{
		UninitializeAbilitySystem();
	}

	AActor* OwnerActor = GetOwner();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogTemp, Verbose, TEXT("Setting up ASC [%s] on actor [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(OwnerActor), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));
	

	//Avatar Actor가 기본적으로 Owner,그러니까 PlayerState로 설정되어 있어서 여기는 그냥 들어오게 됨.
	//아직 폰 교체 로직이 없어서 어떻게 될지 모르겠네.
	//-> 어떻게 해결했던거같은데 기억안남 ㅎ
	if ((ExistingAvatar != nullptr) && (ExistingAvatar != OwnerActor))
	{
		UE_LOG(LogTemp, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// There is already a pawn acting as the ASC's avatar, so we need to kick it out
		// This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed

		ensure(!ExistingAvatar->HasAuthority());
			
		if (UAbilityAssistComponent* OtherAssistComp = FindAbilityAssistComponent(ExistingAvatar))
		{
			OtherAssistComp->UninitializeAbilitySystem();
		}
	}
	
	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, GetOwner());

	if(ensure(DefaultAbilityConfig) && DefaultAbilityConfig->TagRelationshipMapping)
	{
		InASC->SetTagRelationshipMapping(DefaultAbilityConfig->TagRelationshipMapping);
	}
	
	//혹시 몰라서 if문 만들긴 했지만 Client는 위에서 컷되더라구.
	if(GetOwner()->HasAuthority())
	{
		AbilitySystemComponent->AbilityCommittedCallbacks.AddUObject(this, &ThisClass::Server_HandleAbilityCommitted);	
	}
	
	//
	if (GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("리자몽 Server InitASC %s // %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("리자몽 Client InitASC %s // %s"), *GetNameSafe(GetOwner()), *GetNameSafe(AbilitySystemComponent));	
	}
	
	OnAbilitySystemInitialized.Broadcast();
	K2_OnAbilitySystemInitialized.Broadcast();
}

void UAbilityAssistComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 현재 이 컴포넌트의 오너가 아직도 ASC의 아바타 액터라면, (아니라면 새로 ASC의 아바타가 된 폰이 Uninit을 했을 것.)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(CharonGameplayTags::Ability_Option_IgnoreDeath);
		
		// 특정 태그를 가지고 있는 어빌리티를 제외하고 모든 어빌리티를 중단, Cancel함. 
		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		
		//AbilitySystemComponent->ClearAbilityInput(); 라이라는 ASC에서 어빌리티 입력을 다뤘는데 여기선 아니니까 어떻게 처리할지..
		AbilitySystemComponent->RemoveAllGameplayCues();

		FGameplayTagContainer EffectTypesToIgnore;
		EffectTypesToIgnore.AddTag(CharonGameplayTags::GameplayEffect_Option_IgnoreDeath);
		AbilitySystemComponent->RemoveActiveEffects(FGameplayEffectQuery::MakeQuery_MatchNoOwningTags(EffectTypesToIgnore));
		
		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			AbilitySystemComponent->ClearActorInfo();
		}

		ClearAbilitySet();
	}

	//UninitializeAttributes();
	AbilitySystemComponent->AbilityCommittedCallbacks.RemoveAll(this);
	AbilitySystemComponent = nullptr;

	OnAbilitySystemUninitialized.Broadcast();
	K2_OnAbilitySystemUninitialized.Broadcast();
}

void UAbilityAssistComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UPawnInitStateComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(CharonGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UAbilityAssistComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAbilityAssistComponent, AbilitySystemComponent);
	DOREPLIFETIME(UAbilityAssistComponent, DefaultAbilityConfig);

	//DOREPLIFETIME(UAbilityAssistComponent, AttributeChangedDelegates);
}

void UAbilityAssistComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

void UAbilityAssistComponent::SwitchAbilitySet(TSet<UCharonAbilitySet*> Abilities)
{
	ClearAbilitySet();
	for(UCharonAbilitySet* AbilitySet : Abilities)
	{
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &GrantedAbilityHandles);
	}
}

void UAbilityAssistComponent::ClearAbilitySet()
{
	if(GrantedAbilityHandles)
	{
		GrantedAbilityHandles.TakeFromAbilitySystem(AbilitySystemComponent);
		GrantedAbilityHandles = {};
	}
}

void UAbilityAssistComponent::Server_HandleAbilityCommitted(UGameplayAbility* Ability)
{
	if(!GetOwner()->HasAuthority())
	{
		return;
	}
	
	FAbilityCommitInfo AbilityCommitInfo;
	AbilityCommitInfo.Ability = Ability;
	const FGameplayAbilityActorInfo ActorInfo = Ability->GetActorInfo();
	Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(), &ActorInfo , AbilityCommitInfo.CooldownDuration, AbilityCommitInfo.RemainingCooldown );

	HandleAbilityCommitted(AbilityCommitInfo);
	Client_HandleAbilityCommitted(AbilityCommitInfo);
}

void UAbilityAssistComponent::HandleAbilityCommitted(FAbilityCommitInfo AbilityCommitInfo)
{
	OnAbilityCommitted.Broadcast(AbilityCommitInfo);
	// ///
	// if(GetOwner()->HasAuthority())
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("Ability Committed TEST SERVER %s %f %f"), *AbilityCommitInfo.Ability.GetName(), AbilityCommitInfo.CooldownDuration, AbilityCommitInfo.RemainingCooldown);
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("Ability Committed TEST Client %s %f %f"), *AbilityCommitInfo.Ability.GetName(), AbilityCommitInfo.CooldownDuration, AbilityCommitInfo.RemainingCooldown);
	// }
	// ////
}

// void UAbilityAssistComponent::InitTagRelationship()
// {
// 	UCharonAbilityTagRelationshipMapping* TagRelationship = DefaultAbilityConfig->TagRelationshipMapping; 
// 	if(!AbilitySystemComponent || !DefaultAbilityConfig || !TagRelationship)
// 	{
// 		return;
// 	}
// 	
// 	
// 	AbilitySystemComponent->SetTagRelationshipMapping(TagRelationship);
// 	
// 	FGameplayTagContainer NonAbilityTags;
// 	TagRelationship->GetNonAbilityRelationshipTags(&NonAbilityTags);
//
// 	for(FGameplayTag Tag : NonAbilityTags)
// 	{
// 		AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).Add
// 	}
// 	
// }

// void UAbilityAssistComponent::OnRelatedTagAddedOrRemoved(FGameplayTag Tag)
// {
// 	UCharonAbilityTagRelationshipMapping* TagRelationship = DefaultAbilityConfig->TagRelationshipMapping;
// 	ensure(AbilitySystemComponent && TagRelationship);
//
// 	AbilitySystemComponent->Tag
// 	TagRelationship->GetAbilityTagsToBlockAndCancel(Tag, )
// }

void UAbilityAssistComponent::Client_HandleAbilityCommitted_Implementation(FAbilityCommitInfo AbilityCommitInfo)
{
	if(GetOwner()->HasAuthority())
	{
		return;
	}

	HandleAbilityCommitted(AbilityCommitInfo);
}

void UAbilityAssistComponent::ResetToDefaultAbilitySet()
{
	ClearAbilitySet();
	SwitchAbilitySet(DefaultAbilityConfig->Abilities);
}





