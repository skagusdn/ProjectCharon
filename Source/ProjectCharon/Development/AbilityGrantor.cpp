// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityGrantor.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"


// Sets default values
AAbilityGrantor::AAbilityGrantor()
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

void AAbilityGrantor::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		OverlapBox -> OnComponentBeginOverlap.AddDynamic(this, &AAbilityGrantor::OnBoxBeginOverlap);
	}
}

void AAbilityGrantor::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(OtherActor);

	
	if(IAbilitySystemInterface* ASCActor = Cast<IAbilitySystemInterface>(OtherActor))
	{
		if(UAbilitySystemComponent* ASC = ASCActor->GetAbilitySystemComponent())
		{
			if(UCharonAbilitySystemComponent* CharonASC = Cast<UCharonAbilitySystemComponent>(ASC))
			{
				if(GrantedHandlesMap.Find(CharonASC))
				{
					GrantedHandlesMap.Find(CharonASC)-> TakeFromAbilitySystem(CharonASC);
					GrantedHandlesMap.Remove(CharonASC);
					return;
				}

				FCharonAbilitySet_GrantedHandles Handles;
				AbilitiesToGrant->GiveToAbilitySystem(CharonASC, &Handles, this);
				GrantedHandlesMap.Add(CharonASC, Handles);
			}
		}
	}
	
}

