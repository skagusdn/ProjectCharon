// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonPlayerState.h"

#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

ACharonPlayerState::ACharonPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(60.f);
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCharonAbilitySystemComponent>
		(this, TEXT("CharonAbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	
	
}

UAbilitySystemComponent* ACharonPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

