// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonGameLayout.h"

#include "Kismet/GameplayStatics.h"

UCharonGameLayout* UCharonGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetPrimaryGameLayout(PlayerController);
}

UCharonGameLayout* UCharonGameLayout::GetPrimaryGameLayout(APlayerController* PlayerController)
{
	
	return PlayerController ? GetPrimaryGameLayout(PlayerController->GetLocalPlayer()) : nullptr;
}

UCharonGameLayout* UCharonGameLayout::GetPrimaryGameLayout(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		const UCommonLocalPlayer* CommonLocalPlayer = CastChecked<UCommonLocalPlayer>(LocalPlayer);
		if (const UGameInstance* GameInstance = CommonLocalPlayer->GetGameInstance())
		{
			if (UGameUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UGameUIManagerSubsystem>())
			{
				if (const UGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
				{
					if (UPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CommonLocalPlayer))
					{
						return RootLayout;
					}
				}
			}
		}
	}

	return nullptr;
}

UCharonGameLayout::UCharonGameLayout(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}
