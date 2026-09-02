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


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FVehicleAttributeChangedDelegate, AActor*, DamageInstigator, AActor*, DamageCauser,
	float, DamageMagnitude, FGameplayTagContainer, DamageType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRiderUpdated, ACharacter*, Rider, int32, RiderIdx);

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


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVehicleAbilityActivatedDelegate, UGameplayAbility*, ActivatedAbility);

UCLASS(Abstract)
class PROJECTCHARON_API AVehicle : public AActor, public IInteractiveInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	AVehicle();
	
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Charon|Vehicle")
	int32 GetMaxRiderNum() const {return MaxRiderNum;};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charon|Vehicle")
	TArray<TObjectPtr<USceneComponent>> Seats;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintNativeEvent, Category = "Charon|Vehicle")
	bool EnterVehicle(ACharacter* Rider);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintNativeEvent, Category = "Charon|Vehicle")
	bool ExitVehicle(ACharacter* Rider, bool bForcedExit = false);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Charon|Vehicle")
	int32 FindRiderIdx(const ACharacter* Rider);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Charon|Vehicle")
	int32 GetCurrentRiderNum() const {return CurrentRiderNum;};
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Charon|Vehicle")
	bool IsEmptySeat(int32 RiderIndex) const;
	
	//~ IAbilitySystemInterface 시작
	/** 어빌리티 시스템 컴포넌트를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface 끝

	// 베히클에 데미지 적용시 브로드캐스트됨.
	UPROPERTY(BlueprintAssignable)
	FVehicleAttributeChangedDelegate OnVehicleDamageApplied;
	
	//  라이더에게 부여할 어빌리티 및 입력 설정
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRiderSpecData GetRiderSpecData(uint8 RiderIdx);
	
	UFUNCTION(BlueprintCallable)
	const TArray<ACharacter*>& GetRiders() {return Riders;};
	
	UPROPERTY(BlueprintAssignable)
	FRiderUpdated OnRiderEntered;
	
	UPROPERTY(BlueprintAssignable)
	FRiderUpdated OnRiderExited;
	
	
	
protected:
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, ReplicatedUsing="OnRep_Riders", Category = "Charon|Vehicle|Riders")
	TArray<ACharacter*> Riders;
	
	
	UFUNCTION()
	void OnRep_Riders(const TArray<ACharacter*>& OldRiders);
	
	//탑승자로 등록 -> 탑승자 목록은 OnRep으로 리플리케이트.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 RegisterRider(ACharacter* Rider);
	
	//탑승자에서 해제. 탑승자 목록에서 없으면 false 리턴.
	UFUNCTION(BlueprintCallable)
	bool UnregisterRider(ACharacter* Rider);
	
	//탑승자의 메시를 Vehicle에 부착
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AttachToVehicle(ACharacter* Rider);
	//탑승자의 메시를 Vehicle에 탈착
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void DetachFromVehicle(ACharacter* Rider);
	// 현재 캐릭터가 내릴만한 공간이 있는지.
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	bool CanDetachRider(ACharacter* Rider);
	
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	
	
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
	
	
	UPROPERTY(BlueprintReadOnly, Replicated, VisibleAnywhere, Category = "Charon|Vehicle")
	int32 CurrentRiderNum = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Charon|Vehicle")
	int32 MaxRiderNum = 1;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<const class UHealthAttributeSet> VehicleHealthAttributeSet;
	
	void HandleVehicleDamageApplied(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	
	
	
	//어빌리티 및 입력
	//베히클 자체가 가지고 있는 어빌리티들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charon|Vehicle")
	TSet<UCharonAbilitySet*> VehicleAbilitySets;
	// 어빌리티셋 핸들. 일단 만들어두긴 했는데 어빌리티를 다시 회수할 일이 있나?
	FCharonAbilitySet_GrantedHandles VehicleAbilityHandles;
	//탑승자에게 적용할 어빌리티-입력 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charon|Vehicle|Riders")
	TArray<UCharacterAbilityConfig*> AbilityConfigsForRiders;
	//탑승자에게 적용할 InputFunctionSet
	UPROPERTY(BlueprintReadOnly, Category = "Charon|Vehicle|Riders")
	TArray<TObjectPtr<AInputFunctionSet>> InputFunctionSets;
	
	// // 이거 지금 쓰고 있나? TODO : 리뉴얼
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TArray<FVehicleUISet> VehicleUISets;
	

	// 생사 관련 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ULifeStateComponent> LifeStateComponent;
	
};
