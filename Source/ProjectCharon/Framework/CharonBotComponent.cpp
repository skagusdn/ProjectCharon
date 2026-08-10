// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonBotComponent.h"

#include "AIController.h"
#include "Logging.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"


// Sets default values for this component's properties
UCharonBotComponent::UCharonBotComponent()
{
	

	
}

AAIController* UCharonBotComponent::TrySpawnBot(TSubclassOf<ACharacter> BotCharacterClass, FTransform Transform,
	TSubclassOf<AAIController> BotControllerClass)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;
	AAIController* NewController = GetWorld()->SpawnActor<AAIController>(BotControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if (NewController != nullptr)
	{
		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>();
		check(GameMode);
		
		GameMode->RestartPlayer(NewController);

		
		
		//SpawnedBotList.Add(NewController);
	}
	
	return NewController;
}

void UCharonBotComponent::OnRegister()
{
	Super::OnRegister();
	
	AActor* Owner = GetOwner();
	
	if (!Owner || !Owner->IsA(AGameStateBase::StaticClass()))
	{
		UE_LOG(LogCharon, Warning, TEXT("UCharonBotComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
 
		// 컴포넌트 비활성화: Tick 및 기능 차단
		ensureAlwaysMsgf((false), TEXT("UCharonBotComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
		SetActive(false);
	}
}


