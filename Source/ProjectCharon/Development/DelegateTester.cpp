// Fill out your copyright notice in the Description page of Project Settings.


#include "DelegateTester.h"


// Sets default values
ADelegateTester::ADelegateTester()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADelegateTester::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADelegateTester::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADelegateTester::SetDelegate(FTestDelegate InDelegate)
{
	TestDelegate = InDelegate;
}

void ADelegateTester::FireDelegate()
{
	TestDelegate.Execute();
}

