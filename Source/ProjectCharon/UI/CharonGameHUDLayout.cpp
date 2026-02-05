// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonGameHUDLayout.h"

#include "CharonUIExtensions.h"

UCharonGameHUDLayout::UCharonGameHUDLayout() : Super(FObjectInitializer::Get())
{
}

bool UCharonGameHUDLayout::NativeOnHandleBackAction()
{
	

	if (bIsBackHandler)
	{
		// 일단 BP_OnHandleBackAction를 호출해두긴 하는데 이 클래스에서 블루프린트 백 액션을 구현하면 안될듯?
		// 뭐 만에하나 메뉴 붙이는 로직이랑 같이 쓰고 싶은 로직이 있는거 아니면. 
		if (!BP_OnHandleBackAction() && ensure(!EscapeMenuClass.IsNull()))
		{
			UCharonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(), MenuLayerTag, EscapeMenuClass);
		}
		
		return true;
	}
	return false;
}

