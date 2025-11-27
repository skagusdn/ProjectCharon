// Fill out your copyright notice in the Description page of Project Settings.


#include "TestingSomethingLibrary.h"


bool UTestingSomethingLibrary::TestChangeMaterial(UMaterialInterface* InMaterial,
	UStaticMeshComponent* InStaticMeshComponent)
{
	UMaterialInstanceDynamic* MID =  InStaticMeshComponent->CreateDynamicMaterialInstance(0, InMaterial);
	if(!MID)
	{
		return false;
	}
	
	MID->SetVectorParameterValue("Color", FLinearColor::Yellow);

	return true;
}
