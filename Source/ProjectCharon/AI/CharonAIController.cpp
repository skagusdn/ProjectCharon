// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonAIController.h"

#include "GameFramework/Character.h"
#include "Player/CharonPlayerState.h"


// Sets default values
ACharonAIController::ACharonAIController()
{
	bWantsPlayerState = true;
}

void ACharonAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (ACharonPlayerState* PS = GetPlayerState<ACharonPlayerState>())
	{
		// 플에이어 스테이트에 캐릭터 메시 저장 -> 베히클 탈때 쓸꺼. 추후 로직 바꿀 수 있음.
		if(ACharacter* InCharacter = Cast<ACharacter>(InPawn))
		{
			PS->SetCharacterMesh(InCharacter->GetMesh());
		}
	}
}

