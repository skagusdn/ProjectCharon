// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentInteractionManager.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "EnvironmentInteractionLocalPlayerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UEnvironmentInteractionLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public :

	
	
	
protected :
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	
	void SpawnEnvironmentManager();

	UPROPERTY()
	TObjectPtr<AEnvironmentInteractionManager> EnvironmentInteractionManager;
	

	
	
};
