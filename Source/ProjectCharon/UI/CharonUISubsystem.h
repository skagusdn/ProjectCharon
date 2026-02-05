// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "CharonUISubsystem.generated.h"


class UCharonUIDeveloperSettings;
class UCharonUIPolicy;
class UCharonRootLayout;
class UCharonLocalPlayer;
class UCharonUIConfig;
/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public :

	UCharonUISubsystem(){};
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	//void InitUIPolicy();
	
	// UFUNCTION(BlueprintCallable, Category = "Charon|UI")
	// void SwitchUIConfig(UCharonUIConfig* NewConfig);
	UFUNCTION(BlueprintCallable, Category = "Charon|UI")
	void SwitchUIPolicy(UCharonUIPolicy* InPolicy);

	virtual void NotifyPlayerAdded(UCharonLocalPlayer* LocalPlayer);
	virtual void NotifyPlayerRemoved(UCharonLocalPlayer* LocalPlayer);
	virtual void NotifyPlayerDestroyed(UCharonLocalPlayer* LocalPlayer);

	// UFUNCTION(BlueprintCallable, Category = "Charon|UI")
	// UCharonUIConfig* GetUIConfig();

	void RegisterUIConfigToPlayer(UCharonLocalPlayer* LocalPlayer, UCharonUIConfig* UIConfig);
	//UCharonRootLayout* GetRootLayout(UCharonLocalPlayer* LocalPlayer);
	UCharonRootLayout* GetRootLayout(const UCharonLocalPlayer* LocalPlayer) const;
protected:

	UPROPERTY()
	const UCharonUIDeveloperSettings* UIDeveloperSettings;
	
	UPROPERTY(Transient)
	TObjectPtr<UCharonUIPolicy> CurrentUIPolicy = nullptr;
	TSubclassOf<UCharonUIPolicy> DefaultUIPolicy;

	
};
