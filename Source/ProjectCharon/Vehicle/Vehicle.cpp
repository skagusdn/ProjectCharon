// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle.h"

#include "AbilitySystemComponent.h"
#include "NiagaraShared.h"
#include "Deprecated_VehicleLifeStateComponent.h"
#include "VehicleRiderComponent.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "AbilitySystem/Attributes/VehicleBasicAttributeSet.h"
#include "AbilitySystem/Attributes/HealthAttributeSet.h"
#include "Data/InputFunctionSet.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AVehicle::AVehicle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicateUsingRegisteredSubObjectList = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UCharonAbilitySystemComponent>(TEXT("VehicleASC"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	VehicleAbilityHandles = FCharonAbilitySet_GrantedHandles();
	
	LifeStateComponent = CreateDefaultSubobject<ULifeStateComponent>(TEXT("LifeStateComponent"));
	LifeStateComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnVehicleDeathStarted);
	LifeStateComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnVehicleDeathFinished);
	
	VehicleBasicAttributeSet = nullptr;

	//AbilitySystemComponent->AbilityCommittedCallbacks.AddUObject(this, &ThisClass::HandleVehicleAbilityActivation);
}


// Called when the game starts or when spawned
void AVehicle::BeginPlay()
{
	Super::BeginPlay();
}

void AVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 이 아래 부분 의미가 없다. 어차피 베히클 파괴되면 Ride? EnterVehicle? Ability가 알아서 각 Rider를 내리게함.
	// DeathFinished 상태가 아니라면 정상적으로 베히클이 파괴된게 아니라는 뜻.
	ECharonLifeState LifeState =  LifeStateComponent->GetLifeState();
	if(LifeState != ECharonLifeState::DeathFinished)
	{
		TArray<ACharacter*> RidersToExit;
		// 강제적으로 탑승자 내리게하기. RideVehicle 어빌리티 쪽에서 처리하긴하는데 이렇게 중복으로 만들어둬도 되나?
		// 바로 Riders 에서 루프 돌면서 ExitVehicle하면 루프 내에서 Riders를 삭제하는 꼴이라고 경고줌.
		for(TTuple<int32, ACharacter*> Tuple : Riders)
		{
			if(ACharacter* Rider = Tuple.Value)
			{
				RidersToExit.Add(Rider);	
			}
		}

		for(ACharacter* Rider : RidersToExit)
		{
			ExitVehicle(Rider);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void AVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(HasAuthority())
	{
		//ASC 초기화
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 베히클 어빌리티 셋 부여
		for(UCharonAbilitySet* AbilitySet : VehicleAbilitySets)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &VehicleAbilityHandles, this);
		}

		// ///// 테스트 중
		// VehicleHealthAttributeSet = AbilitySystemComponent->GetSet<UHealthAttributeSet>();
		// if(VehicleHealthAttributeSet)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("AVehicle HealthTest : Vehicle Health Set Valid"));
		// }else
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("AVehicle HealthTest : Vehicle Health Set Invalid"));
		// }
		// /////
		
		// LifeComponent 초기화.
		LifeStateComponent->UninitializeFromAbilitySystem(); // 없으면 문제 발생할 수도.
		LifeStateComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
	}

	
	
	// Riders에 부여할 AbilityConfig 및 InputFunctionSet 초기화...라기보단 정리. 
	for (int i = 0; i < MaxRiderNum; i++) {
		//Riders.Add(nullptr);
		Seats.Add(nullptr);

		// 어빌리티 COnfig 및 InputFunctionSet 초기화.
		AbilityConfigsForRiders.Add(nullptr);

		if(AbilityConfigsForRiders.Num() > i && AbilityConfigsForRiders[i]->InputFunctionSetClass != nullptr)
		{
			InputFunctionSets.Add(GetWorld()->SpawnActor<AInputFunctionSet>(AbilityConfigsForRiders[i]->InputFunctionSetClass));
		} else
		{
			InputFunctionSets.Add(nullptr);
		}
	}
	
	VehicleBasicAttributeSet = AbilitySystemComponent->GetSet<UVehicleBasicAttributeSet>();
	if(VehicleBasicAttributeSet)
	{
		VehicleBasicAttributeSet->OnVehicleDamageApplied.AddUObject(this, &ThisClass::HandleVehicleDamageApplied);
	}


	
}

void AVehicle::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AVehicle, Riders); -> 맵은 리플리케이션이 안되넹.
	//DOREPLIFETIME(AVehicle, InputFunctionSets);
	DOREPLIFETIME(AVehicle, CurrentRiderNum);
}

void AVehicle::OnVehicleDeathStarted(AActor* OwningActor)
{
	if(OwningActor != this)
	{
		return;
	}

	K2_OnVehicleDeathStarted();
}

void AVehicle::OnVehicleDeathFinished(AActor* OwningActor)
{
	if(OwningActor != this)
	{
		return;
	}
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyVehicle);
}

void AVehicle::DestroyVehicle()
{
	K2_OnVehicleDeathFinished();
	
	if (HasAuthority())
	{
		SetLifeSpan(0.1f);
	}

	SetActorHiddenInGame(true);
}



void AVehicle::HandleVehicleDamageApplied(AActor* DamageInstigator, AActor* DamageCauser,
                                          const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnVehicleDamageApplied.Broadcast(DamageInstigator, DamageCauser, DamageMagnitude, DamageEffectSpec->GetDynamicAssetTags());
}

// #if WITH_EDITOR
// void AVehicle::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
// {
// 	Super::PostEditChangeProperty(PropertyChangedEvent);
// 	if(PropertyChangedEvent.Property)
// 	{
// 		FName PropertyName = PropertyChangedEvent.Property->GetFName();
// 		//InputFunctionSetClasses는 라이더 최대 수까지만.
// 		//TODO : MaxRiderNum이 변할때도 비슷하게 해줘야 하는데 뭔가 고려할게 많으니 일단 나중에
// 		if(PropertyName == GET_MEMBER_NAME_CHECKED(AVehicle, InputFunctionSetClasses))
// 		{
// 			while(InputFunctionSetClasses.Num() > MaxRiderNum)
// 			{
// 				InputFunctionSetClasses.RemoveAt(InputFunctionSetClasses.Num() - 1);
// 			}
// 		}
// 		
// 	}
// }
// #endif

void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AVehicle::EnterVehicle_Implementation(ACharacter* Rider)
{
	if(!HasAuthority())
	{
		return false;;
	}
	
	const int32 Ret = RegisterRider(Rider);
	if(Ret >= 0)
	{
		AttachToVehicle(Rider);
		//Rider->OnDestroyed.AddDynamic(this, &ThisClass::OnRiderDestroyed);
	}
	
	return Ret >= 0;
}

bool AVehicle::ExitVehicle_Implementation(ACharacter* Rider)
{
	if(!HasAuthority())
	{
		return false;;
	}
	
	const bool Ret = UnregisterRider(Rider);
	if(Ret)
	{
		DetachFromVehicle(Rider);
	}
	
	return Ret;
}

int32 AVehicle::FindRiderIdx (const ACharacter* Rider)
{
	for (int32 i = 0; i < MaxRiderNum; i++) 
	{
		if (Riders.Contains(i) && Riders[i] == Rider) {
			return i;
		}
	}
	return -1;
}

UAbilitySystemComponent* AVehicle::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FRiderSpecData AVehicle::GetRiderSpecData(const uint8 RiderIdx)
{
	FRiderSpecData RiderSpecData{
		AbilityConfigsForRiders.Num() > RiderIdx ? AbilityConfigsForRiders[RiderIdx] : nullptr,
		InputFunctionSets.Num() > RiderIdx ? InputFunctionSets[RiderIdx] : nullptr,
		//VehicleUISets.Num() > RiderIdx ? VehicleUISets[RiderIdx] : FVehicleUISet()
	};

	return RiderSpecData;
}


int32 AVehicle::RegisterRider(ACharacter* Rider)
{
	if(!HasAuthority())
	{
		return -1;
	}
	
	if (CurrentRiderNum >= MaxRiderNum)
	{
		UE_LOG(LogTemp, Error, TEXT("Rider number out of range"));
		return -1;
	}
	
	for (int i = 0; i < MaxRiderNum; i++) {
		if (Riders.Contains(i)) {
			continue;
		}

		Riders.Add(i,Rider);
		CurrentRiderNum++;
		//Client_RegisterRider(Rider, i);
		Multicast_RegisterRider(Rider, i);
		return i;
	}

	return -1;
}

// void AVehicle::Client_RegisterRider_Implementation(ACharacter* Rider, int RiderIdx)
// {
// 	ensure(Rider);
// 	Riders.Add(RiderIdx, Rider);
// }

void AVehicle::Multicast_RegisterRider_Implementation(ACharacter* Rider, int RiderIdx)
{
	ensure(Rider);
	Riders.Add(RiderIdx, Rider);
}

bool AVehicle::UnregisterRider(ACharacter* Rider)
{
	int32 idx = FindRiderIdx(Rider);
	if (idx == -1) {
		return false;
	}
	Riders.Remove(idx);
	CurrentRiderNum--;
	Client_UnregisterRider(Rider, idx);

	return true;
}

void AVehicle::Client_UnregisterRider_Implementation(ACharacter* Rider, int RiderIdx)
{
	ensure(Rider);
	Riders.Remove(RiderIdx);
}

void AVehicle::RemoveInvalidRiders()
{
	for(TTuple<int32, TObjectPtr<ACharacter>> Tuple : Riders)
	{
		if(!Tuple.Value)
		{
			
		}
	}
}






