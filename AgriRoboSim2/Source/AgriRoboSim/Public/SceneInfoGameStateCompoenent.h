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
	TSharedPtr<ROSMessages::std_msgs::String> GameInfoMSG;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TArray<FString> Header;
	TArray<FString> Information;
	UFUNCTION(BlueprintCallable)
	void AddEntryPair(FString Head, FString Body);
	void Load();
	UFUNCTION(BlueprintCallable)
	void Publish();
	UFUNCTION(BlueprintCallable)
	void RequestAppend();
	void RequestNew();
	UFUNCTION(BlueprintCallable)
	void LoadPublish();
	UFUNCTION(BlueprintCallable)
	void RequestNewLoadPublish();
};
