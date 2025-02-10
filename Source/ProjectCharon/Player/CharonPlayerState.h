// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RunAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "CharonPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API ACharonPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "Charon|Abilities")
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;
	
public : 
	ACharonPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ IAbilitySystemInterface 시작
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface 끝

	UFUNCTION(BlueprintCallable)
	UCharonAbilitySystemComponent* GetCharonAbilitySystemComponent() const{return AbilitySystemComponent;};

	
	
};
