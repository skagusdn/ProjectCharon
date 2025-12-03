// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LifeStateComponent.generated.h"

class UCharonAbilitySystemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLifeState_DeathEvent, AActor*, OwningActor);

UENUM(BlueprintType)
enum class ECharonLifeState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API ULifeStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	ULifeStateComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnUnregister() override;
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	
	//virtual void BeginPlay() override;

	void StartDeath();
	void FinishDeath();

	UFUNCTION()
	void OnRep_LifeState(ECharonLifeState OldLifeState);

protected:

	
	
	UPROPERTY()
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(ReplicatedUsing = OnRep_LifeState)
	ECharonLifeState LifeState;
	
public:

	UFUNCTION(BlueprintCallable)
	void InitializeWithAbilitySystem(UCharonAbilitySystemComponent* InASC);
	UFUNCTION(BlueprintCallable)
	void UninitializeFromAbilitySystem();

	UFUNCTION(BlueprintCallable)
	void TryStartDeath();
	UFUNCTION(BlueprintCallable)
	void TryFinishDeath();

	UFUNCTION(BlueprintCallable)
	ECharonLifeState GetLifeState() const {return LifeState;};
	
	UPROPERTY(BlueprintAssignable)
	FLifeState_DeathEvent OnDeathStarted;
	
	UPROPERTY(BlueprintAssignable)
	FLifeState_DeathEvent OnDeathFinished;
	
};
