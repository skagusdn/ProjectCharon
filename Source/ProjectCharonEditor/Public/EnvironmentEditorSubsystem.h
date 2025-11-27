// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "EnvironmentEditorSubsystem.generated.h"

class ACharonWaterManager;
class UEnvironmentDeveloperSettings;
/**
 * 
 */
UCLASS()
class PROJECTCHARONEDITOR_API UEnvironmentEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UEnvironmentEditorSubsystem();
	
	// USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~USubsystem Interface

protected:

	void OnLevelActorAdded(AActor* Actor);

	FDelegateHandle OnLevelActorAddedHandle;

	UPROPERTY()
	const UEnvironmentDeveloperSettings* EnvironmentDeveloperSettings;
	
	TSubclassOf<ACharonWaterManager> CharonWaterManagerClass;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<ACharonWaterManager> CharonWaterManager;
};
