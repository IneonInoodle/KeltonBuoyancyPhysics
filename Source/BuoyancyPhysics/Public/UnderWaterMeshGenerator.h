// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "UnderWaterMeshGenerator.generated.h"

class UStaticMeshComponent;
class UProceduralMeshComponent;

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
		distanceToSurface = FVector::Distance(FVector::ZeroVector, center);

		//Normal to the triangle
		normal = FVector::CrossProduct(p3 - p1, p2 - p1);
		normal.Normalize();

		// formula to get angle between two vectors found here https://www.jofre.de/?page_id=1297#item6
		float angle = FMath::Atan2(FVector::CrossProduct(p2 - p1, p3 - p1).Normalize(), FVector::DotProduct(p2 - p1, p3 - p1));

		//Area of the triangle
		float a = FVector::Distance(p1, p2);
		float c = FVector::Distance(p3, p1);	
		area = (a * c * FMath::Sin(FMath::RadiansToDegrees(angle))) / 2.0f;
	}
	FTriangleData() {}
};

//Helper struct to store triangle data so we can sort the distances
USTRUCT(BlueprintType)
struct FVertexData
{
	GENERATED_BODY()
	//The distance to water from this vertex
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VertexData")
	float distance;
	//An index so we can form clockwise triangles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VertexData")
	int index;
	//The global Vector3 position of the vertex
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VertexData")
	FVector globalVertexPos;
};

UCLASS()
class BUOYANCYPHYSICS_API UUnderWaterMeshGenerator : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> MeshVerticesGlobal;
	UPROPERTY(VisibleAnywhere)
	TArray<FTriangleData> UnderWaterTriangleData;

	void GenerateUnderWaterMesh();
	void DisplayMesh(UProceduralMeshComponent* UnderWaterMesh, TArray<FTriangleData> triangleData);
	void ModifyMesh(UStaticMeshComponent* Comp);
private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ParentMesh;
	UPROPERTY(VisibleAnywhere)
	FTransform MeshTransform;
	UPROPERTY(VisibleAnywhere)
	TArray<int> MeshTriangles;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> MeshVertices;
	UPROPERTY(VisibleAnywhere)
	TArray<float> AllDistancesToWater;

	void AddTriangles();
	void AddTrianglesOneAboveWater(TArray<FVertexData> vertexData);
	void AddTrianglesTwoAboveWater(TArray<FVertexData> vertexData);

	//some relavant info found here https://wiki.unrealengine.com/Accessing_mesh_triangles_and_vertex_positions_in_build
	bool GetStaticMeshVertexLocationsAndTriangles(UStaticMeshComponent* Comp, TArray<FVector>& GlobalVertexPositions, TArray<FVector>& LocalVertexPositions, TArray<int>& TriangleIndexes);

};
