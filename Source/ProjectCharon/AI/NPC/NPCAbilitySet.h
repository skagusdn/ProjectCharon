// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "NPCAbilitySet.generated.h"

class UNPCAbility;

USTRUCT(BlueprintType)
struct FNPCAbilitySet_NPCAbility
{
	GENERATED_BODY()

	// 타겟에 대해 스스로 사용 가능 여부를 판정하는 NPC 전용 어빌리티.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNPCAbility> Ability = nullptr;

	// 어빌리티 레벨.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;
};

/**
 * FNPCAbilitySet_GrantedHandles
 * 일반 어빌리티 핸들은 부모 구조체가, NPC 어빌리티 핸들은 등록 순서(=우선순위)를
 * 보존한 채로 이 구조체가 따로 들고 있는다.
 */
USTRUCT(BlueprintType)
struct FNPCAbilitySet_GrantedHandles : public FCharonAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	void AddNPCAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);

	// 일반 어빌리티와 NPC 어빌리티를 모두 회수.
	void TakeFromAbilitySystem(UCharonAbilitySystemComponent* CharonASC);

	// NPCAbilitySet에 등록된 순서 그대로 (= NPCCombatComponent가 검사할 우선순위 순서)
	const TArray<FGameplayAbilitySpecHandle>& GetNPCAbilitySpecHandles() const { return NPCAbilitySpecHandles; }

protected:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> NPCAbilitySpecHandles;
};

/**
 * NPC/소환수에게 부여할 어빌리티를 한 곳에서 관리하는 데이터 에셋.
 * - GrantedGameplayAbilities (상속) : AI가 직접 고르지 않는 일반 어빌리티.
 * - GrantedNPCAbilities            : NPCCombatComponent가 타겟을 보고 우선순위대로 골라 쓰는 어빌리티.
 */
UCLASS(BlueprintType, Const)
class PROJECTCHARON_API UNPCAbilitySet : public UCharonAbilitySet
{
	GENERATED_BODY()

public:
	UNPCAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 부모의 GiveToAbilitySystem(FCharonAbilitySet_GrantedHandles*)이 이름 숨김으로 가려지지 않도록.
	using UCharonAbilitySet::GiveToAbilitySystem;

	// 일반 어빌리티(부모 몫)와 NPC 어빌리티를 함께 부여.
	void GiveToAbilitySystem(UCharonAbilitySystemComponent* CharonASC, FNPCAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

protected:
	// 타겟에 대해 골라 쓸 NPC 어빌리티. 배열 순서 = 우선순위 (앞일수록 먼저 검사)
	UPROPERTY(EditDefaultsOnly, Category = "NPC Abilities", meta = (TitleProperty = Ability))
	TArray<FNPCAbilitySet_NPCAbility> GrantedNPCAbilities;
};
