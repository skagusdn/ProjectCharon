// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "CharonController.generated.h"


class UCharonUIConfig;
class UCharonActivatableWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateSetDelegate, APlayerController*, PlayerController, APlayerState*, PlayerState);

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
	virtual void PostNetInit() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//void UpdateUIConfigFromGameMode();
	void SetUIConfig(UCharonUIConfig* InUIConfig);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCharonUIConfig* GetUIConfig() const {return UIConfig;};

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateSetDelegate OnPlayerStateSet;
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnRep_PlayerState() override;
	//virtual void InitPlayerState() override;
	//virtual void OnRep_Pawn() override;


	
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

protected:
	UFUNCTION()
	void OnRep_UIConfig(const UCharonUIConfig* OldUIConfig);
	
	UPROPERTY(ReplicatedUsing = OnRep_UIConfig)
	TObjectPtr<UCharonUIConfig> UIConfig;
	
	
	

	
};
