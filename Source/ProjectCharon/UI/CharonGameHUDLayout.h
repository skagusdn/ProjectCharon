// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "CharonGameHUDLayout.generated.h"

/**
 *  Root Layout의 Game 레이어에 부착되어 주요한 HUD를 담당하는 위젯
 */
UCLASS()
class PROJECTCHARON_API UCharonGameHUDLayout : public UCharonActivatableWidget
{
	GENERATED_BODY()

public:
	UCharonGameHUDLayout();

	// virtual void NativeOnInitialized() override;
	// virtual void NativeDestruct() override;

protected:
	//void HandleEscapeAction();

	virtual bool NativeOnHandleBackAction() override;
	
	/**
	 * The menu to be displayed when the user presses the "Pause" or "Escape" button 
	 */
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;

	/**
	 *  메뉴창을 띄울 레이어. 
	 */
	UPROPERTY(EditDefaultsOnly, meta = (Categories = "UI.Layer"))
	FGameplayTag MenuLayerTag;
};
