// Fill out your copyright notice in the Description page of Project Settings.


#include "DevComponent.h"
#include "Net/UnrealNetwork.h"



// Sets default values for this component's properties
UDevComponent::UDevComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	
	TestDelegate.AddDynamic(this, &UDevComponent::OnTestValueUpdate);
	// ...
}

void UDevComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDevComponent, TestValue);
}

void UDevComponent::OnTestValueUpdate(const float& OldValue)
{
	UE_LOG(LogTemp, Warning, TEXT("TestValue Updated"));
	//UE_LOG(LogTemp, Warning, TEXT("TestValue Updated To %f From %f", TestValue, OldValue));
}


void UDevComponent::OnRep_TestValue(const float& OldValue)
{
	//OnTestValueUpdate(OldValue);
	TestDelegate.Broadcast(OldValue);
}



/*
TArray<FVector> UDevComponent::FillBalls(FVector leftDown, FVector rightUp, int32 radius)
{
	TArray<FVector> ret;

	float xl = (rightUp.X - leftDown.X) / 2;
	float yl = (rightUp.Y - leftDown.Y) / 2;
	float zl = (rightUp.Z - leftDown.Z) / 2;

	float x0 = leftDown.X;
	float y0 = leftDown.Y;
	float z0 = leftDown.Z;

	float xEnd = rightUp.X;
	float yEnd = rightUp.Y;
	float zEnd = rightUp.Z;

	TArray<FVector> Layer0;

	float z = (leftDown.Z + rightUp.Z) / 2;

	for (float y = radius; y + radius <= yl; y += 2 * radius) {
		for (float x = radius; x + radius <= xl; x += 2 * radius) {
			Layer0.Add(FVector(x0 + x, y0 + y, z));
			Layer0.Add(FVector(x0 + x, yEnd - y, z));
			Layer0.Add(FVector(xEnd - x, y0 + y, z));
			Layer0.Add(FVector(xEnd - x, yEnd - y, z));
		}
	}

	for (float zStep = radius * 2; zStep <= zl; zStep += 2 * radius) {
		for (FVector v : Layer0) {
			ret.Add(FVector(v.X, v.Y, z + zStep));
			ret.Add(FVector(v.X, v.Y, z - zStep));
		}
	}

	for (FVector v : Layer0) {
		ret.Add(v);
	}

	return ret;
}
*/
