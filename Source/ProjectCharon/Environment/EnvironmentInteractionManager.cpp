// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentInteractionManager.h"


// Sets default values
AEnvironmentInteractionManager::AEnvironmentInteractionManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnvironmentInteractionManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnvironmentInteractionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

