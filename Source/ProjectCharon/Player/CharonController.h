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
	
protected:
	virtual void OnPossess(APawn* InPawn) override;

	//virtual void SetupInputComponent() override;
	
	//virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
};
