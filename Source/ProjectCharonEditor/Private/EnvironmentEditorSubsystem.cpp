// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentEditorSubsystem.h"



#include "WaterZoneActor.h"
#include "ProjectCharon/Environment/EnvironmentDeveloperSettings.h"
#include "ProjectCharon/Environment/Water/CharonWaterManager.h"



UEnvironmentEditorSubsystem::UEnvironmentEditorSubsystem()
{
}

void UEnvironmentEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EnvironmentDeveloperSettings = GetDefault<UEnvironmentDeveloperSettings>();
	if(EnvironmentDeveloperSettings)
	{
		if(EnvironmentDeveloperSettings->CharonWaterManagerClass)
		{
			CharonWaterManagerClass = EnvironmentDeveloperSettings->CharonWaterManagerClass;
		}
	}
	
	// GEngine이 유효한지 확인하고, 델리게이트에 함수를 바인딩합니다.
	if (GEngine)
	{
		OnLevelActorAddedHandle = GEngine->OnLevelActorAdded().AddUObject(this, &UEnvironmentEditorSubsystem::OnLevelActorAdded);
	}
	
}

void UEnvironmentEditorSubsystem::Deinitialize()
{
	if (GEngine)
	{
		GEngine->OnLevelActorAdded().Remove(OnLevelActorAddedHandle);
	}
	
	Super::Deinitialize();
}

void UEnvironmentEditorSubsystem::OnLevelActorAdded(AActor* Actor)
{
	UWorld* ActorWorld = Actor->GetWorld();
	AWaterZone* WaterZoneActor = Cast<AWaterZone>(Actor);
	if(!Actor->bIsEditorPreviewActor && !Actor->HasAnyFlags(RF_Transient)
		&& ActorWorld && ActorWorld->IsEditorWorld()
		&& WaterZoneActor != nullptr)
	{
		if(!CharonWaterManager.IsValid())
		{
			CharonWaterManager = ActorWorld -> SpawnActor<ACharonWaterManager>(CharonWaterManagerClass);
		}

		
	}
	
	
}
