// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonWaterBodyLake.h"



// ACharonWaterBodyLake::ACharonWaterBodyLake()
// {
// 	
// 	PrimaryActorTick.bCanEverTick = true;
// }

AWaterZone* ACharonWaterBodyLake::GetOwningWaterZone()
{
	return WaterBodyComponent ? WaterBodyComponent->GetWaterZone() : nullptr;
}

