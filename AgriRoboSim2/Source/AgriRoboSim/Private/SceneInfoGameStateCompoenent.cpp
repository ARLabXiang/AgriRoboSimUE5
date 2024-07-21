// Fill out your copyright notice in the Description page of Project Settings.


#include "SceneInfoGameStateCompoenent.h"

// Sets default values for this component's properties
USceneInfoGameStateCompoenent::USceneInfoGameStateCompoenent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USceneInfoGameStateCompoenent::BeginPlay()
{
	Super::BeginPlay();
	GameInfoMSG = MakeShareable(new ROSMessages::std_msgs::String());
	GameInfoTopic = NewObject<UTopic>(UTopic::StaticClass());
	GameInfoTopic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/SceneInfo"), TEXT("std_msgs/String"));
	GameInfoTopic->Advertise();
	// ...
	
}


// Called every frame
void USceneInfoGameStateCompoenent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USceneInfoGameStateCompoenent::AddEntryPair(FString Head, FString Body)
{
	Header.Add(Head);
	Information.Add(Body);
}

void USceneInfoGameStateCompoenent::Load()
{
	GameInfoMSG->_Data.Reset();
	GameInfoMSG->_Data.Append(FString::Join(Header,_T(",")));
	GameInfoMSG->_Data.Append(_T("\n"));
	GameInfoMSG->_Data.Append(FString::Join(Information,_T(",")));
}

void USceneInfoGameStateCompoenent::Publish()
{
	GameInfoTopic->Publish(GameInfoMSG);
}

void USceneInfoGameStateCompoenent::RequestAppend()
{
	AddEntryPair("level", GetWorld()->GetMapName());
}

void USceneInfoGameStateCompoenent::RequestNew()
{
	Header.Reset();
	Information.Reset();
	RequestAppend();
}

void USceneInfoGameStateCompoenent::LoadPublish()
{
	Load();
	Publish();
}

void USceneInfoGameStateCompoenent::RequestNewLoadPublish()
{
	RequestNew();
	LoadPublish();
}



