// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonUISubsystem.h"

//#include "CharonGameLayout.h"
#include "CharonRootLayout.h"
#include "CharonUIConfig.h"
#include "CharonUIDeveloperSettings.h"
#include "CharonUIPolicy.h"
#include "IMediaControls.h"
#include "Blueprint/UserWidget.h"
#include "Framework/CharonGameMode.h"
#include "Player/CharonLocalPlayer.h"

void UCharonUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	//InitUIPolicy();
	UIDeveloperSettings = GetDefault<UCharonUIDeveloperSettings>();
	if(UIDeveloperSettings)
	{
		if(UIDeveloperSettings->DefaultUIPolicyClass)
		{
			if(!CurrentUIPolicy)
			{
				DefaultUIPolicy = UIDeveloperSettings->DefaultUIPolicyClass;
				SwitchUIPolicy(NewObject<UCharonUIPolicy>(this, DefaultUIPolicy));
			}
			
		}
	}
	
	//SwitchUIConfig(DefaultUIConfig);
}

void UCharonUISubsystem::Deinitialize()
{
	Super::Deinitialize();
	//SwitchUIConfig(nullptr);
	SwitchUIPolicy(nullptr);
}


void UCharonUISubsystem::SwitchUIPolicy(UCharonUIPolicy* InPolicy)
{
	UE_LOG(LogTemp, Display, TEXT("SwitchUIPolicy"));//
	CurrentUIPolicy = InPolicy;
}

// void UCharonUISubsystem::SwitchUIConfig(UCharonUIConfig* NewConfig)
// {
// 	if(CurrentUIConfig != NewConfig)
// 	{
// 		CurrentUIConfig = NewConfig;
// 	}
// }

// void UCharonUISubsystem::NotifyPlayerAdded(UCharonLocalPlayer* LocalPlayer)
// {
// 	LocalPlayer->OnPlayerControllerSet.AddWeakLambda(this, [this](UCharonLocalPlayer* LocalPlayer, APlayerController* PlayerController)
// 	{
// 		NotifyPlayerRemoved(LocalPlayer);
//
// 		if(APlayerController* Player = LocalPlayer->GetPlayerController(GetWorld()))
// 		{
// 			if(!CurrentRootLayout)
// 			{
// 				CurrentRootLayout = CreateWidget<UCharonRootLayout>(Player, DefaultUIConfig->RootLayout);
// 			}
// 			CurrentRootLayout->SetPlayerContext(LocalPlayer);
// 			CurrentRootLayout->AddToPlayerScreen(1000);
// 			
// 		}
// 		
// 	});
//
// 	if(APlayerController* Player = LocalPlayer->GetPlayerController(GetWorld()))
// 	{
// 		if(!CurrentRootLayout)
// 		{
// 			CurrentRootLayout = CreateWidget<UCharonRootLayout>(Player, DefaultUIConfig->RootLayout);
// 		}
// 		CurrentRootLayout->SetPlayerContext(LocalPlayer);
// 		CurrentRootLayout->AddToPlayerScreen(1000);
// 	}
// 	
// }

void UCharonUISubsystem::NotifyPlayerAdded(UCharonLocalPlayer* LocalPlayer)
{
	UE_LOG(LogTemp, Display, TEXT("UCharonUISubsystem::NotifyPlayerAdded"));//

	if (ensure(LocalPlayer) && CurrentUIPolicy)
	{
		CurrentUIPolicy->NotifyPlayerAdded(LocalPlayer);
	}
}

void UCharonUISubsystem::NotifyPlayerRemoved(UCharonLocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentUIPolicy)
	{
		CurrentUIPolicy->NotifyPlayerRemoved(LocalPlayer);
	}
}

void UCharonUISubsystem::NotifyPlayerDestroyed(UCharonLocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentUIPolicy)
	{
		CurrentUIPolicy->NotifyPlayerDestroyed(LocalPlayer);
	}
}

void UCharonUISubsystem::RegisterUIConfigToPlayer(UCharonLocalPlayer* LocalPlayer, UCharonUIConfig* UIConfig)
{
	if (ensure(LocalPlayer) && CurrentUIPolicy)
	{
		CurrentUIPolicy->RegisterUIConfig(LocalPlayer, UIConfig);
	}
}

// UCharonRootLayout* UCharonUISubsystem::GetRootLayout(UCharonLocalPlayer* LocalPlayer)
// {
// 	if (ensure(LocalPlayer) && CurrentUIPolicy)
// 	{
// 		return CurrentUIPolicy->GetRootLayout(LocalPlayer);
// 	}
// 	return nullptr;
// }

UCharonRootLayout* UCharonUISubsystem::GetRootLayout(const UCharonLocalPlayer* LocalPlayer) const
{
	if (ensure(LocalPlayer) && CurrentUIPolicy)
	{
		return CurrentUIPolicy->GetRootLayout(LocalPlayer);
	}
	return nullptr;
}

// UCharonUIConfig* UCharonUISubsystem::GetUIConfig()
// {
// 	if(GetWorld() && GetWorld()->GetAuthGameMode())
// 	{
// 		if(ACharonGameMode* GameMode = Cast<ACharonGameMode>(GetWorld()->GetAuthGameMode()))
// 		{
// 			return GameMode-> GetDefaultUIConfig();
// 		}
// 	}
// 	return nullptr;
// }

// void UCharonUISubsystem::InitUIConfig()
// {
// 	UCharonLocalPlayer* LocalPlayer = Cast<UCharonLocalPlayer>(GetLocalPlayer());
//
// 	if(!LocalPlayer)
// 	{
// 		return;
// 	}
//
// 	if(CurrentUIConfig)
// 	{
// 		if(CurrentRootLayout)
// 		{
// 			->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
// 			Layout->AddToPlayerScreen(1000);
// 		}
// 	}
// 	
//
// 	
// 	
// }
