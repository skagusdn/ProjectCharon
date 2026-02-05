// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Player/CharonLocalPlayer.h"
#include "UObject/Object.h"
#include "CharonUIPolicy.generated.h"

class UCharonLocalPlayer;
class UCharonUIConfig;
class UCharonRootLayout;
class UCharonUISubsystem;

USTRUCT()
struct FRegisteredRootLayoutInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCharonRootLayout> RootLayout = nullptr;

	FRegisteredRootLayoutInfo(){};
	FRegisteredRootLayoutInfo(ULocalPlayer* InLocalPlayer, UCharonRootLayout* InRootLayout)
		: LocalPlayer(InLocalPlayer), RootLayout(InRootLayout)
	{};

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
	
};


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, Within = CharonUISubsystem)
class PROJECTCHARON_API UCharonUIPolicy : public UObject
{
	GENERATED_BODY()

public :

	UCharonUISubsystem* GetOwningUISubsystem();
	
	UCharonRootLayout* GetRootLayout(const UCharonLocalPlayer* LocalPlayer) const;
protected :
	// UI Subsystem 쪽에서 호출하는 신호 함수. 
	// Player Removed랑 Destroyed는 왜 나눴지? 로컬 플레이어가 여러 명일 경우 때문인가?
	void NotifyPlayerAdded(UCharonLocalPlayer* LocalPlayer);
	void NotifyPlayerRemoved(UCharonLocalPlayer* LocalPlayer);
	void NotifyPlayerDestroyed(UCharonLocalPlayer* LocalPlayer);
	
	void AddLayoutToViewport(UCharonLocalPlayer* LocalPlayer, UCharonRootLayout* Layout);
	void RemoveLayoutFromViewport(UCharonLocalPlayer* LocalPlayer, UCharonRootLayout* Layout);
	
	void CreateLayoutWidget(UCharonLocalPlayer* LocalPlayer);

	void RegisterUIConfig(UCharonLocalPlayer* LocalPlayer, UCharonUIConfig* UIConfig);
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCharonRootLayout> RootLayoutClass;

	UPROPERTY(Transient)
	TArray<FRegisteredRootLayoutInfo> RegisteredRootLayouts;

	friend class UCharonUISubsystem;
};
