// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CharonAbilityTypes.h"
#include "Components/ActorComponent.h"
#include "Data/CharacterAbilityConfig.h"
#include "Data/InputFunctionSet.h"
#include "VehicleRiderComponent.generated.h"

class UGameplayAbility;
class UAttributeBoundWidget;
struct FVehicleUISet;
//class ANewInputFunctionSet;
struct FVehicleInfo;
class AVehicle;


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVehicleAbilityCommittedDelegate, UGameplayAbility*, CommittedAbility);


// USTRUCT(BlueprintType)
// struct FAbilityCommitInfo
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(BlueprintReadOnly)
// 	TObjectPtr<UGameplayAbility> Ability = nullptr;
//
// 	UPROPERTY(BlueprintReadOnly)
// 	float CooldownDuration = 0.f;
//
// 	UPROPERTY(BlueprintReadOnly)
// 	float RemainingCooldown = 0.f;
// };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVehicleAbilityCommittedDelegate, FAbilityCommitInfo, CommittedAbility);

/* 캐릭터에서 Vehicle 탑승 처리하는 컴포넌트.
 *
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTCHARON_API UVehicleRiderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVehicleRiderComponent();

protected:
	//virtual void BeginPlay() override;
	virtual void OnRegister() override;
	
public:
	
	UFUNCTION(BlueprintPure)
	static UVehicleRiderComponent* FindRiderComponent(ACharacter* Character);

	UFUNCTION(BlueprintCallable)
	AVehicle* GetRidingVehicle() const {return RidingVehicle;};

	UFUNCTION(BlueprintCallable)
	bool GetIsRidingVehicle() const{ return bIsRidingVehicle; };
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetRidingVehicle(AVehicle* VehicleToRide);

	// void Server_OnVehicleAbilityActivated(UGameplayAbility* Ability);

	UPROPERTY(BlueprintAssignable)
	FVehicleAbilityCommittedDelegate OnVehicleAbilityCommitted;
protected:

	// UFUNCTION(Client, Reliable)
	// void Client_OnVehicleAbilityActivated(UGameplayAbility* Ability);
	// void OnVehicleAbilityActivated(UGameplayAbility* Ability);

	/*
	 * 탑승 중인 베히클이 어빌리티를 발동했을 떄  
	 */
	void Server_HandleVehicleAbilityCommitted(UGameplayAbility* Ability);
	UFUNCTION(Client, Reliable)
	void Client_HandleVehicleAbilityCommitted(FAbilityCommitInfo AbilityCommitInfo);
	void HandleVehicleAbilityCommit(FAbilityCommitInfo AbilityCommitInfo);
	
	UFUNCTION()
	void OnRep_RidingVehicle(AVehicle* OldVehicle);
	
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_RidingVehicle, meta=(AllowPrivateAccess=true))
	TObjectPtr<AVehicle> RidingVehicle;
	
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(BlueprintReadOnly)
	bool bIsRidingVehicle;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UAnimInstance> OriginalRiderAnimClass;
	// TArray<UAttributeBoundWidget*> VehicleWidgets;
};
