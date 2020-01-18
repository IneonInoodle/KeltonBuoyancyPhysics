// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "UnderWaterMeshGenerator.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FTriangleData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriangleData")
	FVector p1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriangleData")
	FVector p2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriangleData")
	FVector p3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriangleData")
	FVector center;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriangleData")
	float distanceToSurface;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriangleData")
	FVector normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriangleData")
	float area;

	//normally this would be the construction but unreal doesnt allow it so have to make it a method
	FTriangleData(FVector Newp1, FVector Newp2, FVector Newp3) {
		p1 = Newp1;
		p2 = Newp2;
		p3 = Newp3;

		//Center of the triangle
		center = (p1 + p2 + p3) / 3.0f;
		
		//float MyTime = GetWorld()->GetTimeSeconds(); 
		//Distance to the surface from the center of the triangle
		//distanceToSurface = FMath.Abs(WaterController.current.DistanceToWater(this.center, Time.time));

		//Normal to the triangle
		normal = FVector::CrossProduct(p2 - p1, p3 - p1);
		normal.Normalize();

		//Area of the triangle
		float a = FVector::Distance(p1, p2);

		float c = FVector::Distance(p3, p1);
		
		// formula to get angle between two vectors found here https://www.jofre.de/?page_id=1297#item6
		float angle = FMath::Atan2(FVector::CrossProduct(p2 - p1, p3 - p1).Normalize(), FVector::DotProduct(p2 - p1, p3 - p1));		
		area = (a * c * FMath::Sin(FMath::RadiansToDegrees(angle))) / 2.0f;
	}

	FTriangleData() {}
};

UCLASS()
class BUOYANCYPHYSICS_API UUnderWaterMeshGenerator : public UObject
{
	GENERATED_BODY()

public:
	void GenerateUnderWaterMesh();
	void DisplayMesh();
	
};
