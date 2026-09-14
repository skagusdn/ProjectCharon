// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Components/ActorComponent.h"
#include "LifeStateComponent.generated.h"

class UHealthAttributeSet;
class UCharonAbilitySystemComponent;

// 체력이 바뀌었을 때 델리게이트를 통해 전달할 추가 정보. 
USTRUCT(BlueprintType)
struct FHealthChangeInfo
{
	GENERATED_BODY()
	
	// UPROPERTY(BlueprintReadOnly)
	// bool bIsHealed = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bHasHitInfo = false;
	
	UPROPERTY(BlueprintReadOnly)
	FHitResult HitResult;
	
	// 추가적으로 데미지 타입이라던가 필요하면 같이 전달하면 될듯.  
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLifeState_DeathEvent, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FLifeState_AttributeChanged, ULifeStateComponent*, LifeStateComponent, float, OldValue, float, NewValue, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnHealthChanged, ULifeStateComponent*, LifeStateComponent, float, OldValue, float, NewValue, AActor*, Instigator, FHealthChangeInfo, HealthChangeInfo);

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
	
	UFUNCTION(BlueprintCallable)
	virtual void InitializeWithAbilitySystem(UCharonAbilitySystemComponent* InASC);
	UFUNCTION(BlueprintCallable)
	void UninitializeFromAbilitySystem();

	UFUNCTION(BlueprintCallable)
	void TryStartDeath();
	UFUNCTION(BlueprintCallable)
	void TryFinishDeath();

	UFUNCTION(BlueprintCallable)
	ECharonLifeState GetLifeState() const {return LifeState;};

public :

	// Delegate fired when the health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	// Delegate fired when the max health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FLifeState_AttributeChanged OnMaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FLifeState_DeathEvent OnDeathStarted;
	
	UPROPERTY(BlueprintAssignable)
	FLifeState_DeathEvent OnDeathFinished;
	
protected:
	virtual void OnUnregister() override;
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	
	//virtual void BeginPlay() override;

	void StartDeath();
	void FinishDeath();

	UFUNCTION()
	void OnRep_LifeState(ECharonLifeState OldLifeState);

	UFUNCTION()
	void OnRep_AbilitySystemComponent(UCharonAbilitySystemComponent* OldASC);
	
	virtual void HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

protected:
	
	UPROPERTY(ReplicatedUsing = "OnRep_AbilitySystemComponent", Transient)
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(ReplicatedUsing = OnRep_LifeState)
	ECharonLifeState LifeState;

	UPROPERTY()
	TObjectPtr<const UHealthAttributeSet> HealthSet;
	

	
};
