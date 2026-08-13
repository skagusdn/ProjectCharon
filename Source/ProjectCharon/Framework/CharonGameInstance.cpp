// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonGameInstance.h"

#include "CharonGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
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

void UCharonGameInstance::Init()
{
	Super::Init();
	
	// Register our custom init states -> 라이라에서 가져옴. InitState 등록하기. 
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(CharonGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(CharonGameplayTags::InitState_DataAvailable, false, CharonGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(CharonGameplayTags::InitState_DataInitialized, false, CharonGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(CharonGameplayTags::InitState_GameplayReady, false, CharonGameplayTags::InitState_DataInitialized);
	}
	
}
