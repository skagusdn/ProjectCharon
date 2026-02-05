// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityAssistComponent.h"

#include "CharonGameplayTags.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UAbilityAssistComponent::UAbilityAssistComponent()
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
	//혹시 몰라서 if문 만들긴 했지만 Client는 위에서 컷되더라구.
	if(GetOwner()->HasAuthority())
	{
		AbilitySystemComponent->AbilityCommittedCallbacks.AddUObject(this, &ThisClass::Server_HandleAbilityCommitted);	
	}
	
	
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


}

void UAbilityAssistComponent::BeginPlay()
{
	Super::BeginPlay();
	
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
	///
	if(GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Display, TEXT("Ability Committed TEST SERVER %s %f %f"), *AbilityCommitInfo.Ability.GetName(), AbilityCommitInfo.CooldownDuration, AbilityCommitInfo.RemainingCooldown);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Ability Committed TEST Client %s %f %f"), *AbilityCommitInfo.Ability.GetName(), AbilityCommitInfo.CooldownDuration, AbilityCommitInfo.RemainingCooldown);
	}
	////
}

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





