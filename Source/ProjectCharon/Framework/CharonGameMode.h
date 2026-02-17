// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CharonGameMode.generated.h"

class UCharonUIPolicy;
class UCharonRootLayout;
class UCharonUIConfig;
struct FCharonPlayerInfo;
class UCharonPawnData;
class UGameStartingData;



/**
 * 
 */
UCLASS()
class PROJECTCHARON_API ACharonGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	//virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	// virtual void OnPostLogin(AController* NewPlayer) override;
	
public :

	//~AGameModeBase interface
	//virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	//~End of AGameModeBase interface
	
	UFUNCTION(BlueprintCallable, Category = "Charon|GameMode")
	void ReportDeath(AController* Player);

	UFUNCTION(BlueprintCallable, Category = "Charon|GameMode")
	UCharonUIConfig* GetDefaultUIConfig() const {return DefaultUIConfig;};
	// UFUNCTION(BlueprintCallable, Category = "Charon|GameMode")
	// TSubclassOf<UCharonUIPolicy> GetDefaultUIPolicyClass() {return DefaultUIPolicyClass;};
	
protected:
	//virtual void RestartPlayer(AController* NewPlayer) override;
	//TArray<AController*> DeadPlayers;

	virtual void InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer) override;
	
	UFUNCTION(BlueprintCallable, Category = "Charon|GameMode")
	void RegisterPlayer(APlayerController* Player, FCharonPlayerInfo PlayerInfo);

	
	
	//임시
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charon|GameMode")
	TObjectPtr<UCharonUIConfig> DefaultUIConfig;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charon|GameMode")
	// TSubclassOf<UCharonUIPolicy> DefaultUIPolicyClass;


	
};
