// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonAIManager.h"

#include "AIController.h"
#include "GameFramework/Character.h"

class UEnhancedPlayerInput;

ACharacter* UCharonAIManager::TrySpawnAICharacter(TSubclassOf<ACharacter> CharacterClass, FTransform Transform, TSubclassOf<AAIController> AIControllerClass)
{
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == ENetMode::NM_Client)
		{
			return nullptr;
		}
		
		if (ACharacter* AICharacter = World->SpawnActor<ACharacter>(CharacterClass, Transform))
		{
			if (AAIController* AIController = World->SpawnActor<AAIController>(AIControllerClass))
			{
				AIController->Possess(AICharacter);
				//AIControllers.Add(AIController);
				
				return AICharacter;
			}
			
			AICharacter->Destroy();
			
		}
	}
	
	return nullptr;
}

