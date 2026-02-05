// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonUIExtensions.h"

#include "CharonRootLayout.h"
#include "CharonUIPolicy.h"
#include "CharonUISubsystem.h"
#include "CommonActivatableWidget.h"
#include "Player/CharonLocalPlayer.h"

UCommonActivatableWidget* UCharonUIExtensions::PushContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer,
                                                                            FGameplayTag LayerName, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(WidgetClass != nullptr))
	{
		return nullptr;
	}

	if (UCharonUISubsystem* UISubsystem = LocalPlayer->GetGameInstance()->GetSubsystem<UCharonUISubsystem>())
	{
		if (UCharonRootLayout* RootLayout = UISubsystem->GetRootLayout(CastChecked<UCharonLocalPlayer>(LocalPlayer)))
		{
			return RootLayout->PushWidgetToLayerStack(LayerName, WidgetClass);
		
		}
	}

	return nullptr;
}

void UCharonUIExtensions::PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName,
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(!WidgetClass.IsNull()))
	{
		return;
	}

	if (UCharonUISubsystem* UISubsystem = LocalPlayer->GetGameInstance()->GetSubsystem<UCharonUISubsystem>())
	{
		if (UCharonRootLayout* RootLayout = UISubsystem->GetRootLayout(CastChecked<UCharonLocalPlayer>(LocalPlayer)))
		{
			// 아직 입력제한 기능 구현 안했음.
			const bool bSuspendInputUntilComplete = true;
			RootLayout->PushWidgetToLayerStackAsync(LayerName, bSuspendInputUntilComplete, WidgetClass);
			
		}
	}
}

void UCharonUIExtensions::PopContentFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (!ActivatableWidget)
	{
		// Ignore request to pop an already deleted widget
		return;
	}

	if (const ULocalPlayer* LocalPlayer = ActivatableWidget->GetOwningLocalPlayer())
	{
		if (UCharonUISubsystem* UISubsystem = LocalPlayer->GetGameInstance()->GetSubsystem<UCharonUISubsystem>())
		{
			if (UCharonRootLayout* RootLayout = UISubsystem->GetRootLayout(CastChecked<UCharonLocalPlayer>(LocalPlayer)))
			{
				RootLayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
			}
		}
	}
}
