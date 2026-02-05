// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonRootLayout.h"

#include "CharonActivatableWidget.h"
#include "CharonUISubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CharonLocalPlayer.h"

// UCharonGameLayout* UCharonGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
// {
// 	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
// 	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
// 	return GetPrimaryGameLayout(PlayerController);
// }
//
// UCharonGameLayout* UCharonGameLayout::GetPrimaryGameLayout(APlayerController* PlayerController)
// {
// 	
// 	return PlayerController ? GetPrimaryGameLayout(PlayerController->GetLocalPlayer()) : nullptr;
// }
//
// UCharonGameLayout* UCharonGameLayout::GetPrimaryGameLayout(ULocalPlayer* LocalPlayer)
// {
// 	if (LocalPlayer)
// 	{
// 		const UCommonLocalPlayer* CommonLocalPlayer = CastChecked<UCommonLocalPlayer>(LocalPlayer);
// 		if (const UGameInstance* GameInstance = CommonLocalPlayer->GetGameInstance())
// 		{
// 			if (UGameUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UGameUIManagerSubsystem>())
// 			{
// 				if (const UGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
// 				{
// 					if (UPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CommonLocalPlayer))
// 					{
// 						return RootLayout;
// 					}
// 				}
// 			}
// 		}
// 	}
//
// 	return nullptr;
// }

UCharonRootLayout* UCharonRootLayout::GetRootLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetRootLayout(PlayerController);
}

UCharonRootLayout* UCharonRootLayout::GetRootLayout(APlayerController* PlayerController)
{
	return PlayerController ? GetRootLayout(PlayerController->GetLocalPlayer()) : nullptr;
}

UCharonRootLayout* UCharonRootLayout::GetRootLayout(ULocalPlayer* LocalPlayer)
{

	if (LocalPlayer)
	{
		const UCharonLocalPlayer* CommonLocalPlayer = CastChecked<UCharonLocalPlayer>(LocalPlayer);
		if (const UGameInstance* GameInstance = CommonLocalPlayer->GetGameInstance())
		{
			if (UCharonUISubsystem* UISubsystem = GameInstance->GetSubsystem<UCharonUISubsystem>())
			{
				return UISubsystem->GetRootLayout(CommonLocalPlayer);
			}
		}
	}
	
	return nullptr;
}

UCharonRootLayout::UCharonRootLayout(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UCharonRootLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	for(const auto& LayerTuple : Layers)
	{
		LayerTuple.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* UCharonRootLayout::GetLayerWidget(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}

void UCharonRootLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if(!IsDesignTime())
	{
		Layers.Add(LayerTag, LayerWidget);
	}
	
}

// void UCharonRootLayout::TestPushWidgetToLayer(TSubclassOf<UCharonActivatableWidget> TestWidgetClass)
// {
// 	if(TestLayer)
// 	{
// 		//Test_PushWidgetToLayerStack(TestWidgetClass, [](UCharonActivatableWidget&) {});
// 		Test_PushWidgetToLayerStack(TestWidgetClass);
// 	}
// }

