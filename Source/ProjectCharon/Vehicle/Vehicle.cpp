// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle.h"

#include "AbilitySystemComponent.h"
#include "RiderComponent.h"
#include "Data/InputFunctionSet.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AVehicle::AVehicle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicateUsingRegisteredSubObjectList = true;
	
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
		
		if(InputFunctionSetClasses.Num() > i)
		{
			InputFunctionSets.Add(GetWorld()->SpawnActor<AInputFunctionSet>(InputFunctionSetClasses[i]));
		} else
		{
			InputFunctionSets.Add(nullptr);
		}
		
		/////////////////////
	}
}

void AVehicle::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AVehicle, InputFunctionSets);
}

#if WITH_EDITOR
void AVehicle::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		//InputFunctionSetClasses는 라이더 최대 수까지만.
		//TODO : MaxRiderNum이 변할때도 비슷하게 해줘야 하는데 뭔가 고려할게 많으니 일단 나중에
		if(PropertyName == GET_MEMBER_NAME_CHECKED(AVehicle, InputFunctionSetClasses))
		{
			while(InputFunctionSetClasses.Num() > MaxRiderNum)
			{
				InputFunctionSetClasses.RemoveAt(InputFunctionSetClasses.Num() - 1);
			}
		}
		
	}
}
#endif

void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AVehicle::RideVehicle(ACharacter* Rider)
{
	check(Rider);
	
	if(RegisterRider(Rider) < 0)
	{
		return false;
	}

	MountVehicle(Rider);
	NotifyVehicleChanged(Rider, true);
	
	return true;
}

bool AVehicle::UnrideVehicle(ACharacter* Rider)
{
	check(Rider);
	bool Ret = UnregisterRider(Rider);
	if(Ret)
	{
		UnmountVehicle(Rider);
		NotifyVehicleChanged(Rider, false);
	}
	
	return Ret;
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

void AVehicle::NotifyVehicleChanged(ACharacter* Rider, bool IsRidingOn)
{
	check(Rider);
	
	if(URiderComponent* RiderComponent = URiderComponent::FindRiderComponent(Rider))
	{
		if(IsRidingOn)
		{
			int RiderIdx = FindRiderIdx(Rider);
			if(RiderIdx < 0)
			{
				return;
			}
			RiderComponent->ServerHandleRide(this, AbilityConfigsForRiders[RiderIdx], InputFunctionSets[RiderIdx]);
		}
		else
		{
			RiderComponent->ServerHandleUnride();
		}
	}
}




int32 AVehicle::RegisterRider(ACharacter* Rider)
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

bool AVehicle::UnregisterRider(ACharacter* Rider)
{
	int32 idx = FindRiderIdx(Rider);
	if (idx == -1) {
		return false;
	}
	Riders[idx] = nullptr;
	CurrentRiderNum--;

	return true;
}




