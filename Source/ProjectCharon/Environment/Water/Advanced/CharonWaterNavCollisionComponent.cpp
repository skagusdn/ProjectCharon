// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonWaterNavCollisionComponent.h"

#include "AI/NavigationSystemBase.h"
#include "AI/NavigationSystemHelpers.h"
#include "Chaos/Convex.h"
#include "Engine/CollisionProfile.h"
#include "NavigationSystemTypes.h"
//#include "NavigationRelevantData.h"
#include "AI/Navigation/NavAreaBase.h"
#include "AI/Navigation/NavigationRelevantData.h"
#include "HAL/IConsoleManager.h"
#include "Logging.h"
#include "PhysicsEngine/BodySetup.h"

// 크래시 원인 격리용 스위치. 둘 다 0으로 두고 버튼을 눌렀을 때도 크래시가 나면
// 지오메트리/영역 제공이 아니라 내비 옥트리 등록(=바운드) 쪽이 원인이다.
static TAutoConsoleVariable<int32> CVarWaterNavExportGeometry(
	TEXT("Charon.WaterNav.ExportGeometry"),
	1,
	TEXT("1이면 내비메시에 형상을 제공한다. 0이면 제공하지 않는다."));

static TAutoConsoleVariable<int32> CVarWaterNavExportAreaModifier(
	TEXT("Charon.WaterNav.ExportAreaModifier"),
	1,
	TEXT("1이면 내비메시에 NavArea 꼬리표를 제공한다. 0이면 제공하지 않는다."));

namespace CharonWaterNavCollision
{
	static bool IsFiniteVector(const FVector& Vector)
	{
		return FMath::IsFinite(Vector.X) && FMath::IsFinite(Vector.Y) && FMath::IsFinite(Vector.Z);
	}

	// 볼록한 점 집합의 겉면 삼각형을 찾는다.
	// 점 3개로 평면을 만들고, 나머지 점이 전부 한쪽에 있으면 겉면이다.
	// 같은 평면 위의 사각형 면은 삼각형이 겹쳐서 나올 수 있지만, 내비메시 생성에는 문제없다.
	static void AppendConvexHullTriangles(const TArray<FVector>& Points,
		TArray<FVector>& OutVertices, TArray<int32>& OutIndices)
	{
		const int32 BaseIndex = OutVertices.Num();
		OutVertices.Append(Points);

		const int32 NumPoints = Points.Num();
		constexpr double PlaneTolerance = 0.1; // cm

		for (int32 i = 0; i < NumPoints; ++i)
		{
			for (int32 j = i + 1; j < NumPoints; ++j)
			{
				for (int32 k = j + 1; k < NumPoints; ++k)
				{
					const FVector Normal = FVector::CrossProduct(Points[j] - Points[i], Points[k] - Points[i]);
					const double Length = Normal.Size();
					if (Length < UE_KINDA_SMALL_NUMBER)
					{
						continue; // 세 점이 일직선
					}
					const FVector UnitNormal = Normal / Length;

					bool bHasFront = false;
					bool bHasBack = false;
					for (int32 m = 0; m < NumPoints && !(bHasFront && bHasBack); ++m)
					{
						if (m == i || m == j || m == k)
						{
							continue;
						}

						const double Distance = FVector::DotProduct(UnitNormal, Points[m] - Points[i]);
						if (Distance > PlaneTolerance)
						{
							bHasFront = true;
						}
						else if (Distance < -PlaneTolerance)
						{
							bHasBack = true;
						}
					}

					if (bHasFront && bHasBack)
					{
						continue; // 덩어리 내부를 가로지르는 평면
					}

					// 나머지 점이 전부 뒤쪽에 있으면 Normal(= CrossProduct(B-A, C-A))이 바깥을 향한다.
					// 그런데 Recast는 언리얼 좌표를 자기 좌표로 바꿀 때 축이 뒤바뀌어 앞뒤 판정이 반대가 된다.
					// 따라서 Normal이 바깥을 향하는 경우 순서를 뒤집어야 Recast에서 윗면이 '바닥'으로 인식된다.
					// (테스트로 확인 : 이 반전이 없으면 윗면이 천장으로 처리되어 내비메시가 생기지 않았다)
					const bool bKeepOrder = bHasFront;

					OutIndices.Add(BaseIndex + i);
					OutIndices.Add(BaseIndex + (bKeepOrder ? j : k));
					OutIndices.Add(BaseIndex + (bKeepOrder ? k : j));
				}
			}
		}
	}
}

UCharonWaterNavCollisionComponent::UCharonWaterNavCollisionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// 물리와 게임플레이 쿼리에는 전혀 관여하지 않는다.
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCastShadow(false);
	bHiddenInGame = true;
	
	// 콜리전이 꺼져 있어도 내비게이션에는 형상을 내보내도록.
	bCanEverAffectNavigation = true;
	SetCustomNavigableGeometry(EHasCustomNavigableGeometry::EvenIfNotCollidable);
}

void UCharonWaterNavCollisionComponent::SetNavConvexes(const TArray<TArray<FVector>>& WorldSpaceConvexes)
{
	if (NavBodySetup == nullptr)
	{
		NavBodySetup = NewObject<UBodySetup>(this, TEXT("NavBodySetup"));
		NavBodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
	}

	NavBodySetup->AggGeom.EmptyElements();

	NavMeshVertices.Reset();
	NavMeshIndices.Reset();

	const FTransform WorldToLocal = GetComponentTransform().Inverse();

	int32 SkippedConvexCount = 0;

	for (const TArray<FVector>& WorldVerts : WorldSpaceConvexes)
	{
		if (WorldVerts.Num() < 4)
		{
			continue;
		}

		FKConvexElem Convex;
		Convex.VertexData.Reserve(WorldVerts.Num());

		bool bHasInvalidVertex = false;
		for (const FVector& WorldVert : WorldVerts)
		{
			const FVector LocalVert = WorldToLocal.TransformPosition(WorldVert);

			// NaN/무한대가 섞이면 내비 옥트리 삽입이 무한 재귀에 빠져 스택 오버플로로 죽는다.
			if (!CharonWaterNavCollision::IsFiniteVector(LocalVert))
			{
				bHasInvalidVertex = true;
				break;
			}

			Convex.VertexData.Add(LocalVert);
		}

		if (bHasInvalidVertex)
		{
			++SkippedConvexCount;
			continue;
		}

		Convex.UpdateElemBox();

		// 내비메시 모양용 삼각형. 로컬 공간 정점 그대로 사용한다.
		CharonWaterNavCollision::AppendConvexHullTriangles(Convex.VertexData, NavMeshVertices, NavMeshIndices);

		// 내비 지오메트리 export 경로가 Chaos 컨벡스를 요구할 수 있어 미리 만들어 둔다.
		// (WaterBodyRiverComponent::UpdateSplineMesh가 쓰는 방식과 동일)
		TArray<Chaos::FConvex::FVec3Type> ChaosVerts;
		ChaosVerts.Reserve(Convex.VertexData.Num());
		for (const FVector& Vert : Convex.VertexData)
		{
			ChaosVerts.Add(Vert);
		}
		Convex.SetConvexMeshObject(Chaos::FConvexPtr(new Chaos::FConvex(ChaosVerts, 0.0f)));

		NavBodySetup->AggGeom.ConvexElems.Add(MoveTemp(Convex));
	}

	UpdateBounds();

	// 바운드가 이상하면(무한대/NaN) 내비 옥트리 삽입에서 스택 오버플로가 난다. 로그로 확인할 것.
	const FBoxSphereBounds CurrentBounds = Bounds;
	UE_LOGF(LogCharon, Log,
		"UCharonWaterNavCollisionComponent::SetNavConvexes : Convex=%d(건너뜀 %d), Triangles=%d, Origin=%ls, Extent=%ls, Radius=%f",
		GetNavConvexCount(),
		SkippedConvexCount,
		NavMeshIndices.Num() / 3,
		*CurrentBounds.Origin.ToString(),
		*CurrentBounds.BoxExtent.ToString(),
		CurrentBounds.SphereRadius);

	FNavigationSystem::UpdateComponentData(*this);
}

int32 UCharonWaterNavCollisionComponent::GetNavConvexCount() const
{
	return NavBodySetup ? NavBodySetup->AggGeom.ConvexElems.Num() : 0;
}

FBoxSphereBounds UCharonWaterNavCollisionComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (NavBodySetup == nullptr || NavBodySetup->AggGeom.ConvexElems.Num() == 0)
	{
		return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector::ZeroVector, 0.f);
	}

	FBox LocalBox(ForceInit);
	for (const FKConvexElem& Convex : NavBodySetup->AggGeom.ConvexElems)
	{
		LocalBox += Convex.ElemBox;
	}

	const FBoxSphereBounds Result(LocalBox.TransformBy(LocalToWorld));

	// 잘못된 바운드를 그대로 넘기면 내비/씬 옥트리가 무한 분할하며 스택 오버플로를 일으킨다.
	if (!LocalBox.IsValid
		|| !CharonWaterNavCollision::IsFiniteVector(Result.Origin)
		|| !CharonWaterNavCollision::IsFiniteVector(Result.BoxExtent)
		|| !FMath::IsFinite(Result.SphereRadius))
	{
		UE_LOGF(LogCharon, Error, "UCharonWaterNavCollisionComponent::CalcBounds : 바운드가 유효하지 않아 빈 바운드를 반환한다.");
		return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector::ZeroVector, 0.f);
	}

	return Result;
}

bool UCharonWaterNavCollisionComponent::DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const
{
	if (NavMeshIndices.Num() == 0 || CVarWaterNavExportGeometry.GetValueOnAnyThread() == 0)
	{
		return false;
	}

	// 엔진의 컨벡스 → 삼각형 변환을 거치지 않고, 직접 계산한 삼각형을 그대로 넘긴다.
	GeomExport.ExportCustomMesh(NavMeshVertices.GetData(), NavMeshVertices.Num(),
		NavMeshIndices.GetData(), NavMeshIndices.Num(), GetComponentTransform());

	// false : 기본 지오메트리는 추가로 내보내지 않는다.
	return false;
}

void UCharonWaterNavCollisionComponent::GetNavigationData(FNavigationRelevantData& Data) const
{
	if (!NavAreaClass || NavBodySetup == nullptr || CVarWaterNavExportAreaModifier.GetValueOnAnyThread() == 0)
	{
		return;
	}

	// UWaterBodyComponent::GetNavigationData와 같은 방식으로 영역 모디파이어를 만든다.
	FCompositeNavModifier CompositeNavModifier;
	CompositeNavModifier.CreateAreaModifiers(this, NavAreaClass);
	for (FAreaNavModifier& AreaNavModifier : CompositeNavModifier.GetMutableAreas())
	{
		AreaNavModifier.SetExpandTopByCellHeight(true);
	}

	Data.Modifiers.Add(CompositeNavModifier);
}

bool UCharonWaterNavCollisionComponent::IsNavigationRelevant() const
{
	return CanEverAffectNavigation() && IsRegistered() && GetNavConvexCount() > 0;
}
