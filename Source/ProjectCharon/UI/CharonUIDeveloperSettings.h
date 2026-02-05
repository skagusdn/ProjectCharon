// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CharonUIDeveloperSettings.generated.h"


class UCharonUIPolicy;
/**
 * 
 */
UCLASS(config=game, defaultconfig, meta=(DisplayName="Charon UI Developer Settings"))
class PROJECTCHARON_API UCharonUIDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public :
	UPROPERTY(EditAnywhere, Config)
	TSubclassOf<UCharonUIPolicy> DefaultUIPolicyClass;
};
