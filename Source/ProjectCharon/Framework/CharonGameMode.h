// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CharonGameMode.generated.h"

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
	//virtual void OnPostLogin(AController* NewPlayer) override;
	
public :
	UFUNCTION(BlueprintCallable, Category = "Charon|GameMode")
	void ReportDeath(AController* Player);

protected:
	//virtual void RestartPlayer(AController* NewPlayer) override;
	//TArray<AController*> DeadPlayers;
	

	UFUNCTION(BlueprintCallable, Category = "Charon|GameMode")
	void RegisterPlayer(APlayerController* Player, FCharonPlayerInfo PlayerInfo);
};
