// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_Concentration.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FConcentrateFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FConcentrateInterrupted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FConcentrateBlocked);

/**
 *  우선 집중 액션은 어빌리티 단위로 처리하고,
 *  나중에 더 세세한 집중 모드가 필요할 경우 이 집중 어빌리티 태스크를 구현하도록 하자.
 */
UCLASS()
class PROJECTCHARON_API UAbilityTask_Concentration : public UAbilityTask
{
	GENERATED_BODY()

// public:
// 	UPROPERTY(BlueprintAssignable)
// 	FConcentrateFinished OnFinished;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FConcentrateInterrupted OnInterrupted;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FConcentrateInterrupted OnBlocked;
//
// 	static UAbilityTask_Concentration* StartConcentration(UGameplayAbility* OwningAbility, float Duration);
//
// protected:
// 	virtual void Activate() override;
// 	virtual void OnDestroy(bool bInOwnerFinished) override;
//
// private:
// 	void EndConcentration();
// 	
// 	FTimerHandle TimerHandle;
// 	/* 0일 경우 기한 X */
// 	float ConcentrationDuration;
// 	
	

	
};
