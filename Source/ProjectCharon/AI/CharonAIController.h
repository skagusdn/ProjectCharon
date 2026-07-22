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

	// AI 캐릭터를 단순 디버깅 용 이외로 사용하고자  한다면 Crew시스템에도 추가되게 초기화 로직 넣기. 
	
};
