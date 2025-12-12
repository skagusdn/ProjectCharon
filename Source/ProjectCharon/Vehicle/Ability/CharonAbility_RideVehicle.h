// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CharonGameplayAbility.h"
#include "CharonAbility_RideVehicle.generated.h"

class AVehicle;

/**
 *  탈 것을 탈 때 사용하는 어빌리티
 */
UCLASS(Blueprintable)
class PROJECTCHARON_API UCharonAbility_RideVehicle : public UCharonGameplayAbility
{
	GENERATED_BODY()

public:
	
	UCharonAbility_RideVehicle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	
	
protected:

	// //~UGameplayAbility interfac
	// virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// //~End of UGameplayAbility interface
	
	// UFUNCTION(BlueprintCallable)
	// bool TryEnterVehicle(AVehicle* VehicleToEnter, ACharacter* InRider);
	// UFUNCTION(BlueprintCallable)
	// bool TryExitVehicle(AVehicle* VehicleToExit, ACharacter* InRider);
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnRidingVehicleDestroyed(AActor* DestroyedActor);
	
	UFUNCTION(BlueprintCallable)
	bool TryEnterVehicle(AVehicle* VehicleToEnter);
	UFUNCTION(BlueprintCallable)
	bool TryExitVehicle();

	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AVehicle> RidingVehicle;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACharacter> Rider;

	
	
};
