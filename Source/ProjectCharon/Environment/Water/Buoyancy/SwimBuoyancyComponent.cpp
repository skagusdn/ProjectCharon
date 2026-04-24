// Fill out your copyright notice in the Description page of Project Settings.


#include "SwimBuoyancyComponent.h"

#include "Logging.h"
//#include "MovieSceneTracksComponentTypes.h"
#include "WaterBodyActor.h"
#include "Components/SphereComponent.h"
//#include "Subsystems/PropertyVisibilityOverrideSubsystem.h"


// Sets default values for this component's properties
USwimBuoyancyComponent::USwimBuoyancyComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WaterCheckPontoonIndex = -1;
	SwimCheckPontoonIndex = -1;

	bDrawDebugPontoonShapes = false;
	PrimaryComponentTick.bStartWithTickEnabled = true;// 틱이 제대로 안되는 것 같아서 시작부터 틱켜지게.
	bUpdatePontoonsOutOfWater = true;
}

void USwimBuoyancyComponent::BeginPlay()
{
	Super::BeginPlay();

	if(bDrawDebugPontoonShapes)
	{
		for(const FSphericalPontoon& Pontoon : BuoyancyData.Pontoons)
		{
			if(bDrawDebugPontoonShapes && GetOwner() && GetOwner()->GetRootComponent())
			{

				USphereComponent* DebugSphere = NewObject<USphereComponent>(GetOwner());
					
				DebugSphere->SetSphereRadius(Pontoon.Radius);
				DebugSphere->SetRelativeLocation(Pontoon.RelativeLocation);
				DebugSphere->ShapeColor = FColor::Emerald;
				DebugSphere->SetLineThickness(1.f);
				
				DebugSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				DebugSphere->RegisterComponent();
					
				DebugSphere->SetHiddenInGame(false);
				DebugSphere->bIsEditorOnly = true;
				DebugSphere->MarkAsEditorOnlySubobject();
					
				DebugSphereComponents.Add(DebugSphere);
			
				//UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent DebugSphere Attached on %s"), *Pontoon.CenterLocation.ToString());//
			}
		}

		OnEnteredWaterDelegate.AddDynamic(this, &USwimBuoyancyComponent::NotifyDebugPontoonEnteredWater);
		OnExitedWaterDelegate.AddDynamic(this, &USwimBuoyancyComponent::NotifyDebugPontoonExitedWater);
	}

	if(InitCheckPontoons())
	{
		// SwimCheckPontoon이 물에 들어가고 나올때 체크하기 위해서.
		OnEnteredWaterDelegate.AddDynamic(this, &USwimBuoyancyComponent::USwimBuoyancyComponent::CheckSwimPontoonEnteredWater);
		OnExitedWaterDelegate.AddDynamic(this, &USwimBuoyancyComponent::USwimBuoyancyComponent::CheckSwimPontoonExitedWater);
		
	}

	//
	if(GetWorld())
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([This = this]()->void
		{
			if(This->WaterCheckPontoonIndex > 0 && This->BuoyancyData.Pontoons.Num() > This->WaterCheckPontoonIndex)
			{
				FSphericalPontoon& WaterCheckPontoon = This->BuoyancyData.Pontoons[This->WaterCheckPontoonIndex];
				if(This->bDrawDebugPontoonShapes)
				{
					if(USphereComponent* DebugSphere = This->DebugSphereComponents[This->WaterCheckPontoonIndex])
					{
						TArray<AActor*> OutActors;
						DebugSphere->GetOverlappingActors(OutActors);

						bool bOverlappingWaterBody = false;
						for(AActor* Actor : OutActors)
						{
							if(Cast<AWaterBody>(Actor))
							{
								bOverlappingWaterBody = true;
								break;
							}
						}

						if(bOverlappingWaterBody)
						{
							UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent Test WaterCheckPontoon IS overlapping WaterBody"));	
						}
						else
						{
							UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent Test WaterCheckPontoon is NOT overlapping WaterBody"));
						}
					}
				}
				
				UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent Test WaterCheckPontoon Immersion Depth : %f, WaterHeight : %f, CenterLocation : %s, PontoonRadius : %f ,SwimImmersionDepth : %f, LocalForce : %s"),
					WaterCheckPontoon.ImmersionDepth, WaterCheckPontoon.WaterHeight, *WaterCheckPontoon.CenterLocation.ToString(), WaterCheckPontoon.Radius, This->GetImmersionDepth(),*WaterCheckPontoon.LocalForce.ToString());	
			}
			
		});
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.f, true);
	}
	//
	
	
}

bool USwimBuoyancyComponent::InitCheckPontoons()
{
	if(SwimCheckPontoonIndex >= BuoyancyData.Pontoons.Num() || SwimCheckPontoonIndex < 0)
	{
		UE_LOG(LogCharon, Error, TEXT("USwimBuoyancyComponent Error : SwimCheckPontoonIndex is Invalid. "));
		return false;
	}
	
	SwimCheckPontoon = BuoyancyData.Pontoons[SwimCheckPontoonIndex];
	
	if(WaterCheckPontoonIndex >= BuoyancyData.Pontoons.Num() || WaterCheckPontoonIndex < 0)
	{
		UE_LOG(LogCharon, Error, TEXT("USwimBuoyancyComponent Error : WaterCheckPontoonIndex is Invalid."));
		return false;
	}

	WaterCheckPontoon = BuoyancyData.Pontoons[WaterCheckPontoonIndex];
	
	return true;
}

#if WITH_EDITOR
void USwimBuoyancyComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USwimBuoyancyComponent, WaterCheckPontoonIndex))
	{
		if (WaterCheckPontoonIndex >= BuoyancyData.Pontoons.Num())
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("인덱스가 폰툰 사이즈보다 큼. 먼저 폰툰 생성!")));
			//SwimBasePontoonIndex = -1;
		}		
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USwimBuoyancyComponent, SwimCheckPontoonIndex))
	{
		if (SwimCheckPontoonIndex >= BuoyancyData.Pontoons.Num())
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("인덱스가 폰툰 사이즈보다 큼. 먼저 폰툰 생성!")));
		}		
	}

	// 디버깅용 구체 on/off
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USwimBuoyancyComponent, bDrawDebugPontoonShapes))
	{
		if(bDrawDebugPontoonShapes)
		{
			for(USphereComponent* DebugSphere : DebugSphereComponents)
			{
				DebugSphere->SetHiddenInGame(false);
			}
		}
		else
		{
			for(USphereComponent* DebugSphere : DebugSphereComponents)
			{
				DebugSphere->SetHiddenInGame(true);
			}
		}
	}

}

void USwimBuoyancyComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	const FEditPropertyChain& Chain = PropertyChangedEvent.PropertyChain;
	const FName HeadNodeName = Chain.GetHead()->GetValue()->GetFName();

	// //
	// UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent Test Chain Start"));
	// FEditPropertyChain::TDoubleLinkedListNode* CurNode = Chain.GetHead();
	// while(CurNode)
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent Test Node Name : %s"), *CurNode->GetValue()->GetFName().ToString());
	// 	CurNode = CurNode->GetNextNode();
	// }
	// UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent Test Chain End"));
	// //
	
	if(HeadNodeName == GET_MEMBER_NAME_CHECKED(USwimBuoyancyComponent, BuoyancyData ))
	{
		if(WaterCheckPontoonIndex >= BuoyancyData.Pontoons.Num() || WaterCheckPontoonIndex < 0)
		{
			const FName SwimBasePontoonIndexName = GET_MEMBER_NAME_CHECKED(USwimBuoyancyComponent, WaterCheckPontoonIndex);
		
			FString Message = FString::Printf(TEXT("경고 : 폰툰 개수(%d) 범위 안에 인덱스 %s(%d)가 없습니다."), BuoyancyData.Pontoons.Num(), *SwimBasePontoonIndexName.ToString(), WaterCheckPontoonIndex);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
			//SwimBasePontoonIndex = -1;
		}

		if(SwimCheckPontoonIndex >= BuoyancyData.Pontoons.Num() || SwimCheckPontoonIndex < 0)
		{
			const FName SwimBasePontoonIndexName = GET_MEMBER_NAME_CHECKED(USwimBuoyancyComponent, WaterCheckPontoonIndex);
		
			FString Message = FString::Printf(TEXT("경고 : 폰툰 개수(%d) 범위 안에 인덱스 %s(%d)가 없습니다."), BuoyancyData.Pontoons.Num(), *SwimBasePontoonIndexName.ToString(), SwimCheckPontoonIndex);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
			//SwimBasePontoonIndex = -1;
		}

		// 다이나믹하게 에디터 타임에 디버깅 컴포넌트 만들어보려다 실패함 👇👇👇
		
		// UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent Testing Owner : %s"), GetOwner()? *GetOwner()->GetFName().ToString() : *FString("None"));//
		// // 디버깅용 구체
		// if(bDrawDebugPontoonShapes && GetOwner() && GetOwner()->GetRootComponent())
		// {
		// 	
		// 	for(int i = DebugSphereComponents.Num() - 1; i >= 0; i ++)
		// 	{
		// 		USphereComponent* DebugSphere = DebugSphereComponents[i];
		// 		DebugSphere->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		// 		DebugSphere->DestroyComponent();
		// 	}
		// 	DebugSphereComponents.Empty();
		// 	//
		// 	
		// 	for(FSphericalPontoon Pontoon : BuoyancyData.Pontoons)
		// 	{
		// 		USphereComponent* DebugSphere = NewObject<USphereComponent>(GetOwner());
		// 		
		// 		DebugSphere->SetSphereRadius(Pontoon.Radius);
		// 		DebugSphere->SetRelativeLocation(Pontoon.CenterLocation);
		// 		DebugSphere->ShapeColor = FColor::Emerald;
		//
		// 		DebugSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		//
		// 		GetOwner()->AddInstanceComponent(DebugSphere);
		// 		DebugSphere->OnComponentCreated();
		// 		DebugSphere->RegisterComponent();
		// 		
		// 		DebugSphere->SetHiddenInGame(false);
		// 		DebugSphere->bIsEditorOnly = true;
		// 		DebugSphere->MarkAsEditorOnlySubobject();
		// 		
		// 		DebugSphereComponents.Add(DebugSphere);
		//
		// 		UE_LOG(LogTemp, Display, TEXT("USwimBuoyancyComponent DebugSphere Created"));//
		// 	}
		// }
	}
}

void USwimBuoyancyComponent::NotifyDebugPontoonEnteredWater(const FSphericalPontoon& Pontoon)
{
	if(DebugSphereComponents.Num() != BuoyancyData.Pontoons.Num())
	{
		UE_LOG(LogCharon, Error, TEXT("USwimBuoyancyComponent DebugSpheres Num is different with pontoons Num."));
		return;
	}
	
	int PontoonIdx = -1;
	for(int i = 0; i < BuoyancyData.Pontoons.Num(); i++)
	{
		if(Pontoon.Radius == BuoyancyData.Pontoons[i].Radius && Pontoon.CenterSocket == BuoyancyData.Pontoons[i].CenterSocket
			&& Pontoon.RelativeLocation == BuoyancyData.Pontoons[i].RelativeLocation)
		{
			PontoonIdx = i;
			break;
		}
	}

	if(PontoonIdx >= 0)
	{
		DebugSphereComponents[PontoonIdx]->SetLineThickness(4.0f);
	}
}

void USwimBuoyancyComponent::NotifyDebugPontoonExitedWater(const FSphericalPontoon& Pontoon)
{
	if(DebugSphereComponents.Num() != BuoyancyData.Pontoons.Num())
	{
		UE_LOG(LogCharon, Error, TEXT("USwimBuoyancyComponent DebugSpheres Num is different with pontoons Num."));
		return;
	}
	
	int PontoonIdx = -1;
	for(int i = 0; i < BuoyancyData.Pontoons.Num(); i++)
	{
		if(Pontoon.Radius == BuoyancyData.Pontoons[i].Radius && Pontoon.CenterSocket == BuoyancyData.Pontoons[i].CenterSocket
			&& Pontoon.RelativeLocation == BuoyancyData.Pontoons[i].RelativeLocation)
		{
			PontoonIdx = i;
			break;
		}
	}

	if(PontoonIdx >= 0)
	{
		DebugSphereComponents[PontoonIdx]->SetLineThickness(1.0f);
	}
}

bool USwimBuoyancyComponent::GetIsSwimming() const
{
	if(SwimCheckPontoonIndex >= 0 && SwimCheckPontoonIndex < BuoyancyData.Pontoons.Num())
	{
		return BuoyancyData.Pontoons[SwimCheckPontoonIndex].bIsInWater;
	}
	return false;
}


#endif

float USwimBuoyancyComponent::GetImmersionDepth()
{
	if (WaterCheckPontoonIndex >= 0)
	{
		if(WaterCheckPontoonIndex >= BuoyancyData.Pontoons.Num())
		{
			UE_LOG(LogCharon, Error, TEXT("USwimBuoyancyComponent SwimBasePontoonIndex is bigger than pontoons size."));
			return 0.0f;
		}

		FSphericalPontoon& SwimBasePontoon = BuoyancyData.Pontoons[WaterCheckPontoonIndex];
		//float SwimZ = SwimBasePontoon.CenterLocation.Z;

		//일단 임시.
		//폰툰의 immersionDepth 가 어떤식으로 작동하는지 자세하게는 모르지만 그냥 잠겨있는 깊이라고 가정하면,
		//폰툰의 radius(반경) 기준으로.. 반경 맞지? 잠겨있는 깊이가 폰툰의 직경을 넘으면 1, 반경 넘으면 0.5, 물에 안닿이있으면 0.
		// 를 취소하고 수정중
		return FMath::Min(3.0f, ((SwimBasePontoon.ImmersionDepth) / (FMath::Max(1.f, SwimBasePontoon.Radius))));
	}

	return 0.0f;
}

// FVector USwimBuoyancyComponent::GetSwimBuoyancy()
// {
// 	if(WaterCheckPontoonIndex >= 0 && WaterCheckPontoonIndex < BuoyancyData.Pontoons.Num())
// 	{
// 		return BuoyancyData.Pontoons[WaterCheckPontoonIndex].LocalForce;	
// 	}
// 	return FVector::ZeroVector;
// }

// FString USwimBuoyancyComponent::GetDebugString()
// {
// 	return FString::Printf(TEXT("USwimBuoyancyComponent Test WaterCheckPontoon Immersion Depth : %f, WaterHeight : %f, CenterLocation : %s, PontoonRadius : %f ,SwimImmersionDepth : %f"),
// 					WaterCheckPontoon.ImmersionDepth, WaterCheckPontoon.WaterHeight, *WaterCheckPontoon.CenterLocation.ToString(), WaterCheckPontoon.Radius, GetImmersionDepth());
// }

void USwimBuoyancyComponent::CheckSwimPontoonEnteredWater(const FSphericalPontoon& Pontoon)
{
	// 따로 Pontoon에서 ==를 오버라이드 해두지 않아서 그냥 이렇게 체크
	if(SwimCheckPontoon.Radius == Pontoon.Radius
		&& SwimCheckPontoon.RelativeLocation == Pontoon.RelativeLocation && SwimCheckPontoon.CenterSocket == Pontoon.CenterSocket)
	{
		OnSwimPontoonEnteredWaterDelegate.Broadcast(Pontoon);
	}
}

void USwimBuoyancyComponent::CheckSwimPontoonExitedWater(const FSphericalPontoon& Pontoon)
{
	// 따로 Pontoon에서 ==를 오버라이드 해두지 않아서 그냥 이렇게 체크
	if( SwimCheckPontoon.Radius == Pontoon.Radius
		&& SwimCheckPontoon.RelativeLocation == Pontoon.RelativeLocation && SwimCheckPontoon.CenterSocket == Pontoon.CenterSocket)
	{
		OnSwimPontoonExitedWaterDelegate.Broadcast(Pontoon);
	}
}
