// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
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
	

private:

	void InitVariables();

	UStaticMesh* ParentMesh;
	UPrimitiveComponent* ParentPrimitive;
	UUnderWaterMeshGenerator* UnderWaterMeshGenerator;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* UnderWaterMesh;

	UPROPERTY(VisibleAnywhere)
	float WaterDensity = 1025.0f;


	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* mesh;

	void CreateTriangle();	
};
