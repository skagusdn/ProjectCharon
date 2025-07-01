// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EnvironmentDeveloperSettings.generated.h"

class AEnvironmentInteractionManager;
/**
 * 
 */
UCLASS(config=game, defaultconfig, meta=(DisplayName="Environment Interaction Developer Settings"))
class PROJECTCHARON_API UEnvironmentDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, config, Category = "Environment Interaction Manager")
	TSoftClassPtr<AEnvironmentInteractionManager> EnvironmentManagerClass;
};
