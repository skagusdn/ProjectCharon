// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonPlayerState.h"

#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include "Vehicle/VehicleManager/VehicleManagerSubsystem.h"


ACharonPlayerState::ACharonPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(60.f);
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCharonAbilitySystemComponent>
		(this, TEXT("CharonAbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
}

void ACharonPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharonPlayerState, CharacterMesh);
}

void ACharonPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
}

void ACharonPlayerState::OnRep_CharacterMesh(const USkeletalMeshComponent* OldCharacterMesh)
{
	if (UVehicleManagerSubsystem* VehicleManager = GetWorld()->GetSubsystem<UVehicleManagerSubsystem>())
	{
		VehicleManager->UpdateRiderMesh(this, CharacterMesh);
	}
}




UAbilitySystemComponent* ACharonPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharonPlayerState::SetCharacterMesh(USkeletalMeshComponent* NewCharacterMesh)
{
	CharacterMesh = NewCharacterMesh;
	if(HasAuthority())
	{
		OnRep_CharacterMesh(NewCharacterMesh);
	}
}

