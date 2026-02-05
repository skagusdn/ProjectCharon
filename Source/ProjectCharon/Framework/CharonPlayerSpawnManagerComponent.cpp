// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonPlayerSpawnManagerComponent.h"

#include "Logging.h"
#include "GameFramework/GameStateBase.h"


UCharonPlayerSpawnManagerComponent::UCharonPlayerSpawnManagerComponent()
{	
}

void UCharonPlayerSpawnManagerComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	
	if (!Owner || !Owner->IsA(AGameStateBase::StaticClass()))
	{
		UE_LOG(LogCharon, Warning, TEXT("UCharonPlayerSpawnManagerComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
 
		// 컴포넌트 비활성화: Tick 및 기능 차단
		ensureAlwaysMsgf((false), TEXT("UCharonPlayerSpawnManagerComponent can only be attached to GameState! Owner: %s"), Owner ? *Owner->GetName() : TEXT("None"));
		SetActive(false);
	}
}

AActor* UCharonPlayerSpawnManagerComponent::ChoosePlayerStart_Implementation(AController* Player)
{
	return nullptr;
}




