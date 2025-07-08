// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeBoundWidget.h"

void UAttributeBoundWidget::InitAttributeBoundWidget(UCharonAbilitySystemComponent* TargetASC)
{
	check(TargetASC);
	
	ObservingTargetASC = TargetASC;

	PostInitAttributeBoundWidget();
}

void UAttributeBoundWidget::NativeDestruct()
{
	DeinitAttributeBoundWidget();
	
	Super::NativeDestruct();
}
