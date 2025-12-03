// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "InteractiveInterface.h"
#include "AbilitySystemInterface.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "Data/InputFunctionSet.h"
#include "Interaction/InteractiveInterface.h"
#include "Vehicle.generated.h"

struct FCharonAbilitySet_GrantedHandles;
class UVehicleLifeStateComponent;
class UAttributeBoundWidget;
class UCharacterAbilityConfig;
class UCharonInputConfig;
class UCharonAbilitySet;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FVehicleUISet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UAttributeBoundWidget>> WidgetClassList;
};

USTRUCT(BlueprintType)
struct FRiderSpecData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UCharacterAbilityConfig* AbilityConfig;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AInputFunctionSet> InputFunctionSet;

	UPROPERTY(BlueprintReadOnly)
	FVehicleUISet VehicleUISet;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FVehicleAttributeChangedDelegate, AActor*, DamageInstigator, AActor*, DamageCauser,
	float, DamageMagnitude, FGameplayTagContainer, DamageType);

UCLASS(Abstract)
class PROJECTCHARON_API AVehicle : public AActor, public IInteractiveInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	AVehicle();
	
	virtual void Tick(float DeltaTime) override;

	// UPROPERTY(BlueprintReadWrite)
	// TArray<TObjectPtr<ACharacter>> Riders;
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, TObjectPtr<ACharacter>> Riders;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<TObjectPtr<USceneComponent>> Seats;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintImplementableEvent)
	bool EnterVehicle(ACharacter* Rider);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintImplementableEvent)
	bool ExitVehicle(ACharacter* Rider);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 FindRiderIdx(ACharacter* Rider);
	
	//~ IAbilitySystemInterface 시작
	/** 어빌리티 시스템 컴포넌트를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface 끝

	// 탈것에 데미지 적용시 호출됨.
	UPROPERTY(BlueprintAssignable)
	FVehicleAttributeChangedDelegate OnVehicleDamageApplied;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRiderSpecData GetRiderSpecData(uint8 RiderIdx);
	
protected:

	//탑승자로 등록
	UFUNCTION(BlueprintCallable)
	int32 RegisterRider(ACharacter* Rider);
	//탑승자에서 해제. 탑승자 목록에서 없으면 false 리턴.
	UFUNCTION(BlueprintCallable)
	bool UnregisterRider(ACharacter* Rider);
	void RemoveInvalidRiders();
	
	
	//탑승자의 메시를 Vehicle에 부착
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AttachToVehicle(ACharacter* Rider);
	//탑승자의 메시를 Vehicle에 탈착
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void DetachFromVehicle(ACharacter* Rider);
	
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//베히클 죽음 관련
	UFUNCTION()
	virtual void OnVehicleDeathStarted(AActor* OwningActor);
	UFUNCTION()
	virtual void OnVehicleDeathFinished(AActor* OwningActor);
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Vehicle Death Started"))
	void K2_OnVehicleDeathStarted();
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Vehicle Death Finished"))
	void K2_OnVehicleDeathFinished();
	void DestroyVehicle();
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentRiderNum = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxRiderNum = 1;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const class UVehicleBasicAttributeSet> VehicleBasicAttributeSet;
	void HandleVehicleDamageApplied(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	//어빌리티 및 입력
	//베히클 자체가 가지고 있는 어빌리티들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSet<UCharonAbilitySet*> VehicleAbilitySets;
	// 어빌리티셋 핸들. 일단 만들어두긴 했는데 어빌리티를 다시 회수할 일이 있나?
	FCharonAbilitySet_GrantedHandles VehicleAbilityHandles;
	//탑승자에게 적용할 어빌리티-입력 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UCharacterAbilityConfig*> AbilityConfigsForRiders;
	//탑승자에게 적용할 InputFunctionSet
	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<AInputFunctionSet>> InputFunctionSets;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVehicleUISet> VehicleUISets;
	

	// 생사 관련 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UVehicleLifeStateComponent> VehicleLifeStateComponent;
};
