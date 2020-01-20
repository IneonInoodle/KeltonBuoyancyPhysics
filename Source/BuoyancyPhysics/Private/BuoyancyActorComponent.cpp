// Fill out your copyright notice in the Description page of Project Settings.


#include "BuoyancyActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "UnderWaterMeshGenerator.h"
#include "Private/KismetTraceUtils.h"
#include "GenericPlatformMath.h"

// Sets default values for this component's properties
UBuoyancyActorComponent::UBuoyancyActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	UnderWaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("UnderwaterMesh"));
	AboveWaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("UnderwaterMesh"));
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

	if (UnderWaterMeshGenerator->AboveWaterTriangleData.Num() > 0)
	{
		AddAboveWaterForces();
	}

}


void UBuoyancyActorComponent::InitVariables()
{	
	ParentMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>()->GetStaticMesh();
	ParentPrimitive = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
	
	UnderWaterMeshGenerator = NewObject<UUnderWaterMeshGenerator>();
	
	UnderWaterMeshGenerator->ModifyMesh(GetOwner()->FindComponentByClass<UStaticMeshComponent>());
	//shift center of mass
	ParentPrimitive->SetCenterOfMass(CenterOfMass);
}

void UBuoyancyActorComponent::AddUnderWaterForces()
{

	float Cf = GetResistanceCoefficient(
		WaterDensity,
		ParentPrimitive->ComponentVelocity.Size(),
		UnderWaterMeshGenerator->CalculateUnderWaterLength());

	//To calculate the slamming force we need the velocity at each of the original triangles
	TArray<FSlammingForceData> slammingForceData = UnderWaterMeshGenerator->slammingForceData;

	CalculateSlammingVelocities(slammingForceData);

	//To connect the submerged triangles with the original triangles
	TArray<int> indexOfOriginalTriangle = UnderWaterMeshGenerator->indexOfOriginalTriangle;

	//Need this data for slamming forces
	float boatArea = UnderWaterMeshGenerator->boatArea;
	float boatMass = ParentPrimitive->GetMass(); //Replace this line with your boat's total mass

	//Get all triangles
	TArray<FTriangleData> underWaterTriangleData = UnderWaterMeshGenerator->UnderWaterTriangleData;

	for (int i = 0; i < underWaterTriangleData.Num(); i++)
	{
		//This triangle
		FTriangleData triangleData = underWaterTriangleData[i];




		//Calculate the forces
		FVector forceToAdd = FVector::ZeroVector;

		//Force 1 - The hydrostatic force (buoyancy)
		forceToAdd += BuoyancyForce(WaterDensity, triangleData);

		//Force 2 - Viscous Water Resistance
		forceToAdd += ViscousWaterResistanceForce(WaterDensity, triangleData, Cf);

		//Force 3 - Pressure drag
		forceToAdd += PressureDragForce(triangleData);

		//Force 4 - Slamming force
		//Which of the original triangles is this triangle a part of
		int originalTriangleIndex = indexOfOriginalTriangle[i];

		FSlammingForceData slammingData = slammingForceData[originalTriangleIndex];

		forceToAdd += SlammingForce(slammingData, triangleData, boatArea, boatMass);

		//Add the force to the boat
		ParentPrimitive->AddForceAtLocation(forceToAdd, triangleData.center);

		//Debug

		//Normal		
		DrawDebugLine(
			GetWorld(),
			triangleData.center,
			triangleData.center + triangleData.normal * 3.0f,
			FColor::Green,
			false, -1, 2,
			1
		);
		/*
		DrawDebugPoint(
			GetWorld(),
			triangleData.center,
			10.0f,
			FColor::Purple,
			false, -1, 2	
		);*/

		//Buoyancy

		DrawDebugLine(
			GetWorld(),
			triangleData.center,
			triangleData.center + buoyancyForce.Normalize() * -3.0f,
			FColor::Red,
			false, -1, 0,
			1
		);
	}
}

void UBuoyancyActorComponent::AddAboveWaterForces()
{
	//Get all triangles
	TArray<FTriangleData> aboveWaterTriangleData = UnderWaterMeshGenerator->aboveWaterTriangleData;

	//Loop through all triangles
	for (int32 i = 0; i < aboveWaterTriangleData.Num(); i++)
	{
		FTriangleData triangleData = aboveWaterTriangleData[i];


		//Calculate the forces
		FVector forceToAdd = FVector::ZeroVector;

		//Force 1 - Air resistance 
		//Replace VisbyData.C_r with your boat's drag coefficient
		//TODO add variable for drag coefficent
		forceToAdd += AirResistanceForce(AirDensity, triangleData, 1.0f);

		//Add the forces to the boat
		ParentPrimitive->AddForceAtLocation(forceToAdd, triangleData.center);


		//Debug

		//The normal
		//Debug.DrawRay(triangleCenter, triangleNormal * 3f, Color.white);

		//The velocity
		//Debug.DrawRay(triangleCenter, triangleVelocityDir * 3f, Color.black);

		if (triangleData.cosTheta > 0.0f)
		{
			//Debug.DrawRay(triangleCenter, triangleVelocityDir * 3f, Color.black);
		}

		//Air resistance
		//-3 to show it in the opposite direction to see what's going on
		//Debug.DrawRay(triangleCenter, airResistanceForce.normalized * -3f, Color.blue);
	}
}

// found here formula found here http://www.gamasutra.com/view/news/237528/Water_interaction_model_for_boats_in_video_games.php
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
	//

	FVector buoyancyForce = rho * GetWorld()->GetGravityZ() * triangleData.distanceToSurface * triangleData.area * triangleData.normal;
	
	//The vertical component of the hydrostatic forces don't cancel out but the horizontal do
	buoyancyForce.X = 0.0f;
	buoyancyForce.Y = 0.0f;

	return buoyancyForce;
}

FVector UBuoyancyActorComponent::GetTriangleVelocity(UPrimitiveComponent* parentPrimitive, FVector triangleCenter)
{
	FVector v_B = parentPrimitive->GetComponentVelocity();

	FVector omega_B = parentPrimitive->GetPhysicsAngularVelocity();

	//TODO might need to make sure this is world space
	FVector r_BA = triangleCenter - parentPrimitive->GetCenterOfMass();

	FVector v_A = v_B + FVector::CrossProduct(omega_B, r_BA);

	return v_A;
}

float UBuoyancyActorComponent::GetTriangleArea(FVector p1, FVector p2, FVector p3)
{
	//Alternative 1 - Heron's formula
	float a = FVector::Distance(p1, p2);
	//float b = Vector3.Distance(vertice_2_pos, vertice_3_pos);
	float c = FVector::Distance(p3, p1);

	// formula to get angle between two vectors found here https://www.jofre.de/?page_id=1297#item6
	float angle = FMath::Atan2(FVector::CrossProduct(p2 - p1, p3 - p1).Normalize(), FVector::DotProduct(p2 - p1, p3 - p1));
	float area = (a * c * FMath::Sin(FMath::RadiansToDegrees(angle))) / 2.0f;

	return area;
}

//
// Resistance forces from http://www.gamasutra.com/view/news/263237/Water_interaction_model_for_boats_in_video_games_Part_2.php
//
FVector UBuoyancyActorComponent::ViscousWaterResistanceForce(float rho, FTriangleData triangleData, float Cf)
{
	//Viscous resistance occurs when water sticks to the boat's surface and the boat has to drag that water with it

	   // F = 0.5 * rho * v^2 * S * Cf
	   // rho - density of the medium you have
	   // v - speed
	   // S - surface area
	   // Cf - Coefficient of frictional resistance

	   //We need the tangential velocity 
	   //Projection of the velocity on the plane with the normal normalvec
	   //http://www.euclideanspace.com/maths/geometry/elements/plane/lineOnPlane/
	FVector B = triangleData.normal;
	FVector A = triangleData.velocity;

	FVector velocityTangent = FVector::CrossProduct(B, (FVector::CrossProduct(A, B) / B.Size())) / B.Size();

	//The direction of the tangential velocity (-1 to get the flow which is in the opposite direction)
	//TODO CHECK
	FVector tangentialDirection = velocityTangent.GetSafeNormal() * -1.0f;

	//Debug.DrawRay(triangleCenter, tangentialDirection * 3f, Color.black);
	//Debug.DrawRay(triangleCenter, velocityVec.normalized * 3f, Color.blue);
	//Debug.DrawRay(triangleCenter, normal * 3f, Color.white);

	//The speed of the triangle as if it was in the tangent's direction
	//So we end up with the same speed as in the center of the triangle but in the direction of the flow
	FVector v_f_vec = triangleData.velocity.Size() * tangentialDirection;

	//The final resistance force
	FVector viscousWaterResistanceForce = 0.5f * rho * v_f_vec.Size() * v_f_vec * triangleData.area * Cf;

	viscousWaterResistanceForce = CheckForceIsValid(viscousWaterResistanceForce, "Viscous Water Resistance");

	return viscousWaterResistanceForce;
}

//The Coefficient of frictional resistance - belongs to Viscous Water Resistance but is same for all so calculate once
float UBuoyancyActorComponent::GetResistanceCoefficient(float rho, float velocity, float length)
{
	//Reynolds number

	// Rn = (V * L) / nu
	// V - speed of the body
	// L - length of the sumbmerged body
	// nu - viscosity of the fluid [m^2 / s]

	//Viscocity depends on the temperature, but at 20 degrees celcius:
	float nu = 0.000001f;
	//At 30 degrees celcius: nu = 0.0000008f; so no big difference

	//Reynolds number
	float Rn = (velocity * length) / nu;

	//The resistance coefficient
	float Cf = 0.075f / FMath::Pow((FMath::LogX(10.0f,Rn) - 2.0f), 2.0f);

	return Cf;
}

FVector UBuoyancyActorComponent::SlammingForce(FSlammingForceData slammingData, FTriangleData triangleData, float boatArea, float boatMass)
{
	//To capture the response of the fluid to sudden accelerations or penetrations

	   //Add slamming if the normal is in the same direction as the velocity (the triangle is not receding from the water)
	   //Also make sure thea area is not 0, which it sometimes is for some reason
	if (triangleData.cosTheta < 0f || slammingData.originalArea <= 0.0f)
	{
		return FVector::Zero;
	}

	
	//Step 1 - Calculate acceleration
	//Volume of water swept per second
	FVector dV = slammingData.submergedArea * slammingData.velocity;
	FVector dV_previous = slammingData.previousSubmergedArea * slammingData.previousVelocity;

	//Calculate the acceleration of the center point of the original triangle (not the current underwater triangle)
	//But the triangle the underwater triangle is a part of
	FVector accVec = (dV - dV_previous) / (slammingData.originalArea * GetWorld()->DeltaTimeSeconds);

	//The magnitude of the acceleration
	float acc = accVec.Size();

	//Debug.Log(slammingForceData.originalArea);

	//Step 2 - Calculate slamming force
	// F = clamp(acc / acc_max, 0, 1)^p * cos(theta) * F_stop
	// p - power to ramp up slamming force - should be 2 or more

	// F_stop = m * v * (2A / S)
	// m - mass of the entire boat
	// v - velocity
	// A - this triangle's area
	// S - total surface area of the entire boat

	FVector F_stop = boatMass * triangleData.velocity * ((2.0f * triangleData.area) / boatArea);

	//float p = DebugPhysics.current.p;

	//float acc_max = DebugPhysics.current.acc_max;

	float p = 2.0f;

	float acc_max = acc;

	float slammingCheat = DebugPhysics.current.slammingCheat;

	FVector slammingForce = FMath::Pow(FMath::Clamp(acc / acc_max,0.0f,1.0f), p) * triangleData.cosTheta * F_stop * slammingCheat;

	//Vector3 slammingForce = Vector3.zero;

	//Debug.Log(slammingForce);

	//The force acts in the opposite direction
	slammingForce *= -1.0f;

	slammingForce = CheckForceIsValid(slammingForce, "Slamming");

	return slammingForce;

}

//Force 3 - Air resistance on the part of the ship above the water (typically 4 to 8 percent of total resistance)
FVector UBuoyancyActorComponent::AirResistanceForce(float rho, FTriangleData triangleData, float C_air)
{
	// R_air = 0.5 * rho * v^2 * A_p * C_air
	   // rho - air density
	   // v - speed of ship
	   // A_p - projected transverse profile area of ship
	   // C_r - coefficient of air resistance (drag coefficient)

	   //Only add air resistance if normal is pointing in the same direction as the velocity
	if (triangleData.cosTheta < 0f)
	{
		return FVector::ZeroVector;
	}

	//Find air resistance force
	FVector airResistanceForce = 0.5f * rho * triangleData.velocity.magnitude * triangleData.velocity * triangleData.area * C_air;

	//Acting in the opposite side of the velocity
	airResistanceForce *= -1.0f;

	airResistanceForce = CheckForceIsValid(airResistanceForce, "Air resistance");

	return airResistanceForce;
}

// found here as well https://www.habrador.com/tutorials/unity-boat-tutorial/5-resistance-forces/
FVector UBuoyancyActorComponent::PressureDragForce(FTriangleData triangleData)
{

	

	//Modify for different turning behavior and planing forces
		//f_p and f_S - falloff power, should be smaller than 1
		//C - coefficients to modify 

	float velocity = triangleData.velocity.Size();

	//A reference speed used when modifying the parameters
	float velocityReference = velocity;

	velocity = velocity / velocityReference;

	FVector pressureDragForce = FVector::ZeroVector;

	if (triangleData.cosTheta > 0.0f)
	{
		//float C_PD1 = 10f;
		//float C_PD2 = 10f;
		//float f_P = 0.5f;

		//To change the variables real-time - add the finished values later
		float C_PD1 = DebugPhysics.current.C_PD1;
		float C_PD2 = DebugPhysics.current.C_PD2;
		float f_P = DebugPhysics.current.f_P;

		pressureDragForce = -(C_PD1 * velocity + C_PD2 * (velocity * velocity)) * triangleData.area * FMath::Pow(triangleData.cosTheta, f_P) * triangleData.normal;
	}
	else
	{
		//float C_SD1 = 10f;
		//float C_SD2 = 10f;
		//float f_S = 0.5f;

		//To change the variables real-time - add the finished values later
		float C_SD1 = DebugPhysics.current.C_SD1;
		float C_SD2 = DebugPhysics.current.C_SD2;
		float f_S = DebugPhysics.current.f_S;

		pressureDragForce = (C_SD1 * velocity + C_SD2 * (velocity * velocity)) * triangleData.area * FMath::Pow(Mathf.Abs(triangleData.cosTheta), f_S) * triangleData.normal;
	}

	pressureDragForce = CheckForceIsValid(pressureDragForce, "Pressure drag");

	return pressureDragForce;
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

FVector UBuoyancyActorComponent::CheckForceIsValid(FVector force, FString forceName)
{
	if (!FGenericPlatformMath::IsNaN(force.X + force.Y + force.Z))
	{
		return force;
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("Force in Nan"));

		return FVector::ZeroVector;
	}
}

