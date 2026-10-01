// Fill out your copyright notice in the Description page of Project Settings.


#include "NavigationQueryFilter_Fish.h"

#include "NavArea_Water.h"
#include "NavAreas/NavArea_Default.h"

UNavigationQueryFilter_Fish::UNavigationQueryFilter_Fish()
{
	AddTravelCostOverride(UNavArea_Water::StaticClass(), 1.f);   // 물은 정상 비용
	AddExcludedArea(UNavArea_Default::StaticClass());            // 땅은 아예 제외
}
