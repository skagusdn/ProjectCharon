// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CharonGameplayAbility.h"
#include "NPCAbility.generated.h"

/**
 * NPC에게 부여되는 어빌리티 
 * 우선 단순한 공격 어빌리티만 구성.
 */
UCLASS(Abstract)
class PROJECTCHARON_API UNPCAbility : public UCharonGameplayAbility
{
	GENERATED_BODY()
	
public :
	UNPCAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 이 어빌리티를 Target에게 지금 사용할 수 있는지. (GAS 기본 조건 + 사거리 + 추가 조건)
	// NPCCombatComponent가 후보 어빌리티를 고를 때 CDO로 호출하므로, 재정의 시 인스턴스 상태에 의존하지 말 것.
	virtual bool CanActivateAgainstTarget(AActor* Target, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

protected:

	// NPCCombatComponent에 저장된 현재 타겟.
	UFUNCTION(BlueprintPure, Category = "NPC|Ability")
	AActor* GetNPCTarget() const;

	// 타깃이 어빌리티 범위 내에 있는지. 타깃이 필요한 어빌리티가 아니라면 무조건 true
	UFUNCTION(BlueprintNativeEvent, Category = "NPC|Ability")
	bool IsTargetInRange(AActor* Target) const;
	virtual bool IsTargetInRange_Implementation(AActor* Target) const;

	UFUNCTION(BlueprintNativeEvent, Category = "NPC|Ability")
	bool IsExtraConditionMet(AActor* Target) const;
	virtual bool IsExtraConditionMet_Implementation(AActor* Target) const;

	// UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "NPC|Ability")
	// bool bNeedTarget;

};
