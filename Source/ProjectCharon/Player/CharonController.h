// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "CharonController.generated.h"



/**
 * 
 */
UCLASS()
class PROJECTCHARON_API ACharonController : public APlayerController
{
	GENERATED_BODY()

public :
	UCharonAbilitySystemComponent* GetCharonAbilitySystemComponent() const;

	// Network시계 관련
	UFUNCTION(BlueprintPure)
	float GetServerWorldTimeDelta() const;
	UFUNCTION(BlueprintPure)
	float GetServerWorldTime() const;
	void PostNetInit() override;

	
protected:
	virtual void OnPossess(APawn* InPawn) override;

	/** Frequency that the client requests to adjust it's local clock. Set to zero to disable periodic updates. */
	UPROPERTY(EditDefaultsOnly, Category=GameState)
	float NetworkClockUpdateFrequency = 5.f;

private:

	// Network시계 관련
	void RequestWorldTime_Internal();
	UFUNCTION(Server, Unreliable)
	void ServerRequestWorldTime(float ClientTimestamp);
	UFUNCTION(Client, Unreliable)
	void ClientUpdateWorldTime(float ClientTimestamp, float ServerTimestamp);
	
	float ServerWorldTimeDelta = 0.f;
	float ShortestRoundTripTime = BIG_NUMBER;


	

	
};
