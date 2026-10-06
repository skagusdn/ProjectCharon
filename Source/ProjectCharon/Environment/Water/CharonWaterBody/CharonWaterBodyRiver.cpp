// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonWaterBodyRiver.h"

#include "BakedShallowWaterSimulationComponent.h"
#include "Logging.h"
#include "WaterBodyComponent.h"
#include "AI/NavArea_Water.h"
#include "Components/BoxComponent.h"
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
		UE_LOGF(LogCharon, Warning, "RebuildNavCollisionFromBakedSim : 베이크된 시뮬레이션 데이터가 없습니다. ShallowWaterRiverActor에서 Bake를 먼저 실행하세요.");
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

					// NaN/무한대나 터무니없는 값이 섞이면 내비 옥트리에서 스택 오버플로로 이어진다.
					if (Depth > 1e-5f && FMath::IsFinite(Height) && FMath::Abs(Height) < 1e7f)
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

		// ///테스트 중~
		// UStaticMeshComponent* NewTestComp = NewObject<UStaticMeshComponent>(this);
		// UBodySetup* NewBodySetup = NewObject<UBodySetup>(NewTestComp);
		// NewBodySetup->AggGeom = BodySetup->AggGeom;
		
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

			// 윗면 정점(4~)만 각자 위치의 수면 높이로 맞춘다. 바닥(0~3)은 그대로 둔다.
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
				// 원래 바닥(강바닥 근처)을 그대로 두면 컨벡스 안에 들어온 강바닥까지 Water 영역이 되고,
				// Recast는 컨벡스 속을 채우지 않으므로 수면 아래 강바닥에도 내비메시가 생긴다.
				WorldVerts[Idx - 4].Z = WorldVerts[Idx].Z - NavSlabThickness;
			}

			NavConvexes.Add(MoveTemp(WorldVerts));
		}
	}
	
	
	
	// // ───────── 디버그 : 계산된 NavConvexes 시각화 ─────────
	// // 내비 컴포넌트에 넘기기 전의 월드 좌표 컨벡스를 그대로 그린다.
	// {
	// 	UWorld* DebugWorld = GetWorld();
	//
	// 	// 버튼을 여러 번 눌러도 겹치지 않게 이전 라인을 먼저 지운다.
	// 	// (콘솔에서 FlushPersistentDebugLines 로 직접 지울 수도 있다)
	// 	FlushPersistentDebugLines(DebugWorld);
	//
	// 	constexpr float VertexRadius = 20.f;
	// 	constexpr float LineThickness = 3.f;
	// 	constexpr float MarkerHeight = 300.f;
	//
	// 	for (int32 ConvexIndex = 0; ConvexIndex < NavConvexes.Num(); ++ConvexIndex)
	// 	{
	// 		const TArray<FVector>& WorldVerts = NavConvexes[ConvexIndex];
	// 		if (WorldVerts.Num() < 8)
	// 		{
	// 			UE_LOGF(LogCharon, Warning, "NavConvex[%d] : 정점 %d개로 시각화를 건너뜀.", ConvexIndex, WorldVerts.Num());
	// 			continue;
	// 		}
	//
	// 		// 컨벡스마다 다른 색으로 그려서 슬래브 경계를 구분한다.
	// 		const FColor HullColor = FLinearColor::MakeFromHSV8(static_cast<uint8>((ConvexIndex * 37) % 256), 255, 255).ToFColor(true);
	//
	// 		FVector TopCenter = FVector::ZeroVector;
	// 		double TopMinZ = UE_BIG_NUMBER;
	// 		double TopMaxZ = -UE_BIG_NUMBER;
	// 		double BottomMinZ = UE_BIG_NUMBER;
	// 		double BottomMaxZ = -UE_BIG_NUMBER;
	//
	// 		for (int32 i = 0; i < 4; ++i)
	// 		{
	// 			const FVector BottomVert = WorldVerts[i];
	// 			const FVector TopVert = WorldVerts[i + 4];
	//
	// 			// 바닥 정점은 파랑, 수면 높이로 조정된 윗면 정점은 시안.
	// 			DrawDebugSphere(DebugWorld, BottomVert, VertexRadius, 8, FColor::Blue, /*bPersistentLines=*/true, -1.f, 0, 1.f);
	// 			DrawDebugSphere(DebugWorld, TopVert, VertexRadius, 8, FColor::Cyan, /*bPersistentLines=*/true, -1.f, 0, 1.f);
	//
	// 			// 수직 엣지 (바닥 i ↔ 윗면 i+4)
	// 			DrawDebugLine(DebugWorld, BottomVert, TopVert, HullColor, /*bPersistentLines=*/true, -1.f, 0, LineThickness);
	//
	// 			// 정점 순서를 알 수 없으므로 각 면의 모든 쌍을 잇는다.
	// 			// 면 대각선까지 그려져 X자가 보이지만 형상 확인에는 충분하다.
	// 			for (int32 j = i + 1; j < 4; ++j)
	// 			{
	// 				DrawDebugLine(DebugWorld, BottomVert, WorldVerts[j], HullColor, /*bPersistentLines=*/true, -1.f, 0, LineThickness);
	// 				DrawDebugLine(DebugWorld, TopVert, WorldVerts[j + 4], HullColor, /*bPersistentLines=*/true, -1.f, 0, LineThickness);
	// 			}
	//
	// 			TopCenter += TopVert;
	// 			TopMinZ = FMath::Min(TopMinZ, TopVert.Z);
	// 			TopMaxZ = FMath::Max(TopMaxZ, TopVert.Z);
	// 			BottomMinZ = FMath::Min(BottomMinZ, BottomVert.Z);
	// 			BottomMaxZ = FMath::Max(BottomMaxZ, BottomVert.Z);
	// 		}
	//
	// 		TopCenter /= 4.0;
	//
	// 		// 윗면 중심에서 위로 뻗는 선. 컨벡스 개수를 세고 순서를 파악하기 쉽게.
	// 		DrawDebugLine(DebugWorld, TopCenter, TopCenter + FVector(0., 0., MarkerHeight), HullColor,
	// 			/*bPersistentLines=*/true, -1.f, 0, LineThickness);
	//
	// 		// 같은 위치의 수면 높이도 함께 찍어서 윗면이 수면에 맞았는지 비교한다.
	// 		float WaterHeightAtCenter = 0.f;
	// 		const bool bHasWaterAtCenter = FindWaterHeightNear(FVector(TopCenter.X, TopCenter.Y, BottomMaxZ), WaterHeightAtCenter);
	//
	// 		if (bHasWaterAtCenter)
	// 		{
	// 			// 수면 높이에 초록색 수평 십자 표시.
	// 			const FVector WaterCenter(TopCenter.X, TopCenter.Y, WaterHeightAtCenter);
	// 			DrawDebugLine(DebugWorld, WaterCenter - FVector(150., 0., 0.), WaterCenter + FVector(150., 0., 0.), FColor::Green,
	// 				/*bPersistentLines=*/true, -1.f, 0, LineThickness);
	// 			DrawDebugLine(DebugWorld, WaterCenter - FVector(0., 150., 0.), WaterCenter + FVector(0., 150., 0.), FColor::Green,
	// 				/*bPersistentLines=*/true, -1.f, 0, LineThickness);
	// 		}
	//
	// 		UE_LOGF(LogCharon, Log,
	// 			"NavConvex[%d] : TopZ=%.1f~%.1f (Spread=%.2f), BottomZ=%.1f~%.1f, Thickness=%.1f, WaterZ=%ls, TopCenter=%ls",
	// 			ConvexIndex,
	// 			TopMinZ, TopMaxZ, TopMaxZ - TopMinZ,
	// 			BottomMinZ, BottomMaxZ,
	// 			TopMinZ - BottomMaxZ,
	// 			bHasWaterAtCenter ? *FString::Printf(TEXT("%.1f"), WaterHeightAtCenter) : TEXT("N/A"),
	// 			*TopCenter.ToString());
	// 	}
	//
	// 	UE_LOGF(LogCharon, Log, "NavConvexes 시각화 완료 : %d개. 지우려면 콘솔에 FlushPersistentDebugLines 입력.", NavConvexes.Num());
	// }
	
	
	
	
	if (NavCollisionComponent == nullptr)
	{
		Modify();
		
		//TestComp = NewObject<UCharonWaterNavCollisionComponent>(this, NAME_None, RF_Transactional);

		
		NavCollisionComponent = NewObject<UCharonWaterNavCollisionComponent>(this, TEXT("WaterNavCollision"), RF_Transactional);
		NavCollisionComponent->SetupAttachment(GetRootComponent());
		NavCollisionComponent->RegisterComponent();
		AddInstanceComponent(NavCollisionComponent);
	}
	
	if (!NavAreaClass)
	{
		UE_LOGF(LogCharon, Warning, "RebuildNavCollisionFromBakedSim : NavAreaClass가 비어 있어 내비 영역이 기본값으로 생성됩니다.");
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
	
	UE_LOGF(LogCharon, Log, "RebuildNavCollisionFromBakedSim : 내비 컨벡스 %d개 생성, 윗면 정점 %d개 조정 완료. Build Paths를 실행하세요.",
		NavConvexes.Num(), AdjustedVertCount);
#endif
}


void ACharonWaterBodyRiver::BeginPlay()
{
	Super::BeginPlay();

}



