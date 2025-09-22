// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScannerPrimitiveCheck.h"
#include "PhysicsEngine/BodySetup.h"
// Sets default values for this component's properties
UHitScannerPrimitiveCheck::UHitScannerPrimitiveCheck()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHitScannerPrimitiveCheck::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHitScannerPrimitiveCheck::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FString UHitScannerPrimitiveCheck::HitPrimitiveName(UPrimitiveComponent* OtherComp, int32 HitCompID, FVector& Center, float& Radius)
{
	FString name = "none";
	Center = FVector(0, 0, 0);
	if (HitCompID == -1) {return name;}
	if (OtherComp != nullptr && HitCompID > -1)
	{
		if (UStaticMeshComponent* mesh = Cast<UStaticMeshComponent>(OtherComp))
		{
			FKShapeElem* el = mesh->GetStaticMesh()->GetBodySetup()->AggGeom.GetElement(HitCompID);
			if(FKSphereElem* casted_el = static_cast<FKSphereElem*>(el)) {Center = casted_el->Center; Radius = casted_el->Radius;}
			name = el->GetName().ToString();
			
		}

		else if (UStaticMesh* mesh2 = Cast<UStaticMesh>(OtherComp))
		{
			FKShapeElem* el = mesh2->GetBodySetup()->AggGeom.GetElement(HitCompID);
			if(FKSphereElem* casted_el = static_cast<FKSphereElem*>(el)) {Center = casted_el->Center; Radius = casted_el->Radius;}
			name = el->GetName().ToString();
		}
	}
	return name;
}