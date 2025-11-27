// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonWaterBodyRiver.h"





AWaterZone* ACharonWaterBodyRiver::GetOwningWaterZone()
{
	return WaterBodyComponent ? WaterBodyComponent->GetWaterZone() : nullptr;
}


void ACharonWaterBodyRiver::BeginPlay()
{
	Super::BeginPlay();
	
}



