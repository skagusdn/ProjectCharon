// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CharonGameplayAbility.h"
#include "RiderAbility.generated.h"

class URiderAbility;
class AVehicle;
class UVehicleAbility;

UCLASS(Blueprintable, BlueprintType)
class PROJECTCHARON_API URiderSkillInfo : public UDataAsset
{
	GENERATED_BODY()

public :
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<URiderAbility> RiderAbilityClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSlateBrush IconBrush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SkillDescription;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SkillName;
};

/**
 *  Vehicle Ability를 트리거하는 어빌리티
 */
UCLASS(Blueprintable, Abstract, BlueprintType)
class PROJECTCHARON_API URiderAbility : public UCharonGameplayAbility
{
	GENERATED_BODY()

public:
	URiderAbility();

protected:

	//GameplayAbility Interface 시작~
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	//GameplayAbility Interface 끝~

	void SetVehicleInfo();
	UFUNCTION(BlueprintCallable, Category = "Charon | RiderAbility")
	bool DoesVehicleImplementsTargetClass() const;
	UFUNCTION(BlueprintCallable, Category = "Charon | RiderAbility")
	bool DoesVehicleHasTargetAbility() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon | RiderAbility")
	TSubclassOf<UVehicleAbility> VehicleAbilityClassToTrigger;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AVehicle> TargetVehicle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon | RiderAbility")
	TSubclassOf<AVehicle> TargetVehicleClass;

	// 퀵슬롯에 표시해야하는 지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon | RiderAbility")
	bool bShouldRegisterToSlot;

	// 라이더 어빌리티 자체 쿨다운뿐만 아닌 타겟 베히클 쿨다운도 사용할지
	// 둘 중 하나라도 쿨타임이면 어빌리티 못 씀.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon | RiderAbility")
	bool bDoesShareCooldownWithTargetVehicleAbility;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon | RiderAbility")
	TObjectPtr<URiderSkillInfo> SkillInfo;
};
