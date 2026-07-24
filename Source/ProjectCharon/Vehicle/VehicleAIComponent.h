// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleAIComponent.generated.h"

class ACharonAIController;
class AVehicle;
/* 
 * 베히클 관련 AI 캐릭터 생성을 맡을 컴포넌트. 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UVehicleAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UVehicleAIComponent();
	virtual void BeginPlay() override;

protected:
	

	virtual void OnRegister() override;
	
	void SpawnAIRiders();
	void ForceAIRide(ACharacter* Rider);
	
	TArray<TObjectPtr<ACharacter>> AIRiders;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AVehicle> OwnerVehicle;
	
	// 일단 임시로 스폰할 AI캐릭터, AI컨트롤러 클래스는 수동지정.
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharacter> AICharacterClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharonAIController> AIControllerClass;
	
public:
	
};
