// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonGameplayStatics.h"

#include "Player/CharonController.h"

float UCharonGameplayStatics::GetServerWorldTime(UObject* WorldContextObject)
{
	if (!WorldContextObject) return 0.f;
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return 0.f;
	UGameInstance* const GameInstance = World->GetGameInstance();
	if (!GameInstance) return 0.f;
	ACharonController* const PlayerController = Cast<ACharonController>(GameInstance->GetFirstLocalPlayerController(World));
	if (!PlayerController) return World->GetTimeSeconds();
	return PlayerController->GetServerWorldTime();
}

float UCharonGameplayStatics::GetServerWorldTimeDelta(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return 0.f;
	}

	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return 0.f;
	}

	UGameInstance* const GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return 0.f;
	}

	ACharonController* const PlayerController = Cast<ACharonController>(GameInstance->GetFirstLocalPlayerController(World));
	if (!PlayerController)
	{
		return 0.f;
	}

	return PlayerController->GetServerWorldTimeDelta();
}
