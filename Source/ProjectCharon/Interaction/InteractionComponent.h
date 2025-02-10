// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveInterface.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

/* 캐릭터에서 상호작용 처리하는 컴포넌트.
 *	나중에 UI랑도 연계.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UInteractionComponent();

protected:
	
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	
	TObjectPtr<ACharacter> OwnerCharacter;

	bool bIsInteracting = false;

	TScriptInterface<IInteractiveInterface> InteractingTarget = nullptr;
	
public:
		
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Charon|Interaction")
	void TryInteraction(TScriptInterface<IInteractiveInterface> Target);

	UFUNCTION(BlueprintCallable, Category="Charon|Interaction")
	bool IsInteracting() {return bIsInteracting;};

	UFUNCTION(BlueprintCallable, Category="Charon|Interaction")
	void TryCancelInteraction();
};
