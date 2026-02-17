// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CharonUIConfig.generated.h"

class UCommonActivatableWidget;
class UCharonRootLayout;

USTRUCT(BlueprintType)
struct FActivatableWidgetForLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCommonActivatableWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag LayerTag;
};
	

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonUIConfig : public UDataAsset
{
	GENERATED_BODY()

public :
	
	// UPROPERTY(EditAnywhere)
	// TSubclassOf<UCharonRootLayout> RootLayoutClass;

	// // 일단 임시. 
	// UPROPERTY(EditAnywhere)
	// TSubclassOf<UCommonActivatableWidget> HUDLayoutClass;
	UPROPERTY(EditAnywhere)
	TArray<FActivatableWidgetForLayer> WidgetsToRegisterForLayers;
};
