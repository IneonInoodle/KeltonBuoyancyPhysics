// Fill out your copyright notice in the Description page of Project Settings.


#include "BuoyancyActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "UnderWaterMeshGenerator.h"
#include "Private/KismetTraceUtils.h"

// Sets default values for this component's properties
UBuoyancyActorComponent::UBuoyancyActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	// ...
}


// Called when the game starts
void UBuoyancyActorComponent::BeginPlay()
{
	Super::BeginPlay();

	InitVariables();

	// ...
	
}




// Called every frame
void UBuoyancyActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UnderWaterMeshGenerator->GenerateUnderWaterMesh();

	//for debugging
	UnderWaterMeshGenerator->DisplayMesh(UnderWaterMesh, UnderWaterMeshGenerator->UnderWaterTriangleData);
	
	//NOTE!!!! Unreal doesnt have a fixed time step like unity, physics should actually be implemented by creating one https://forums.unrealengine.com/community/community-content-tools-and-tutorials/87505-using-a-fixed-physics-timestep-in-unreal-engine-free-the-physics-approach
	// in this case I did the lazy thing and just ignore this for now. But really should do 

	////Add forces to the part of the boat that's below the water -- TODO ADD TO FIXED TIMESTEP
	if (UnderWaterMeshGenerator->UnderWaterTriangleData.Num() > 0)
	{
		AddUnderWaterForces();
	}

}


// This is called when actor is already in level and map is opened
void UBuoyancyActorComponent::PostLoad()
{
	Super::PostLoad();


	//CreateTriangle();
	UE_LOG(LogTemp, Warning, TEXT("PostLoad"));
}


void UBuoyancyActorComponent::InitVariables()
{	
	ParentMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>()->GetStaticMesh();
	ParentPrimitive = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
	UnderWaterMeshGenerator = NewObject<UUnderWaterMeshGenerator>();
}

void UBuoyancyActorComponent::AddUnderWaterForces()
{
	//Get all triangles
	TArray<FTriangleData> underWaterTriangleData = UnderWaterMeshGenerator->UnderWaterTriangleData;

	for (int i = 0; i < underWaterTriangleData.Num(); i++)
	{
		//This triangle
		FTriangleData triangleData = underWaterTriangleData[i];

		//Calculate the buoyancy force
		FVector buoyancyForce = BuoyancyForce(WaterDensity, triangleData);

		//Add the force to the boat
		ParentPrimitive->AddForceAtLocation(buoyancyForce, triangleData.center);


		//Debug

	
		//Normal
		DrawDebugLine(
			GetWorld(),
			triangleData.center,
			triangleData.center + triangleData.normal * 3.0f,
			FColor::Green,
			false, -1, 0,
			12.333
		);

		//Buoyancy

		DrawDebugLine(
			GetWorld(),
			triangleData.center,
			triangleData.center + buoyancyForce.Normalize() * -3.0f,
			FColor::Blue,
			false, -1, 0,
			12.333
		);
	}
}

// found here https://www.habrador.com/tutorials/unity-boat-tutorial/3-buoyancy/
FVector UBuoyancyActorComponent::BuoyancyForce(float rho, FTriangleData triangleData)
{
	//Buoyancy is a hydrostatic force - it's there even if the water isn't flowing or if the boat stays still

			// F_buoyancy = rho * g * V
			// rho - density of the mediaum you are in
			// g - gravity
			// V - volume of fluid directly above the curved surface 

			// V = z * S * n 
			// z - distance to surface
			// S - surface area
			// n - normal to the surface
	
	FVector buoyancyForce = rho * GetWorld()->GetGravityZ() * triangleData.distanceToSurface * triangleData.area * triangleData.normal;

	//The vertical component of the hydrostatic forces don't cancel out but the horizontal do
	buoyancyForce.X = 0.0f;
	buoyancyForce.Y = 0.0f;

	return buoyancyForce;
}

void UBuoyancyActorComponent::CreateTriangle()
{
	TArray<FVector> vertices;
	vertices.Add(FVector(0, 0, 0));
	vertices.Add(FVector(0, 100, 0));
	vertices.Add(FVector(0, 0, 100));

	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);

	TArray<FVector> normals;
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));

	TArray<FVector2D> UV0;
	UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(10, 0));
	UV0.Add(FVector2D(0, 10));


	TArray<FProcMeshTangent> tangents;
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(false);
}

