// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonUIPolicy.h"

#include "CharonRootLayout.h"
#include "CharonUIConfig.h"
#include "CharonUISubsystem.h"
#include "Player/CharonLocalPlayer.h"

UCharonUISubsystem* UCharonUIPolicy::GetOwningUISubsystem()
{
	return CastChecked<UCharonUISubsystem>(GetOuter());
}

UCharonRootLayout* UCharonUIPolicy::GetRootLayout(const UCharonLocalPlayer* LocalPlayer) const
{
	const FRegisteredRootLayoutInfo* LayoutInfo =  RegisteredRootLayouts.FindByKey(LocalPlayer);

	return LayoutInfo ? LayoutInfo->RootLayout : nullptr; 
	
}

void UCharonUIPolicy::NotifyPlayerAdded(UCharonLocalPlayer* LocalPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("UCharonUIPolicy::NotifyPlayerAdded"));


	LocalPlayer->OnPlayerControllerSet.AddWeakLambda(this, [this](UCharonLocalPlayer* CharonLocalPlayer, APlayerController* PlayerController)
	{
		NotifyPlayerRemoved(CharonLocalPlayer);

		if (FRegisteredRootLayoutInfo* LayoutInfo = RegisteredRootLayouts.FindByKey(CharonLocalPlayer))
		{
			AddLayoutToViewport(CharonLocalPlayer, LayoutInfo->RootLayout);
			//LayoutInfo->bAddedToViewport = true;
		}
		else
		{
			CreateLayoutWidget(CharonLocalPlayer);
		}
	});

	if (FRegisteredRootLayoutInfo* LayoutInfo = RegisteredRootLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
		//LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreateLayoutWidget(LocalPlayer);
	}
	
}

void UCharonUIPolicy::NotifyPlayerRemoved(UCharonLocalPlayer* LocalPlayer)
{
	if (FRegisteredRootLayoutInfo* LayoutInfo = RegisteredRootLayouts.FindByKey(LocalPlayer))
	{
		RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayout);
	}
}

void UCharonUIPolicy::NotifyPlayerDestroyed(UCharonLocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	LocalPlayer->OnPlayerControllerSet.RemoveAll(this);
	const int32 LayoutInfoIdx = RegisteredRootLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		UCharonRootLayout* Layout = RegisteredRootLayouts[LayoutInfoIdx].RootLayout;
		RemoveLayoutFromViewport(LocalPlayer, Layout);

		//OnRootLayoutReleased(LocalPlayer, Layout);
	}
}

void UCharonUIPolicy::AddLayoutToViewport(UCharonLocalPlayer* LocalPlayer, UCharonRootLayout* Layout)
{
	//UE_LOG(LogCommonGame, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	//OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UCharonUIPolicy::RemoveLayoutFromViewport(UCharonLocalPlayer* LocalPlayer, UCharonRootLayout* Layout)
{
	// 일단 레이아웃 제거는 플레이어가 실제 게임플레이 월드로 travel하거나 거기서 빠져나올 때 정도만 쓸 거같다.
	// 우선 만들어두는 뒀지만 실제 어떤 로직이 필요할진 가봐야 알듯.
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{		
		Layout->RemoveFromParent();

		const int32 LayoutInfoIdx = RegisteredRootLayouts.IndexOfByKey(LocalPlayer);
		if (LayoutInfoIdx != INDEX_NONE)
		{
			RegisteredRootLayouts.RemoveAt(LayoutInfoIdx);
		}
		//OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UCharonUIPolicy::CreateLayoutWidget(UCharonLocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		//TSubclassOf<UCharonRootLayout> LayoutWidgetClass = GetLayoutWidgetClass(LocalPlayer);
		if (ensure(RootLayoutClass && !RootLayoutClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UCharonRootLayout* NewLayoutObject = CreateWidget<UCharonRootLayout>(PlayerController, RootLayoutClass);
			RegisteredRootLayouts.Emplace(LocalPlayer, NewLayoutObject);
			
			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

void UCharonUIPolicy::RegisterUIConfig(UCharonLocalPlayer* LocalPlayer, UCharonUIConfig* UIConfig)
{
	// 만약 UIConfig 등록 시점에 Layout이 등록되지 않았다면? 그런 상황이 있을 수도... 있을 것 같긴한데..
	// 어차피 이 시점엔 LocalPlayer 있으니까 바로 등록하면 되는거 아니야?
	if (FRegisteredRootLayoutInfo* LayoutInfo = RegisteredRootLayouts.FindByKey(LocalPlayer))
	{
		for(FActivatableWidgetForLayer WidgetInfo : UIConfig->WidgetsToRegisterForLayers)
		{
			LayoutInfo->RootLayout->PushWidgetToLayerStack(WidgetInfo.LayerTag, WidgetInfo.WidgetClass);
		}
		
	}
}
