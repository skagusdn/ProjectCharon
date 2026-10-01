// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPCAbilitySet.h"
#include "Components/ActorComponent.h"
#include "NPCCombatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UNPCCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UNPCCombatComponent(const FObjectInitializer& ObjectInitializer);

	void InitializeWithAbilitySystem(UCharonAbilitySystemComponent* InASC);
	void UninitializeFromAbilitySystem();

	UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
	void SetCurrentTarget(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "NPC|Combat")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	// 현재 타겟에게 사용 가능한 NPCAbility가 하나라도 있는지.
	UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
	bool CanUseAnyAbilityAgainstCurrentTarget() const;

	// 사용 가능한 NPCAbility 중 우선순위가 가장 높은 것을 사용. 서버 전용.
	UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
	bool TryUseAbilityAgainstCurrentTarget();

	// NPCAbility 중 하나라도 실행 중인지.
	UFUNCTION(BlueprintPure, Category = "NPC|Combat")
	bool IsUsingNPCAbility() const;

protected:

	// 우선순위 순으로 검사해서 첫 번째로 사용 가능한 어빌리티의 핸들 반환. 없으면 Invalid 핸들.
	FGameplayAbilitySpecHandle FindUsableNPCAbility(AActor* Target) const;

	// NPC에게 부여할 어빌리티 목록 (일반 어빌리티 + NPC 어빌리티를 함께 담은 에셋)
	UPROPERTY(EditDefaultsOnly, Category = "NPC|Combat")
	TArray<TObjectPtr<UNPCAbilitySet>> AbilitySets;

	UPROPERTY(Transient)
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	FNPCAbilitySet_GrantedHandles GrantedAbilitySetHandles;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentTarget;
};
