// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonWaterBodyRiver.h"

#include "BakedShallowWaterSimulationComponent.h"
#include "Logging.h"
#include "WaterBodyComponent.h"
#include "AI/NavArea_Water.h"
#include "Environment/Water/Advanced/CharonWaterNavCollisionComponent.h"
#include "PhysicsEngine/BodySetup.h"


ACharonWaterBodyRiver::ACharonWaterBodyRiver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NavAreaClass = UNavArea_Water::StaticClass();
}

AWaterZone* ACharonWaterBodyRiver::GetOwningWaterZone()
{
	return WaterBodyComponent ? WaterBodyComponent->GetWaterZone() : nullptr;
}

void ACharonWaterBodyRiver::RebuildNavCollisionFromBakedSim()
{
#if WITH_EDITOR
	UWaterBodyComponent* LocalWaterBodyComponent = GetWaterBodyComponent();
	if (LocalWaterBodyComponent == nullptr)
	{
		return;
	}

	UBakedShallowWaterSimulationComponent* BakedSim = LocalWaterBodyComponent->GetBakedShallowWaterSimulation();
	if (BakedSim == nullptr || BakedSim->BakedSimulationData == nullptr || !BakedSim->BakedSimulationData->HasValidData())
	{
		UE_LOGF(LogCharon, Warning, "RebuildNavCollisionFromBakedSim : No baked simulation data. Run Bake on ShallowWaterRiverActor first.");
		return;
	}

	const UShallowWaterSimulationDataBase* SimData = BakedSim->BakedSimulationData;

	// 셀 크기는 공개 API만으로 유도한다.
	const FVector CellStep = SimData->IndexToWorld(FIntVector2(1, 0)) - SimData->IndexToWorld(FIntVector2(0, 0));
	const float CellSize = FMath::Max(static_cast<float>(FMath::Abs(CellStep.X)), 1.f);
	const int32 RingMax = FMath::CeilToInt(NavTopSearchRadius / CellSize);

	// 정점의 월드 XY에서 가장 가까운 '물이 있는 셀'의 수면 높이를 찾는다.
	// 위치만으로 결정되므로 이웃 컨벡스가 공유하는 경계 정점은 같은 높이가 나온다. = 계단이 사라짐
	auto FindWaterHeightNear = [&](const FVector& WorldPos, float& OutHeight) -> bool
	{
		for (int32 Ring = 0; Ring <= RingMax; ++Ring)
		{
			bool bFoundInRing = false;
			float BestInRing = -UE_BIG_NUMBER;

			for (int32 dy = -Ring; dy <= Ring; ++dy)
			{
				for (int32 dx = -Ring; dx <= Ring; ++dx)
				{
					// 링 테두리만 검사. 안쪽은 이전 반복에서 이미 봤다.
					if (Ring > 0 && FMath::Abs(dx) != Ring && FMath::Abs(dy) != Ring)
					{
						continue;
					}

					const FVector SamplePos = WorldPos + FVector(dx * CellSize, dy * CellSize, 0.);

					FVector Velocity;
					float Height = 0.f;
					float Depth = 0.f;
					SimData->QueryShallowWaterSimulationAtPosition(SamplePos, Velocity, Height, Depth);

					if (Depth > 1e-5f && FMath::IsFinite(Height))
					{
						bFoundInRing = true;
						BestInRing = FMath::Max(BestInRing, Height);
					}
				}
			}

			if (bFoundInRing)
			{
				OutHeight = BestInRing;
				return true;
			}
		}

		return false;
	};

	TArray<TArray<FVector>> NavConvexes;
	int32 AdjustedVertCount = 0;

	const TArray<UPrimitiveComponent*> CollisionComponents = LocalWaterBodyComponent->GetCollisionComponents(/*bInOnlyEnabledComponents=*/false);

	for (UPrimitiveComponent* CollisionComponent : CollisionComponents)
	{
		if (CollisionComponent == nullptr)
		{
			continue;
		}

		UBodySetup* BodySetup = CollisionComponent->GetBodySetup();
		if (BodySetup == nullptr)
		{
			continue;
		}

		const FTransform ComponentTransform = CollisionComponent->GetComponentTransform();

		for (const FKConvexElem& Convex : BodySetup->AggGeom.ConvexElems)
		{
			if (Convex.VertexData.Num() < 8)
			{
				continue;
			}

			TArray<FVector> WorldVerts;
			WorldVerts.Reserve(Convex.VertexData.Num());
			for (const FVector& Vert : Convex.VertexData)
			{
				WorldVerts.Add(ComponentTransform.TransformPosition(Vert));
			}

			// 윗면 정점(4~)만 각자 위치의 수면 높이로 맞춘다.
			for (int32 Idx = 4; Idx < WorldVerts.Num(); ++Idx)
			{
				float WaterHeight = 0.f;
				if (FindWaterHeightNear(WorldVerts[Idx], WaterHeight))
				{
					WorldVerts[Idx].Z = WaterHeight + NavTopPadding;
					++AdjustedVertCount;
				}
				// 주변에 물이 없으면 원래 높이를 유지한다.

				// 짝이 되는 바닥 정점을 윗면 바로 아래로 올려 얇은 판으로 만든다.
				WorldVerts[Idx - 4].Z = WorldVerts[Idx].Z - NavSlabThickness;
			}

			NavConvexes.Add(MoveTemp(WorldVerts));
		}
	}

	if (NavCollisionComponent == nullptr)
	{
		Modify();
		NavCollisionComponent = NewObject<UCharonWaterNavCollisionComponent>(this, TEXT("WaterNavCollision"), RF_Transactional);
		NavCollisionComponent->SetupAttachment(GetRootComponent());
		NavCollisionComponent->RegisterComponent();
		AddInstanceComponent(NavCollisionComponent);
	}

	NavCollisionComponent->Modify();
	NavCollisionComponent->SetNavAreaClass(NavAreaClass);
	NavCollisionComponent->SetNavConvexes(NavConvexes);

	// 계단 형상이 내비메시에 중복으로 들어가지 않도록 워터바디 쪽 내비게이션을 끈다.
	LocalWaterBodyComponent->Modify();
	LocalWaterBodyComponent->SetCanEverAffectNavigation(false);

	// 워터바디의 WaterNavAreaClass도 같은 값으로 맞춘다.
	// 내비게이션이 꺼져 실제로 쓰이지는 않지만, 값이 어긋나 혼동되는 것을 막는다.
	LocalWaterBodyComponent->SetNavAreaClass(NavAreaClass);
	for (UPrimitiveComponent* CollisionComponent : CollisionComponents)
	{
		if (CollisionComponent)
		{
			CollisionComponent->SetCanEverAffectNavigation(false);
		}
	}

	MarkPackageDirty();

	UE_LOGF(LogCharon, Log, "RebuildNavCollisionFromBakedSim : Created %d nav convexes, adjusted %d top vertices.",
		NavConvexes.Num(), AdjustedVertCount);
#endif
}


void ACharonWaterBodyRiver::BeginPlay()
{
	Super::BeginPlay();
}
