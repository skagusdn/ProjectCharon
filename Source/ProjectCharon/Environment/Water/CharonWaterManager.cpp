// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonWaterManager.h"


// Sets default values
ACharonWaterManager::ACharonWaterManager()
{
	TestNumber = 0;
	
}

// Called when the game starts or when spawned
void ACharonWaterManager::BeginPlay()
{
	Super::BeginPlay();
	
}

#if WITH_EDITOR	
void ACharonWaterManager::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if(bFinished)
	{
		TestNumber++;
	}
	
}
#endif


