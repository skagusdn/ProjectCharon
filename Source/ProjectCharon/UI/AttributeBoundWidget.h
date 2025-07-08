// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "AttributeBoundWidget.generated.h"



/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UAttributeBoundWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable)
	void InitAttributeBoundWidget(UCharonAbilitySystemComponent* TargetASC);

	UFUNCTION(BlueprintImplementableEvent)
	void PostInitAttributeBoundWidget();

	
protected :

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UCharonAbilitySystemComponent> ObservingTargetASC;

	UFUNCTION(BlueprintImplementableEvent)
	void DeinitAttributeBoundWidget();

	virtual void NativeDestruct() override;
};
