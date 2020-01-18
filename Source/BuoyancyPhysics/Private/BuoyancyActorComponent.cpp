// Fill out your copyright notice in the Description page of Project Settings.


#include "BuoyancyActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UnderWaterMeshGenerator.h"

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

	// ...
	
}




// Called every frame
void UBuoyancyActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);



	// ...
}


// This is called when actor is already in level and map is opened
void UBuoyancyActorComponent::PostLoad()
{
	Super::PostLoad();

	InitVariables();
	CreateTriangle();
	UE_LOG(LogTemp, Warning, TEXT("PostLoad"));
}


void UBuoyancyActorComponent::InitVariables()
{	
	ParentMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>()->GetStaticMesh();
	ParentPrimitive = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
	UnderWaterMeshGenerator = NewObject<UUnderWaterMeshGenerator>();
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
	mesh->ContainsPhysicsTriMeshData(true);
}

