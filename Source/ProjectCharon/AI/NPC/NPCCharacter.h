// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "NPCCharacter.generated.h"

class UCharonAbilitySystemComponent;
class ULifeStateComponent;
class UNPCCombatComponent;

/*
 * NPC 캐릭터.
 * 소환수나 중립 NPC, 몬스터 전부 포함.
 * PlayerState가 없으므로 ASC를 캐릭터가 직접 소유한다. (Owner == Avatar == this)
 */
UCLASS(Abstract)
class PROJECTCHARON_API ANPCCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ANPCCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnAbilitySystemInitialized();
	virtual void OnAbilitySystemUninitialized();

	//캐릭터 죽음 관련
	UFUNCTION()
	virtual void OnDeathStarted(AActor* OwningActor);
	UFUNCTION()
	virtual void OnDeathFinished(AActor* OwningActor);
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Death Started"))
	void K2_OnDeathStarted();
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Death Finished"))
	void K2_OnDeathFinished();

	void UninitNPCCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;

	// 생사 관련 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULifeStateComponent> LifeStateComponent;

	// 전투 관련 컴포넌트 (어빌리티 부여, 타겟에 대한 어빌리티 선택/사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNPCCombatComponent> CombatComponent;

	// 초기화 중복 방지
	bool bAbilitySystemInitialized = false;
	
	
public:
	
	//~ IAbilitySystemInterface 시작
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface 끝

	UFUNCTION(BlueprintCallable)
	UCharonAbilitySystemComponent* GetCharonAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintCallable)
	ULifeStateComponent* GetLifeStateComponent() const { return LifeStateComponent; }

	UFUNCTION(BlueprintCallable)
	UNPCCombatComponent* GetCombatComponent() const { return CombatComponent; }
	
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
