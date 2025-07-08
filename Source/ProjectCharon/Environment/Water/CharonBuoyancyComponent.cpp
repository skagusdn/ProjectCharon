// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonBuoyancyComponent.h"

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

