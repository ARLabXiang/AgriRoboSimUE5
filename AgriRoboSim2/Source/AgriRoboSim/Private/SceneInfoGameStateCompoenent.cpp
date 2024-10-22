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
	StaticGameInfoMSG = MakeShareable(new ROSMessages::std_msgs::String());
	PeriodGameInfoMSG = MakeShareable(new ROSMessages::std_msgs::String());
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

void USceneInfoGameStateCompoenent::AddStaticEntryPair(FString Head, FString Body)
{
	StaticHeader.Add(Head);
	StaticInformation.Add(Body);
}

void USceneInfoGameStateCompoenent::AddPeriodEntryPair(FString Head, FString Body)
{
	PeriodHeader.Add(Head);
	PeriodInformation.Add(Body);
}

void USceneInfoGameStateCompoenent::RequestStaticNew()
{
	StaticHeader.Reset();
	StaticInformation.Reset();
	AddStaticEntryPair("level-static", GetWorld()->GetMapName());
}
void USceneInfoGameStateCompoenent::RequestPeriodNew()
{
	PeriodHeader.Reset();
	PeriodInformation.Reset();
	AddPeriodEntryPair("level-period", GetWorld()->GetMapName());
}

void USceneInfoGameStateCompoenent::LoadStaticPublish()
{
	StaticGameInfoMSG->_Data.Reset();
	StaticGameInfoMSG->_Data.Append(FString::Join(StaticHeader,_T("||")));
	StaticGameInfoMSG->_Data.Append(_T("\n"));
	StaticGameInfoMSG->_Data.Append(FString::Join(StaticInformation,_T("||")));

	GameInfoTopic->Publish(StaticGameInfoMSG);
}
void USceneInfoGameStateCompoenent::LoadPeriodPublish()
{
	PeriodGameInfoMSG->_Data.Reset();
	PeriodGameInfoMSG->_Data.Append(FString::Join(PeriodHeader,_T("||")));
	PeriodGameInfoMSG->_Data.Append(_T("\n"));
	PeriodGameInfoMSG->_Data.Append(FString::Join(PeriodInformation,_T("||")));

	GameInfoTopic->Publish(PeriodGameInfoMSG);
}
//
// void USceneInfoGameStateCompoenent::RequestNewLoadPublish()
// {
// 	RequestNew();
// 	LoadPublish();
// }



