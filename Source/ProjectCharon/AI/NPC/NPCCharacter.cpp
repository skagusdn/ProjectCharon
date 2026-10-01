// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCCharacter.h"

#include "NPCCombatComponent.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "Character/LifeStateComponent.h"


ANPCCharacter::ANPCCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AbilitySystemComponent = CreateDefaultSubobject<UCharonAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// NPC는 GE를 클라이언트가 예측할 일이 없으므로 Minimal.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	LifeStateComponent = CreateDefaultSubobject<ULifeStateComponent>(TEXT("LifeStateComponent"));
	LifeStateComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	LifeStateComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	CombatComponent = CreateDefaultSubobject<UNPCCombatComponent>(TEXT("CombatComponent"));
}

void ANPCCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	OnAbilitySystemInitialized();
}

void ANPCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 사망을 거치지 않고 제거되는 경우(레벨 언로드, 소환 해제 등)
	OnAbilitySystemUninitialized();

	Super::EndPlay(EndPlayReason);
}

void ANPCCharacter::OnAbilitySystemInitialized()
{
	if (bAbilitySystemInitialized)
	{
		return;
	}
	bAbilitySystemInitialized = true;

	LifeStateComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
	CombatComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void ANPCCharacter::OnAbilitySystemUninitialized()
{
	if (!bAbilitySystemInitialized)
	{
		return;
	}
	bAbilitySystemInitialized = false;

	CombatComponent->UninitializeFromAbilitySystem();
	LifeStateComponent->UninitializeFromAbilitySystem();

	AbilitySystemComponent->CancelAbilities();
	AbilitySystemComponent->RemoveAllGameplayCues();
}

void ANPCCharacter::OnDeathStarted(AActor* OwningActor)
{
	if (OwningActor != this)
	{
		return;
	}

	// 사망 중에는 더 이상 타겟을 공격하지 않도록.
	CombatComponent->SetCurrentTarget(nullptr);
	K2_OnDeathStarted();
}

void ANPCCharacter::OnDeathFinished(AActor* OwningActor)
{
	if (OwningActor != this)
	{
		return;
	}

	K2_OnDeathFinished();
	UninitNPCCharacter();
}

void ANPCCharacter::UninitNPCCharacter()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	OnAbilitySystemUninitialized();
	SetActorHiddenInGame(true);
}


UAbilitySystemComponent* ANPCCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

