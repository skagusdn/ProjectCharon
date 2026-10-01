// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CharonAIController.generated.h"

UCLASS(BlueprintType)
class PROJECTCHARON_API ACharonAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACharonAIController();
	
protected:
	void OnPossess(APawn* InPawn) override;
	// AI 캐릭터를 단순 디버깅 용 이외로 사용하고자  한다면 Crew시스템에도 추가되게 초기화 로직 넣기.
	// 아니야 이 클래스는 그냥 커스텀 ai 컨트롤러의 최상단 클래스로 쓰고, 나중에 봇 전용 로직 추가하고 싶으면
	// 따로 이 클래스를 상속해서 전용 클래스 하나 더 만들자. 
	
};
