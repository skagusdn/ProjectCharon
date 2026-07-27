// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CharonAIManager.generated.h"

struct FInputActionValue;
class UInputAction;
class AAIController;
/**
 *  봇 플레이어나 몬스터 등 ai를 관리할 매니저 서브시스템. 
 */
UCLASS()
class PROJECTCHARON_API UCharonAIManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	ACharacter* TrySpawnAICharacter(TSubclassOf<ACharacter> CharacterClass, FTransform Transform, TSubclassOf<AAIController> AIControllerClass);
	
	
protected:
	
	//TArray<TObjectPtr<AAIController>> AIControllers;
	
	
	
	
};
