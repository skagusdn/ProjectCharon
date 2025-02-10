// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CharonAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public :
	UCharonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
		
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DevelopmentOnly))
	void TestGiveAbility(TSubclassOf<UGameplayAbility> AbilityClass, FGameplayTag InputTag);

	
	
	// Input
	void AbilityLocalInputTagPressed(FGameplayTag InputTag);
	void AbilityLocalInputTagReleased(FGameplayTag InputTag);

protected:

};
