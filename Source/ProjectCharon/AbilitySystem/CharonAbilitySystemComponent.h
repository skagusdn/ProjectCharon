// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CharonAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCharonAttributeChanged, float, float);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCharonAttributeChanged_Dynamic, float, OldValue, float, NewValue);

/**
 * 커스텀 Ability System Component
 */
UCLASS()
class PROJECTCHARON_API UCharonAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public :
	UCharonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	
	UFUNCTION(BlueprintCallable, Category = "Charon | Ability", DisplayName="BindEventOnAttributeChange")
	void K2_BindEventOnAttributeChange(UObject* EventSource, FGameplayAttribute AttributeToBind, FOnCharonAttributeChanged_Dynamic Event);
	UFUNCTION(BlueprintCallable, Category = "Charon | Ability", DisplayName="UnBindEventOnAttributeChange")
	void K2_UnBindEventOnAttributeChange(FOnCharonAttributeChanged_Dynamic Event);
	void BindEventOnAttributeChange(FGameplayAttribute AttributeToBind, FOnCharonAttributeChanged Event);
	void UnBindEventOnAttributeChange(FDelegateHandle EventHandle);

	

	
	// Input
	void AbilityLocalInputTagPressed(FGameplayTag InputTag);
	void AbilityLocalInputTagReleased(FGameplayTag InputTag);

protected:

	TOptional<FGameplayAttribute> FindAttribute(const FGameplayAttribute& AttributeToFind);
	
	//TMap<FDelegateHandle, FGameplayAttribute> AttributeChangeBindingHandles;
	TMap<FOnCharonAttributeChanged_Dynamic, TPair<FDelegateHandle, FGameplayAttribute>> AttributeChangeBindHandles_Dynamic;
	TMap<FDelegateHandle,FGameplayAttribute> AttributeChangeBindHandles;
};
