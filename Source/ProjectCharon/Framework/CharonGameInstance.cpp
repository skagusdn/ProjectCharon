// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonGameInstance.h"

#include "Player/CharonLocalPlayer.h"
#include "UI/CharonUISubsystem.h"

int32 UCharonGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId)
{
	int32 ReturnVal = Super::AddLocalPlayer(NewPlayer, UserId);
	if (ReturnVal != INDEX_NONE)
	{
		GetSubsystem<UCharonUISubsystem>()->NotifyPlayerAdded(Cast<UCharonLocalPlayer>(NewPlayer));
	}
	
	return ReturnVal;
}

bool UCharonGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	GetSubsystem<UCharonUISubsystem>()->NotifyPlayerDestroyed(Cast<UCharonLocalPlayer>(ExistingPlayer));

	return Super::RemoveLocalPlayer(ExistingPlayer);
}
