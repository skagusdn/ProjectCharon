// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CharonGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
};
