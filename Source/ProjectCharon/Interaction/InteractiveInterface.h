// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "UObject/Interface.h"
#include "InteractiveInterface.generated.h"

UENUM(BlueprintType)
enum class EInteractResult : uint8
{
	InteractFail,
	InteractEnd,
	InteractInProgress
};

UENUM(BlueprintType)
enum class EInteractionInputPolicy : uint8
{
	Press,
	Hold,
	Toggle
};

// UENUM(BlueprintType)
// enum class EInteractionActivationPolicy : uint8
// {
// 	CallFunction	UMETA(DisplayName = "Call Function on Target"),
// 	GrantAndActivateAbility	UMETA(DisplayName = "Grant Ability to Player and Activate Ability"),
// 	ActivateOwningAbility	UMETA(DisplayName = "Active Ability of Interactve Object"),
// };

// 근처에 갔을 때 뜨는 상호작용 UI에 표시할 텍스트 및 색깔 등. 일단 임시. 
USTRUCT(BlueprintType)
struct FInteractionDescription
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FText DescriptionText;
	UPROPERTY(BlueprintReadWrite)
	FColor UITextColor;
	UPROPERTY(BlueprintReadWrite)
	FColor UIBackgroundColor;
};

USTRUCT(BlueprintType)
struct FInteractionResultWithAbilityData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EInteractResult InteractionResult;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityForInteraction;

	UPROPERTY(BlueprintReadWrite)
	FGameplayEventData EventData;
	
};

USTRUCT(BlueprintType)
struct FInteractionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> InteractiveObject;

	UPROPERTY(BlueprintReadWrite)
	EInteractionInputPolicy InteractionInputPolicy;
	
	// UPROPERTY(BlueprintReadWrite)
	// EInteractionActivationPolicy InteractionActivationPolicy;
	
	UPROPERTY(BlueprintReadWrite)
	FInteractionDescription DescriptionData;

	// // 상호작용에 어빌리티를 사용하는 타입일 경우. 
	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// TSubclassOf<class UGameplayAbility> InteractionAbility;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractiveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTCHARON_API IInteractiveInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Charon|Interaction")
	FInteractionResultWithAbilityData Interact(ACharacter* Character, UCharonGameplayAbility* InteractionAbility);

	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Charon|Interaction")
	// void CancelInteraction(ACharacter* Character);
	//
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Charon|Interaction")
	// bool CanInteract(ACharacter* Character);

	// // 상호작용을 진행하거나 끝내기 위해서 한 번 더 입력이 필요한 타입일때 호출할 함수. 
	// UFUNCTION(BlueprintImplementableEvent, Category = "Charon|Interaction")
	// void PressInputOnceMore(ACharacter* Character);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Charon|Interaction")
	FInteractionInfo GetInteractionInfo() const;
};
