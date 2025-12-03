// Fill out your copyright notice in the Description page of Project Settings.


#include "EffectGrantor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/BoxComponent.h"

// Sets default values
AEffectGrantor::AEffectGrantor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComp);

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComponent"));
	OverlapBox -> SetupAttachment(GetRootComponent());
	// 모든 채널에 : 모든걸 무시하겠따
	OverlapBox -> SetCollisionResponseToAllChannels(ECR_Ignore);
	// 폰 채널에 : 폰은 오버랩하게따
	OverlapBox -> SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	
	
}

// Called when the game starts or when spawned
void AEffectGrantor::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		OverlapBox -> OnComponentBeginOverlap.AddDynamic(this, &AEffectGrantor::OnBoxBeginOverlap);
	}
	
}

void AEffectGrantor::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(const IAbilitySystemInterface* AscInterface = dynamic_cast<IAbilitySystemInterface*>(OtherActor))
	{
		if(UAbilitySystemComponent * OtherASC = AscInterface->GetAbilitySystemComponent())
		{
			const FGameplayEffectContextHandle ContextHandle = OtherASC -> MakeEffectContext();
			const FGameplayEffectSpecHandle SpecHandle = OtherASC -> MakeOutgoingSpec(EffectToGrant, 1.f, ContextHandle);
			OtherASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	
			//Destroy();
		}
	}

	
}




