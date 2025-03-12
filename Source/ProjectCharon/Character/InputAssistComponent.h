// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "Components/ActorComponent.h"
#include "Data/CharacterAbilityConfig.h"
#include "InputAssistComponent.generated.h"


class AInputFunctionSet;
struct FGameplayAbilitySpecHandle;


/**
 * 캐릭터의 입력 등록과 교체, 어빌리티나 함수를 바인딩 해주는 입력 보조 컴포넌트
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UInputAssistComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UInputAssistComponent();

private:
	
	
protected:

	TObjectPtr<const UCharonInputConfig> DefaultInputConfig = nullptr;
	TObjectPtr<const UCharonInputConfig> PresentInputConfig = nullptr;

	UPROPERTY()
	TObjectPtr<AInputFunctionSet> DefaultInputFunctions = nullptr;
	
	// //보조 InputConfig 넣을 곳. 설정창 불러오기라던가, 뭔가 추가기능이라던가. 
	//TSet<UCharonInputConfig> SubInputConfigs;
	
	//PresentAbilityConfig의 인풋 바인드 핸들
	TArray<uint32> AbilityBindHandles;
	TArray<uint32> NativeBindHandles;

	
	
	virtual void BeginPlay() override;

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	void ClearAbilityConfig();

	
	
public:
	//컴포넌트 초기화.
	UFUNCTION(BlueprintCallable)
	void InitInputAssist(const UCharonInputConfig* InputConfig, AInputFunctionSet* InInputFunctions);
	
	// InputConfig를 등록. 인풋액션-어빌리티, 인풋액션-함수 쌍을 CharonInputComponent를 통해 Bind.
	UFUNCTION(Client, Reliable, Category="Charon|Input")
	void SwitchInputConfig(const UCharonInputConfig* InputConfig, AInputFunctionSet* InputFunctions);
	void UnregisterInputConfig();

	//디폴트 입력으로 되돌아가기.
	UFUNCTION(BlueprintCallable, Client, Reliable, Category="Charon|Input")
	void ResetToDefaultInputConfig();
	

};
