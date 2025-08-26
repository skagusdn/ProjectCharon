// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "Components/ActorComponent.h"
#include "Data/CharacterAbilityConfig.h"
#include "InputAssistComponent.generated.h"

class AVehicle;
class AInputFunctionSet;
struct FGameplayAbilitySpecHandle;

//
// USTRUCT(BlueprintType)
// struct FInputLayerHandle
// {
// 	GENERATED_BODY()
//
// 	explicit FInputLayerHandle(const uint64 InValue) : Value(InValue) {};
// 	
// 	bool IsValid() const { return Value != 0; }
// 	bool operator==(const FInputLayerHandle& Other) const { return Value == Other.Value; }
// 	
// private :
// 	uint64 Value = 0;
// 	
// };
//
// USTRUCT(BlueprintType)
// struct FInputLayer
// {
// 	GENERATED_BODY()
// 	
// 	TObjectPtr<UCharonInputConfig> InputConfig;
// 	TObjectPtr<AInputFunctionSet> InputFunctionSet;
// 	FInputLayerHandle LayerHandle;
// };
//
// USTRUCT(Blueprintable)
// struct FInputLayerCondition
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(BlueprintReadWrite)
// 	TObjectPtr<UObject> RequiredObject;
// 	UPROPERTY(BlueprintReadWrite)
// 	FGameplayTagContainer RequiredTags;
// 	UPROPERTY(BlueprintReadWrite)
// 	TSubclassOf<AVehicle> RequiredVehicle;
// 	UPROPERTY(BlueprintReadWrite)
// 	TSubclassOf<UCharonGameplayAbility> RequiredAbility;
// };


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
	virtual void BeginPlay() override;

	void ClearAbilityConfig();

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	TObjectPtr<const UCharonInputConfig> DefaultInputConfig = nullptr;
	TObjectPtr<const UCharonInputConfig> PresentInputConfig = nullptr;

	TObjectPtr<const UCharonInputConfig> TemporaryInputConfig = nullptr;
	
	UPROPERTY()
	TObjectPtr<AInputFunctionSet> DefaultInputFunctions = nullptr;
	TObjectPtr<AInputFunctionSet> PresentInputFunctions = nullptr;

	TObjectPtr<AInputFunctionSet> TemporaryInputFunctions = nullptr;
	
	// //보조 InputConfig 넣을 곳. 설정창 불러오기라던가, 뭔가 추가기능이라던가. 
	//TSet<UCharonInputConfig> SubInputConfigs;
	
	//PresentAbilityConfig의 인풋 바인드 핸들
	TArray<uint32> AbilityBindHandles;
	TArray<uint32> NativeBindHandles;
	
	//TArray<const FInputLayer> InputLayers;
	
	

	
	
public:
	//컴포넌트 초기화.
	UFUNCTION(BlueprintCallable)
	void InitInputAssist(const UCharonInputConfig* InputConfig, AInputFunctionSet* InInputFunctions);
	
	// InputConfig를 등록. 인풋액션-어빌리티, 인풋액션-함수 쌍을 CharonInputComponent를 통해 Bind.
	UFUNCTION(Client, Reliable)
	void SwitchInputConfig(const UCharonInputConfig* InputConfig, AInputFunctionSet* InputFunctions, bool bIsTemporary = false);
	void UnregisterInputConfig(bool bIsTemporary);

	//메인이 아닌 임시적인 인풋 모드 교체. Vehicle에 타는건 메인. 일부 어빌리티의 타겟팅이나 견착 모드같은건 임시. 
	UFUNCTION(BlueprintCallable, Category="Charon|Input", DisplayName="SwitchTemporaryInputConfig")
	void K2_SwitchTemporaryInputConfig(const UCharonInputConfig* InputConfig, AInputFunctionSet* InputFunctions);
	//UFUNCTION(Client, Reliable)
	//void SwitchTemporaryInputConfig(const UCharonInputConfig* InputConfig, AInputFunctionSet* InputFunctions);
	UFUNCTION(BlueprintCallable, Category="Charon|Input", DisplayName="UnregisterTemporaryInputConfig")
	void K2_UnregisterTemporaryInputConfig(const UCharonInputConfig* InputConfig, AInputFunctionSet* InputFunctions);
	//void UnregisterTemporaryInputConfig();
	
	//디폴트 입력으로 되돌아가기.
	UFUNCTION(BlueprintCallable, Client, Reliable, Category="Charon|Input")
	void ResetToDefaultInputConfig();
	

};
