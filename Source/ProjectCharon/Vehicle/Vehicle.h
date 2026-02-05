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

class ULifeStateComponent;
struct FCharonAbilitySet_GrantedHandles;
class UDeprecated_VehicleLifeStateComponent;
class UAttributeBoundWidget;
class UCharacterAbilityConfig;
class UCharonInputConfig;
class UCharonAbilitySet;
class UGameplayAbility;

// USTRUCT(BlueprintType)
// struct FVehicleUISet
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite)
// 	TArray<TSubclassOf<UAttributeBoundWidget>> WidgetClassList;
// };

USTRUCT(BlueprintType)
struct FRiderSpecData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UCharacterAbilityConfig* AbilityConfig = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AInputFunctionSet> InputFunctionSet = nullptr;

	// UPROPERTY(BlueprintReadOnly)
	// FVehicleUISet VehicleUISet;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FVehicleAttributeChangedDelegate, AActor*, DamageInstigator, AActor*, DamageCauser,
	float, DamageMagnitude, FGameplayTagContainer, DamageType);

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVehicleAbilityActivatedDelegate, UGameplayAbility*, ActivatedAbility);

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

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintNativeEvent)
	bool EnterVehicle(ACharacter* Rider);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintNativeEvent)
	bool ExitVehicle(ACharacter* Rider);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 FindRiderIdx(const ACharacter* Rider);
	
	//~ IAbilitySystemInterface 시작
	/** 어빌리티 시스템 컴포넌트를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface 끝

	// 베히클에 데미지 적용시 브로드캐스트됨.
	UPROPERTY(BlueprintAssignable)
	FVehicleAttributeChangedDelegate OnVehicleDamageApplied;

	// // 베히클이 베히클 어빌리티를 발동할때 브로드캐스트 됨.
	// UPROPERTY(BlueprintAssignable)
	// FVehicleAbilityActivatedDelegate OnVehicleAbilityActivated;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRiderSpecData GetRiderSpecData(uint8 RiderIdx);
	
protected:
	
	//탑승자로 등록
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 RegisterRider(ACharacter* Rider);
	// UFUNCTION(Client, Reliable)
	// void Client_RegisterRider(ACharacter* Rider, int RiderIdx);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RegisterRider(ACharacter* Rider, int RiderIdx);
	
	//탑승자에서 해제. 탑승자 목록에서 없으면 false 리턴.
	UFUNCTION(BlueprintCallable)
	bool UnregisterRider(ACharacter* Rider);
	UFUNCTION(Client, Reliable)
	void Client_UnregisterRider(ACharacter* Rider, int RiderIdx);
	
	void RemoveInvalidRiders();
	
	//탑승자의 메시를 Vehicle에 부착
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AttachToVehicle(ACharacter* Rider);
	//탑승자의 메시를 Vehicle에 탈착
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void DetachFromVehicle(ACharacter* Rider);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//베히클 죽음(파괴) 관련
	UFUNCTION()
	virtual void OnVehicleDeathStarted(AActor* OwningActor);
	UFUNCTION()
	virtual void OnVehicleDeathFinished(AActor* OwningActor);
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Vehicle Death Started"))
	void K2_OnVehicleDeathStarted();
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Vehicle Death Finished"))
	void K2_OnVehicleDeathFinished();
	void DestroyVehicle();

	// //베히클이 어빌리티를 발동했을 때 라이더 컴포넌트로 신호.
	// void HandleVehicleAbilityActivation(UGameplayAbility* ActivatedAbility);
	
	UPROPERTY(BlueprintReadOnly, Replicated, VisibleAnywhere, Category = "Charon | Vehicle")
	int32 CurrentRiderNum = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon | Vehicle")
	int32 MaxRiderNum = 1;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const class UVehicleBasicAttributeSet> VehicleBasicAttributeSet;

	UPROPERTY()
	TObjectPtr<const class UHealthAttributeSet> VehicleHealthAttributeSet;
	
	void HandleVehicleDamageApplied(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	
	
	
	//어빌리티 및 입력
	//베히클 자체가 가지고 있는 어빌리티들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon | Vehicle")
	TSet<UCharonAbilitySet*> VehicleAbilitySets;
	// 어빌리티셋 핸들. 일단 만들어두긴 했는데 어빌리티를 다시 회수할 일이 있나?
	FCharonAbilitySet_GrantedHandles VehicleAbilityHandles;
	//탑승자에게 적용할 어빌리티-입력 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charon | Vehicle | Riders")
	TArray<UCharacterAbilityConfig*> AbilityConfigsForRiders;
	//탑승자에게 적용할 InputFunctionSet
	UPROPERTY(BlueprintReadOnly, Category = "Charon | Vehicle | Riders")
	TArray<TObjectPtr<AInputFunctionSet>> InputFunctionSets;
	
	// // 이거 지금 쓰고 있나? TODO : 리뉴얼
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TArray<FVehicleUISet> VehicleUISets;
	

	// 생사 관련 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ULifeStateComponent> LifeStateComponent;
};
