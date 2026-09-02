// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CharonAbilityUtilities.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;

USTRUCT(BlueprintType)
struct FAbilityCooldownInfo
{
	GENERATED_BODY()
	
	bool bHasAbility = false;
	bool bCanActivate = false;       // 발동 가능 여부
	bool bIsOnCooldown = false;      // 쿨다운 중인지 여부
	float TimeRemaining = 0.0f;      // 남은 쿨다운 시간 (초)
	float TotalDuration = 0.0f;      // 전체 쿨다운 시간 (초)
};

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonAbilityUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public :
	/**
	 * 타겟 ASC에서 특정 어빌리티 클래스가 존재하는지 확인하고,
	 * 현재 시점에서 Commit(Cost + Cooldown 등) 가능한지 검사합니다.
	 *
	 * @param TargetASC 검사할 타겟의 Ability System Component
	 * @param AbilityClass 찾고자 하는 어빌리티 클래스
	 * @param OutFoundSpec (선택) 찾은 어빌리티의 Spec 핸들 및 정보 포인터
	 * @return Commit 가능 여부 (존재하지 않거나 조건 불만족 시 false)
	 */
	UFUNCTION(BlueprintCallable, Category = "Charon|Abilities")
	static bool CanCommitAbilityByClass(
		UAbilitySystemComponent* TargetASC, 
		TSubclassOf<UGameplayAbility> AbilityClass,
		FGameplayAbilitySpecHandle& OutFoundSpecHandle
	);
	
	UFUNCTION(BlueprintCallable, Category = "Charon|Abilities")
	static FAbilityCooldownInfo CheckAbilityStatusAndCooldown(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayAbility> TargetAbilityClass);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Charon|Abilities")
	static FGameplayTagContainer GetCooldownTagsFromAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Charon|Abilities")
	static FGameplayTagContainer GetCooldownTagsFromAbilityInstance(UGameplayAbility* Ability);
};
