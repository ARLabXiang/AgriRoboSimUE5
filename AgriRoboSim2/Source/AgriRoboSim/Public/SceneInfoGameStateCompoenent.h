// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ROS2NodeComponent.h"
#include "std_msgs/String.h"
#include "std_msgs/UInt8MultiArray.h"
#include "SceneInfoGameStateCompoenent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGRIROBOSIM_API USceneInfoGameStateCompoenent : public UROS2NodeComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USceneInfoGameStateCompoenent();
	UPROPERTY()
	UTopic* GameInfoTopic;
	TSharedPtr<ROSMessages::std_msgs::String> StaticGameInfoMSG;
	TSharedPtr<ROSMessages::std_msgs::String> PeriodGameInfoMSG;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	TArray<FString> StaticHeader;
	TArray<FString> StaticInformation;
	TArray<FString> PeriodHeader;
	TArray<FString> PeriodInformation;
	UFUNCTION(BlueprintCallable)
	void AddStaticEntryPair(FString Head, FString Body);
	UFUNCTION(BlueprintCallable)
	void AddPeriodEntryPair(FString Head, FString Body);

	UFUNCTION(BlueprintCallable)
	void RequestStaticNew();
	UFUNCTION(BlueprintCallable)
	void RequestPeriodNew();
	UFUNCTION(BlueprintCallable)
	void LoadStaticPublish();
	UFUNCTION(BlueprintCallable)
	void LoadPeriodPublish();
};
