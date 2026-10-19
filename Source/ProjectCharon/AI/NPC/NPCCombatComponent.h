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

	UFUNCTION(BlueprintPure, Category = "NPC|Combat|Home")
	FVector GetHomeLocation() const { return HomeLocation; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Combat|Home")
	void SetHomeLocation(const FVector& NewHomeLocation) { HomeLocation = NewHomeLocation; }

	UFUNCTION(BlueprintPure, Category = "NPC|Combat|Home")
	float GetPatrolRadius() const { return PatrolRadius; }

	// Location이 귀환 지점에서 LeashRadius 안에 있는지. 높이는 무시한다.
	UFUNCTION(BlueprintPure, Category = "NPC|Combat|Home")
	bool IsWithinLeash(const FVector& Location) const;

protected:

	virtual void InitializeComponent() override;

	// 귀환 지점을 중심으로 정찰할 반경(cm).
	UPROPERTY(EditAnywhere, Category = "NPC|Combat|Home", meta = (ClampMin = "0.0"))
	float PatrolRadius = 800.f;

	// 귀환 지점에서 이 거리 이상 벗어나면 복귀(cm).
	UPROPERTY(EditAnywhere, Category = "NPC|Combat|Home", meta = (ClampMin = "0.0"))
	float LeashRadius = 2000.f;

	// InitializeComponent 시점(AI 컨트롤러 빙의 전)의 위치로 초기화된다.
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "NPC|Combat|Home")
	FVector HomeLocation = FVector::ZeroVector;

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
