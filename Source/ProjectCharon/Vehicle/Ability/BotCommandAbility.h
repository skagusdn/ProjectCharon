// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RiderAbility.h"
#include "BotCommandAbility.generated.h"

/**
 *  같이 베히클에 탑승한 봇에게 타겟 어빌리티를 발동하라고 명령하는 Rider 어빌리티.
 */
UCLASS()
class PROJECTCHARON_API UBotCommandAbility : public URiderAbility
{
	GENERATED_BODY()
	
public :
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION(BlueprintCallable, Category="Charon|BotCommandAbility")
	void InjectAbilityInput();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon|BotCommandAbility")
	TSubclassOf<URiderAbility> TargetRiderAbility;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charon|BotCommandAbility")
	int32 TargetRiderIndex = -1;
};
