// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LifeStateComponent.h"
#include "Deprecated_VehicleLifeStateComponent.generated.h"


class UVehicleBasicAttributeSet;
struct FGameplayEffectSpec;
class AVehicle;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UDeprecated_VehicleLifeStateComponent : public ULifeStateComponent
{
	GENERATED_BODY()

public:
	
	UDeprecated_VehicleLifeStateComponent(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWithAbilitySystem(UCharonAbilitySystemComponent* InASC) override;
	
protected:
	virtual void OnRegister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	UPROPERTY()
	const TObjectPtr<const UVehicleBasicAttributeSet> VehicleBasicAttributeSet;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AVehicle> OwnerVehicle;

public:
	
};
