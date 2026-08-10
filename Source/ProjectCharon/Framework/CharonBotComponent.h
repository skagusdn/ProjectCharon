// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharonBotComponent.generated.h"


class AAIController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UCharonBotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UCharonBotComponent();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	AAIController* TrySpawnBot(TSubclassOf<ACharacter> BotCharacterClass, FTransform Transform, TSubclassOf<AAIController> BotControllerClass);
	
protected:

	virtual void OnRegister() override;
public:
	
};
