// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "InteractiveInterface.h"
#include "AbilitySystemInterface.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "Data/InputFunctionSet.h"
#include "Interaction/InteractiveInterface.h"
#include "Vehicle.generated.h"

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

UCLASS()
class PROJECTCHARON_API AVehicle : public AActor, public IInteractiveInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVehicle();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
	TArray<TObjectPtr<ACharacter>> Riders;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<TObjectPtr<USceneComponent>> Seats;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool RideVehicle(ACharacter* Rider);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool UnrideVehicle(ACharacter* Rider);
	
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

	UFUNCTION(BlueprintCallable)
	int32 RegisterRider(ACharacter* Rider);

	UFUNCTION(BlueprintCallable)
	bool UnregisterRider(ACharacter* Rider);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void MountVehicle(ACharacter* Rider);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UnmountVehicle(ACharacter* Rider);
	
	// Rider의 RiderComponent에게 승차/하차 신호 보내기.
	virtual void NotifyVehicleChanged(ACharacter* Rider, bool IsRidingOn);
	
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// //에디터에서 프로퍼티 바꿀때 호출.
	// virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentRiderNum = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxRiderNum = 1;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const class UVehicleBasicAttributeSet> VehicleBasicAttributeSet;
	void HandleVehicleDamageApplied(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	//라이더에게 부여할 입력/어빌리티 및 UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UCharacterAbilityConfig*> AbilityConfigsForRiders;

	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<AInputFunctionSet>> InputFunctionSets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVehicleUISet> VehicleUISets;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
};
