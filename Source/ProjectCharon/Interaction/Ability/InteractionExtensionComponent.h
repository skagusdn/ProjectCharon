// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/ActorComponent.h"
#include "InteractionExtensionComponent.generated.h"


class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UInteractionExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UInteractionExtensionComponent();

	UFUNCTION(BlueprintCallable, Category="Charon|Interaction", meta=(DefaultToSelf="InteractionObject"))
	FGameplayAbilitySpecHandle GrantAbilityAndActivate(UAbilitySystemComponent* AbilitySystemComponent,	UGameplayAbility* InteractAbility,
		TSubclassOf<UGameplayAbility> AbilityForInteraction, bool HasOptionalEventData = false, FGameplayEventData OptionalEventData = FGameplayEventData());
protected:
	
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

	//void HandleAbilityForInteractionEnd(UGameplayAbility* Ability);

public:
	
};
