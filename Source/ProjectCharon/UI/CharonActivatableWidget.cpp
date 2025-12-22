// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonActivatableWidget.h"

UCharonActivatableWidget::UCharonActivatableWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

TOptional<FUIInputConfig> UCharonActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case ECharonWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case ECharonWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case ECharonWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case ECharonWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}
