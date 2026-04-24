// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonBuoyancyComponent.h"

UCharonBuoyancyComponent::UCharonBuoyancyComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bUpdatePontoonsOutOfWater = false;
}

int32 UCharonBuoyancyComponent::UpdatePontoons(float DeltaTime, float ForwardSpeed, float ForwardSpeedKmh,
	UPrimitiveComponent* PrimitiveComponent)
{
	AActor* Owner = GetOwner();
	check(Owner);

	int32 NumPontoonsInWater = 0;
	if (bIsOverlappingWaterBody || bUpdatePontoonsOutOfWater)
	{
		int PontoonIndex = 0;
		for (FSphericalPontoon& Pontoon : BuoyancyData.Pontoons)
		{
			if (PontoonConfiguration & (1 << PontoonIndex))
			{
				// 폰툰의 위치 : 폰툰 센터 소켓 설정을 쓸 경우 소켓 + 오프셋, 아니면 루트 컴포넌트 + 폰툰 상대 위치
				if (Pontoon.bUseCenterSocket)
				{
					const FTransform& SimulatingComponentTransform = PrimitiveComponent->GetSocketTransform(Pontoon.CenterSocket);
					Pontoon.CenterLocation = SimulatingComponentTransform.GetLocation() + Pontoon.Offset;
					Pontoon.SocketRotation = SimulatingComponentTransform.GetRotation();
				}
				else
				{
					Pontoon.CenterLocation = PrimitiveComponent->GetComponentTransform().TransformPosition(Pontoon.RelativeLocation);
				}
				// 폰툰의 물 높이 구함
				GetWaterSplineKey(Pontoon.CenterLocation, Pontoon.SplineInputKeys, Pontoon.SplineSegments);
				const FVector PontoonBottom = Pontoon.CenterLocation - FVector(0, 0, Pontoon.Radius);
				UWaterBodyComponent* TempWaterBodyComponent = Pontoon.CurrentWaterBodyComponent;
				/*Pass in large negative default value so we don't accidentally assume we're in water when we're not.*/
				Pontoon.WaterHeight = GetWaterHeight(PontoonBottom - FVector::UpVector * 100.f, Pontoon.SplineInputKeys, -100000.f, TempWaterBodyComponent, Pontoon.WaterDepth, Pontoon.WaterPlaneLocation, Pontoon.WaterPlaneNormal, Pontoon.WaterSurfacePosition, Pontoon.WaterVelocity, Pontoon.WaterBodyIndex);
				Pontoon.CurrentWaterBodyComponent = TempWaterBodyComponent;

				const bool bPrevIsInWater = Pontoon.bIsInWater;
				const float ImmersionDepth = Pontoon.WaterHeight - PontoonBottom.Z;
				/*check if the pontoon is currently in water*/
				if (ImmersionDepth >= 0.f)
				{
					Pontoon.bIsInWater = true;
					Pontoon.ImmersionDepth = ImmersionDepth;
					NumPontoonsInWater++;
				}
				else
				{
					Pontoon.bIsInWater = false;
					Pontoon.ImmersionDepth = 0.f;
				}

#if ENABLE_DRAW_DEBUG
				if (CVarWaterDebugBuoyancy.GetValueOnAnyThread())
				{
					DrawDebugSphere(GetWorld(), Pontoon.CenterLocation, Pontoon.Radius, 16, FColor::Red, false, -1.f, 0, 1.f);
				}
#endif
				ComputeBuoyancy(Pontoon, ForwardSpeedKmh);

				if (Pontoon.bIsInWater && !bPrevIsInWater)
				{
					Pontoon.SplineSegments.Reset();
					// BlueprintImplementables don't really work on the actor component level unfortunately, so call back in to the function defined on the actor itself.
					OnPontoonEnteredWater(Pontoon);
				}
				if (!Pontoon.bIsInWater && bPrevIsInWater)
				{
					Pontoon.SplineSegments.Reset();
					OnPontoonExitedWater(Pontoon);
				}
			}
			PontoonIndex++;
		}

#if ENABLE_DRAW_DEBUG
		if (CVarWaterDebugBuoyancy.GetValueOnAnyThread())
		{
			const float NumPoints = CVarWaterBuoyancyDebugPoints.GetValueOnAnyThread();
			const float Size = CVarWaterBuoyancyDebugSize.GetValueOnAnyThread();
			const float StartOffset = NumPoints * 0.5f;
			const float Scale = Size / NumPoints;
			TMap<const UWaterBodyComponent*, float> DebugSplineKeyMap;
			TMap<const UWaterBodyComponent*, float> DebugSplineSegmentsMap;
			for (int i = 0; i < NumPoints; ++i)
			{
				for (int j = 0; j < NumPoints; ++j)
				{
					FVector Location = PrimitiveComponent->GetComponentLocation() + (FVector::RightVector * (i - StartOffset) * Scale) + (FVector::ForwardVector * (j - StartOffset) * Scale);
					GetWaterSplineKey(Location, DebugSplineKeyMap, DebugSplineSegmentsMap);
					FVector Point(Location.X, Location.Y, GetWaterHeight(Location - FVector::UpVector * 200.f, DebugSplineKeyMap, GetOwner()->GetActorLocation().Z));
					DrawDebugPoint(GetWorld(), Point, 5.f, IsOverlappingWaterBody() ? FColor::Green : FColor::Red, false, -1.f, 0);
				}
			}
		}
#endif
	}
	return NumPontoonsInWater;
}

void UCharonBuoyancyComponent::AddPontoons(float Radius, TArray<FVector> Locations)
{
	for (FVector location : Locations) {
		AddCustomPontoon(Radius, location);
	}
}

// 두 벡터를 양 끝으로 하는 직육면체 공간에 밸런스 있게 구를 넣을 수 있는 로케이션 반환. 폰툰 만들기 용.
TArray<FVector> UCharonBuoyancyComponent::CalculateBallsLocation(FVector UpLeftFront, FVector DownRightBack, int32 Radius, int MaxPontoonNum)
{
	TArray<FVector> ret;

	float x0 = FMath::Min(UpLeftFront.X, DownRightBack.X);
	float y0 = FMath::Min(UpLeftFront.Y, DownRightBack.Y);
	float z0 = FMath::Min(UpLeftFront.Z, DownRightBack.Z);

	float xEnd = FMath::Max(UpLeftFront.X, DownRightBack.X);
	float yEnd = FMath::Max(UpLeftFront.Y, DownRightBack.Y);
	float zEnd = FMath::Max(UpLeftFront.Z, DownRightBack.Z);

	float xl = (xEnd - x0) / 2;
	float yl = (yEnd - y0) / 2;
	float zl = (zEnd - z0) / 2;

	
	TArray<FVector> Layer0;

	if(MaxPontoonNum == 0)
	{
		MaxPontoonNum = 1000;
	}
	MaxPontoonNum = FMath::Min(1000, MaxPontoonNum);
	
	MaxPontoonNum -= MaxPontoonNum % 4;
	int PontoonNumPerLayer = 0;
	int totalPontoonNum = 0;
	float zMiddle = (UpLeftFront.Z + DownRightBack.Z) / 2;
	
	
	for (float y = Radius; y + Radius <= yl; y += 2 * Radius) {
		for (float x = Radius; x + Radius <= xl; x += 2 * Radius) {
			if(PontoonNumPerLayer + 4 > MaxPontoonNum)
			{
				break;
			}
			PontoonNumPerLayer += 4;
			
			Layer0.Add(FVector(x0 + x, y0 + y, zMiddle));
			Layer0.Add(FVector(x0 + x, yEnd - y, zMiddle));
			Layer0.Add(FVector(xEnd - x, y0 + y, zMiddle));
			Layer0.Add(FVector(xEnd - x, yEnd - y, zMiddle));
		}
	}

	totalPontoonNum += PontoonNumPerLayer;

	for (float zStep = Radius * 2; zStep <= zl; zStep += 2 * Radius) {
		for (const FVector& v : Layer0) {
			if(totalPontoonNum + PontoonNumPerLayer * 2 > MaxPontoonNum)
			{
				break;
			}
			totalPontoonNum += PontoonNumPerLayer * 2;
			
			ret.Add(FVector(v.X, v.Y, zMiddle + zStep));
			ret.Add(FVector(v.X, v.Y, zMiddle - zStep));
		}
	}

	for (const FVector& v : Layer0) {
		ret.Add(v);
	}

	return ret;
}

void UCharonBuoyancyComponent::InitPontoons(FVector UpLeftFront, FVector DownRightBack, int32 Radius, int MaxPontoonNum) 
{
	TArray<FVector> Locations = CalculateBallsLocation(UpLeftFront, DownRightBack, Radius, MaxPontoonNum);

	AddPontoons(Radius, Locations);
}

double UCharonBuoyancyComponent::GetPercentOfPontoonsInWater() const
{
	double PontoonNum = BuoyancyData.Pontoons.Num();
	if(PontoonNum <= 0)
	{
		return 0;
	}

	double NumInWater = 0;
	for(FSphericalPontoon Pontoon : BuoyancyData.Pontoons)
	{
		NumInWater += Pontoon.bIsInWater ? 1 : 0;
	}
	
	return NumInWater / PontoonNum;
}

