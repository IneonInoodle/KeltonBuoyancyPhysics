// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "UnderWaterMeshGenerator.h"
#include "BuoyancyActorComponent.generated.h"

class UStaticMesh;
class UUnderWaterMeshGenerator;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUOYANCYPHYSICS_API UBuoyancyActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuoyancyActorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void PostLoad();
	
	UPROPERTY(VisibleAnywhere)
	UUnderWaterMeshGenerator* UnderWaterMeshGenerator;
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* UnderWaterMesh;
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* AboveWaterMesh;

	//to shift the center of mass
	UPROPERTY(VisibleAnywhere)
	FVector CenterOfMass;
private:
	
	UPROPERTY(VisibleAnywhere)
	UStaticMesh* ParentMesh;
	UPROPERTY(VisibleAnywhere)
	UPrimitiveComponent* ParentPrimitive;

	//Note RHO of water in real life is normally 1000kg/m^3 
	UPROPERTY(VisibleAnywhere)
	float WaterDensity = 1.0f;
	UPROPERTY(VisibleAnywhere)
	float AirDensity = 0.001f;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* mesh;

	void InitVariables();
	void AddUnderWaterForces();
	void AddAboveWaterForces();
	void CreateTriangle();
	FVector CheckForceIsValid(FVector force, FString forceName);


	//Physics Equations!!
	FVector BuoyancyForce(float rho, FTriangleData triangleData);
	FVector GetTriangleVelocity(UPrimitiveComponent* parentPrimitive, FVector triangleCenter);
	float GetTriangleArea(FVector p1, FVector p2, FVector p3);
	FVector ViscousWaterResistanceForce(float rho, FTriangleData triangleData, float Cf);
	float GetResistanceCoefficient(float rho, float velocity, float length);
	FVector PressureDragForce(FTriangleData triangleData);
	FVector SlammingForce(FSlammingForceData slammingData, FTriangleData triangleData, float boatArea, float boatMass);
	FVector AirResistanceForce(float rho, FTriangleData triangleData, float C_air);
	void CalculateSlammingVelocities(TArray<FSlammingForceData> slammingForceData);


};
