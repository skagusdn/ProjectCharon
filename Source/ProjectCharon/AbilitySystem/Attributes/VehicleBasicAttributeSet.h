// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonAttributeSet.h"
#include "VehicleBasicAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UVehicleBasicAttributeSet : public UCharonAttributeSet
{
	GENERATED_BODY()

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_VehicleHP(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_VehicleMaxHP(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_VehicleSpeed(const FGameplayAttributeData& OldValue);

	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VehicleHP)
	FGameplayAttributeData VehicleHP;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VehicleMaxHP)
	FGameplayAttributeData VehicleMaxHP;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VehicleSpeed)
	FGameplayAttributeData VehicleSpeed;
};
