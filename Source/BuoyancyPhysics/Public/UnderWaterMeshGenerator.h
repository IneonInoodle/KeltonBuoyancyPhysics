// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "UnderWaterMeshGenerator.generated.h"

class UStaticMeshComponent;
class UProceduralMeshComponent;
class UPrimitiveComponent;

/**
 * 
 */

 //Data that belongs to one triangle in the original boat mesh
 //and is needed to calculate the slamming force
USTRUCT(BlueprintType)
struct FSlammingForceData
{	
	GENERATED_BODY()

	//The area of the original triangles - calculate once in the beginning because always the same
	float originalArea;
	//How much area of a triangle in the whole boat is submerged
	float submergedArea;
	//Same as above but previous time step
	float previousSubmergedArea;
	//Need to save the center of the triangle to calculate the velocity
	FVector triangleCenter;
	//Velocity
	FVector velocity;
	//Same as above but previous time step
	FVector previousVelocity;
};

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

	//The velocity of the triangle at the center
	FVector velocity;

	//The velocity normalized
	FVector velocityDir;

	//The angle between the normal and the velocity
	//Negative if pointing in the opposite direction
	//Positive if pointing in the same direction
	float cosTheta;

	//normally this would be the construction but unreal doesnt allow it so have to make it a method
	FTriangleData(FVector Newp1, FVector Newp2, FVector Newp3, UPrimitiveComponent* ParentPrim) {
		p1 = Newp1;
		p2 = Newp2;
		p3 = Newp3;

		//Center of the triangle
		center = (p1 + p2 + p3) / 3.0f;
		
		//float MyTime = GetWorld()->GetTimeSeconds(); 

		//Distance to the surface from the center of the triangle
		distanceToSurface = FVector::Distance(FVector::ZeroVector, center);

		//Normal to the triangle
		normal = FVector::CrossProduct(p2 - p3, p1 - p3).GetClampedToSize(-1,1);
		

		//Triangle Area

		// formula to get angle between two vectors found here https://www.jofre.de/?page_id=1297#item6
		float angle = FMath::Atan2(FVector::CrossProduct(p2 - p1, p3 - p1).Normalize(), FVector::DotProduct(p2 - p1, p3 - p1));

		//Area of the triangle
		float a = FVector::Distance(p1, p2);
		float c = FVector::Distance(p3, p1);	
		area = (a * c * FMath::Sin(FMath::RadiansToDegrees(angle))) / 2.0f;


		//Get triangle velocity 
		FVector v_B = ParentPrim->GetComponentVelocity();

		FVector omega_B = ParentPrim->GetPhysicsAngularVelocity();

		//TODO might need to make sure this is world space
		FVector r_BA = center - ParentPrim->GetCenterOfMass();
		FVector v_A = v_B + FVector::CrossProduct(omega_B, r_BA);

		//Velocity vector of the triangle at the center
		velocity = v_A;

		//TODO CHECK
		//Velocity direction
		velocityDir = velocity.GetSafeNormal();

		//Angle between the normal and the velocity
		//Negative if pointing in the opposite direction
		//Positive if pointing in the same direction
		cosTheta = FVector::DotProduct(velocityDir, normal);

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

	//The part of the boat that's above water
	UPROPERTY(VisibleAnywhere)
	TArray<FTriangleData> aboveWaterTriangleData;

	//Slamming resistance forces
   //Data that belongs to one triangle in the original boat mesh
	TArray<FSlammingForceData> slammingForceData;
	//To connect the submerged triangles with the original triangles
    TArray<int32> indexOfOriginalTriangle;
	//The total area of the entire boat
	float boatArea;

	float timeSinceStart;


	void GenerateUnderWaterMesh();
	void DisplayMesh(UProceduralMeshComponent* UnderWaterMesh, TArray<FTriangleData> triangleData);
	void ModifyMesh(UStaticMeshComponent* Comp, UPrimitiveComponent* Prim, UProceduralMeshComponent* pmc);
	float CalculateUnderWaterLength();
private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ParentMesh;
	UPROPERTY(VisibleAnywhere)
	UPrimitiveComponent* ParentPrim;
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* underWaterMesh;

	UPROPERTY(VisibleAnywhere)
	FTransform MeshTransform;
	UPROPERTY(VisibleAnywhere)
	TArray<int> MeshTriangles;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> MeshVertices;
	UPROPERTY(VisibleAnywhere)
	TArray<float> AllDistancesToWater;

	void AddTriangles();
	void AddTrianglesOneAboveWater(TArray<FVertexData> vertexData, int32 triangleCounter);
	void AddTrianglesTwoAboveWater(TArray<FVertexData> vertexData, int32 triangleCounter);
	void CalculateOriginalTrianglesArea();

	//some relavant info found here https://wiki.unrealengine.com/Accessing_mesh_triangles_and_vertex_positions_in_build
	bool GetStaticMeshVertexLocationsAndTriangles(UStaticMeshComponent* Comp, TArray<FVector>& GlobalVertexPositions, TArray<FVector>& LocalVertexPositions, TArray<int>& TriangleIndexes);
	
	// TODO should create static helper library class so I dont have to have this in two places
	float GetTriangleArea(FVector p1, FVector p2, FVector p3);
};
