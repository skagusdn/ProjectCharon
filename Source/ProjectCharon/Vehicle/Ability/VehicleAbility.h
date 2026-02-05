// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CharonGameplayAbility.h"
#include "VehicleAbility.generated.h"

USTRUCT(BlueprintType)
struct FVehicleAbilityMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UVehicleAbility> VehicleAbilityClass;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class AVehicle> TargetVehicle; 
	
	UPROPERTY(BlueprintReadWrite)
	float RemainingCooldown;
};

class AVehicle;
/**
 *  베히클이 사용할 어빌리티 
 */
UCLASS(Blueprintable, Abstract, BlueprintType)
class PROJECTCHARON_API UVehicleAbility : public UCharonGameplayAbility
{
	GENERATED_BODY()

public:

	UVehicleAbility();
	
	UFUNCTION(BlueprintCallable)
	TSubclassOf<AVehicle> GetTargetVehicleClass() const {return TargetVehicleClass;};
	
protected:

	// GameplayAbility Interface 시작~
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags) override;
	// GameplayAbility Interface 끝~

	// 어빌리티가 이벤트로 발동됐을 때 이벤트 데이터를 체크할 함수.
	UFUNCTION(BlueprintNativeEvent)
	bool CheckEventData(const FGameplayEventData& EventData);

	// 이 어빌리티를 사용할 베히클 클래스. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon | VehicleAbility")
	TSubclassOf<AVehicle> TargetVehicleClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon | VehicleAbility")
	bool bShouldCheckTriggerData;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AVehicle> AvatarVehicle;
	
};
