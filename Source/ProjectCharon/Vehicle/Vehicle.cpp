// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle.h"

#include "AbilitySystemComponent.h"
#include "Data/NewInputFunctionSet.h"

// Sets default values
AVehicle::AVehicle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("VehicleASC"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	
}


// Called when the game starts or when spawned
void AVehicle::BeginPlay()
{
	Super::BeginPlay();
}

void AVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
	for (int i = 0; i < MaxRiderNum; i++) {
		Riders.Add(nullptr);
		Seats.Add(nullptr);

		// 되나 시도중.
		AbilityConfigsForRiders.Add(nullptr);
		InputFunctionSets.Add(GetWorld()->SpawnActor<ANewInputFunctionSet>());
	}
}

// Called every frame
void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int32 AVehicle::FindRiderIdx (ACharacter* Rider)
{
	for (int32 i = 0; i < Riders.Num(); i++) 
	{
		if (Riders[i] == Rider) {
			return i;
		}
	}
	return -1;
}

UAbilitySystemComponent* AVehicle::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


int32 AVehicle::GetCurrentRiderNum()
{
	return CurrentRiderNum;
}

int32 AVehicle::Ride(ACharacter* Rider)
{
	if (CurrentRiderNum >= MaxRiderNum)
	{
		UE_LOG(LogTemp, Error, TEXT("Rider number out of range"));
		return -1;
	}
	
	for (int i = 0; i < MaxRiderNum; i++) {
		if (Riders[i] != nullptr) {
			continue;
		}

		Riders[i] = Rider;
		CurrentRiderNum++;
		return i;
	}

	return -1;
}

bool AVehicle::UnRide(ACharacter* character)
{
	int32 idx = FindRiderIdx(character);
	if (idx == -1) {
		return false;
	}
	Riders[idx] = nullptr;
	CurrentRiderNum--;

	return true;
}


