// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderWaterMeshGenerator.h"
#include "Components/StaticMeshComponent.h"
#include "PxTriangleMesh.h"
#include "PxVec3.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysXPublicCore.h"
#include "PxSimpleTypes.h"
#include "ProceduralMeshComponent.h"


void UUnderWaterMeshGenerator::GenerateUnderWaterMesh()
{	

	//UE_LOG(LogTemp, Warning, TEXT("GeneratUnderWaterMesh"));
	// get triangles below water
	UnderWaterTriangleData.Empty();

	//get distance to water
	for (int32 i = 0; i < MeshVertices.Num(); i++) {

		//The coordinate should be in global position
		FVector globalPos = MeshTransform.TransformPosition(MeshVertices[i]);

		//Save the global position so we only need to calculate it once here
		//And if we want to debug we can convert it back to local
		MeshVerticesGlobal[i] = globalPos;

		AllDistancesToWater[i] = globalPos.Z - 0;
	}

	AddTriangles();
}

void UUnderWaterMeshGenerator::DisplayMesh(UProceduralMeshComponent* UnderWaterMesh, TArray<FTriangleData> triangleData)
{	
	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector> normals;
	//Build the mesh
	for (int32 i = 0; i < triangleData.Num(); i++)
	{	
		
		//From global coordinates to local coordinates
		FVector p1 = MeshTransform.InverseTransformPosition(triangleData[i].p1);
		FVector p2 = MeshTransform.InverseTransformPosition(triangleData[i].p2);
		FVector p3 = MeshTransform.InverseTransformPosition(triangleData[i].p3);


		normals.Add((triangleData[i].normal));
		vertices.Add(p1);
		triangles.Add(vertices.Num() - 1);

		normals.Add((triangleData[i].normal));
		vertices.Add(p2);
		triangles.Add(vertices.Num() - 1);

		normals.Add((triangleData[i].normal));
		vertices.Add(p3);
		triangles.Add(vertices.Num() - 1);

	}

	
	//Remove the old mesh
	//UnderWaterMesh->ClearAllMeshSections();


	//UnderWaterMesh->CreateMeshSection(0, vertices, triangles, normals,, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

	//UnderWaterMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);

	// Enable collision data
	//UnderWaterMesh->ContainsPhysicsTriMeshData(false);

	//UnderWaterMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void UUnderWaterMeshGenerator::ModifyMesh(UStaticMeshComponent* Comp)
{
	MeshTransform = Comp->GetComponentTransform();
	//UE_LOG(LogTemp, Warning, TEXT("ModifiyMesh"));
	if (!GetStaticMeshVertexLocationsAndTriangles(Comp, MeshVerticesGlobal, MeshVertices, MeshTriangles)) {
		//UE_LOG(LogTemp,Warning,TEXT("Getting Vertices and Triangle Failed!!! Bad"));
	}

	AllDistancesToWater.Init(0, MeshVertices.Num() + 1);
}

void UUnderWaterMeshGenerator::AddTriangles()
{
	//List that will store the data we need to sort the vertices based on distance to water
	TArray<FVertexData> vertexData;

	//Add init data that will be replaced
	vertexData.Add(FVertexData());
	vertexData.Add(FVertexData());
	vertexData.Add(FVertexData());

	//UE_LOG(LogTemp, Warning, TEXT("Addtrias"));
	//Loop through all the triangles (3 vertices at a time = 1 triangle)
	int32 i = 0;
	
	while (i < MeshTriangles.Num())
	{	
		UE_LOG(LogTemp, Warning, TEXT("%d    %d"), MeshTriangles.Num(), i);
		//Loop through the 3 vertices
		for (int32 x = 0; x < 3; x++)
		{
			//Save the data we need

			//
			vertexData[x].distance = AllDistancesToWater[MeshTriangles[i]];

			vertexData[x].index = x;

			vertexData[x].globalVertexPos = MeshVerticesGlobal[MeshTriangles[i]];

			float f = AllDistancesToWater[MeshTriangles[i]];
			UE_LOG(LogTemp, Warning, TEXT("Distance to water %f"), f);

			i++;
		}


		//All vertices are above the water
		if (vertexData[0].distance > 0.0f && vertexData[1].distance > 0.0f && vertexData[2].distance > 0.0f)
		{	
			//UE_LOG(LogTemp, Warning, TEXT("Addtrias 1"));
			continue;
		}


		//Create the triangles that are below the waterline

		//All vertices are underwater
		if (vertexData[0].distance < 0.0f && vertexData[1].distance < 0.0f && vertexData[2].distance < 0.0f)
		{	
			//UE_LOG(LogTemp, Warning, TEXT("Addtrias 2"));
			FVector p1 = vertexData[0].globalVertexPos;
			FVector p2 = vertexData[1].globalVertexPos;
			FVector p3 = vertexData[2].globalVertexPos;

			//Save the triangle
			UnderWaterTriangleData.Add(FTriangleData(p1, p2, p3));
		}
		//1 or 2 vertices are below the water
		else
		{	
			//Sort the vertices, may need to reverse this
			vertexData.Sort([](const FVertexData& a, const FVertexData& b) { return a.distance < b.distance; });

			//vertexData.Sort((const FVertexData x, const FVertexData y) = > x.distance.CompareTo(y.distance));

			//One vertice is above the water, the rest is below
			if (vertexData[0].distance > 0.0f && vertexData[1].distance < 0.0f && vertexData[2].distance < 0.0f)
			{	

				//UE_LOG(LogTemp, Warning, TEXT("Addtrias 3"));
				AddTrianglesOneAboveWater(vertexData);
			}
			//Two vertices are above the water, the other is below
			else if (vertexData[0].distance > 0.0f && vertexData[1].distance > 0.0f && vertexData[2].distance < 0.0f)
			{	

				//UE_LOG(LogTemp, Warning, TEXT("Addtrias 4"));
				AddTrianglesTwoAboveWater(vertexData);
			}
		}
	}
}

void UUnderWaterMeshGenerator::AddTrianglesOneAboveWater(TArray<FVertexData> vertexData)
{
	//H is always at position 0
	FVector H = vertexData[0].globalVertexPos;

	//Left of H is M
	//Right of H is L

	//Find the index of M
	int M_index = vertexData[0].index - 1;
	if (M_index < 0)
	{
		M_index = 2;
	}

	//We also need the heights to water
	float h_H = vertexData[0].distance;
	float h_M = 0.0f;
	float h_L = 0.0f;

	FVector M = FVector::ZeroVector;
	FVector L = FVector::ZeroVector;
	//This means M is at position 1 in the List
	if (vertexData[1].index == M_index)
	{
		M = vertexData[1].globalVertexPos;
		L = vertexData[2].globalVertexPos;

		h_M = vertexData[1].distance;
		h_L = vertexData[2].distance;
	}
	else
	{
		M = vertexData[2].globalVertexPos;
		L = vertexData[1].globalVertexPos;

		h_M = vertexData[2].distance;
		h_L = vertexData[1].distance;
	}


	//Now we can calculate where we should cut the triangle to form 2 new triangles
	//because the resulting area will always form a square

	//Point I_M
	FVector MH = H - M;

	float t_M = -h_M / (h_H - h_M);

	FVector MI_M = t_M * MH;

	FVector I_M = MI_M + M;


	//Point I_L
	FVector LH = H - L;

	float t_L = -h_L / (h_H - h_L);

	FVector LI_L = t_L * LH;

	FVector I_L = LI_L + L;


	//Save the data, such as normal, area, etc      
	//2 triangles below the water  
	UnderWaterTriangleData.Add(FTriangleData(M, I_M, I_L));
	UnderWaterTriangleData.Add(FTriangleData(M, I_L, L));
}

void UUnderWaterMeshGenerator::AddTrianglesTwoAboveWater(TArray<FVertexData> vertexData)
{
	//H and M are above the water
			//H is after the vertice that's below water, which is L
			//So we know which one is L because it is last in the sorted list
	FVector L = vertexData[2].globalVertexPos;

	//Find the index of H
	int H_index = vertexData[2].index + 1;
	if (H_index > 2)
	{
		H_index = 0;
	}


	//We also need the heights to water
	float h_L = vertexData[2].distance;
	float h_H = 0.0f;
	float h_M = 0.0f;

	FVector H = FVector::ZeroVector;
	FVector M = FVector::ZeroVector;

	//This means that H is at position 1 in the list
	if (vertexData[1].index == H_index)
	{
		H = vertexData[1].globalVertexPos;
		M = vertexData[0].globalVertexPos;

		h_H = vertexData[1].distance;
		h_M = vertexData[0].distance;
	}
	else
	{
		H = vertexData[0].globalVertexPos;
		M = vertexData[1].globalVertexPos;

		h_H = vertexData[0].distance;
		h_M = vertexData[1].distance;
	}


	//Now we can find where to cut the triangle

	//Point J_M
	FVector LM = M - L;

	float t_M = -h_L / (h_M - h_L);

	FVector LJ_M = t_M * LM;

	FVector J_M = LJ_M + L;


	//Point J_H
	FVector LH = H - L;

	float t_H = -h_L / (h_H - h_L);

	FVector LJ_H = t_H * LH;

	FVector J_H = LJ_H + L;


	//Save the data, such as normal, area, etc
	//1 triangle below the water
	UnderWaterTriangleData.Add(FTriangleData(L, J_H, J_M));
}

bool UUnderWaterMeshGenerator::GetStaticMeshVertexLocationsAndTriangles(UStaticMeshComponent* Comp, TArray<FVector>& GlobalVertexPositions, TArray<FVector>& LocalVertexPositions, TArray<int>& TriangleIndexes)
{
	if (!Comp)
	{
		return false;
	}

	if (!Comp->IsValidLowLevel())
	{
		return false;
	}

	//Component Transform
	FTransform RV_Transform = Comp->GetComponentTransform();

	//Body Setup valid?
	UBodySetup* BodySetup = Comp->GetBodySetup();

	if (!BodySetup || !BodySetup->IsValidLowLevel())
	{
		return false;
	}

	//array as of 4.9
	for (PxTriangleMesh* EachTriMesh : BodySetup->TriMeshes)
	{
		if (!EachTriMesh)
		{
			return false;
		}
		
		//Number of vertices
		PxU32 VertexCount = EachTriMesh->getNbVertices();

		//Vertex array
		const PxVec3* Vertices = EachTriMesh->getVertices();

		//TRIANGLE POSITIONS 
		int32 TriNumber = EachTriMesh->getNbTriangles();
		const void* Triangles = EachTriMesh->getTriangles();

		// Grab triangle indices
		int32 I0, I1, I2;

		for (int32 TriIndex = 0; TriIndex < TriNumber; ++TriIndex)
		{	
			//could not work here
			if (EachTriMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
			{
				PxU16* P16BitIndices = (PxU16*)Triangles;
				I0 = P16BitIndices[(TriIndex * 3) + 0];
				I1 = P16BitIndices[(TriIndex * 3) + 1];
				I2 = P16BitIndices[(TriIndex * 3) + 2];
			}
			else
			{
				PxU32* P32BitIndices = (PxU32*)Triangles;
				I0 = P32BitIndices[(TriIndex * 3) + 0];
				I1 = P32BitIndices[(TriIndex * 3) + 1];
				I2 = P32BitIndices[(TriIndex * 3) + 2];
			}

			// Note amound of triangle indexes should always be 3x the amount of triangles
			TriangleIndexes.Add(I0);
			TriangleIndexes.Add(I1);
			TriangleIndexes.Add(I2);
		}

		// VERTEX POSITIONS

		//For each vertex, get global by transforming the position to match the component Transform, and get local
		for (PxU32 v = 0; v < VertexCount; v++)
		{
			GlobalVertexPositions.Add(RV_Transform.TransformPosition(P2UVector(Vertices[v])));
			LocalVertexPositions.Add(P2UVector(Vertices[v]));
		}
	}

	return true;
}

