// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharonPlayerSpawnManagerComponent.generated.h"

/*
 *
 * 굳이 쓰지 말자. 나중에 삭제.
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UCharonPlayerSpawnManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UCharonPlayerSpawnManagerComponent();

protected:
	virtual void OnRegister() override;

	UFUNCTION(BlueprintNativeEvent)
	AActor* ChoosePlayerStart(AController* Player);

	friend class ACharonGameMode;
public:
	
};
