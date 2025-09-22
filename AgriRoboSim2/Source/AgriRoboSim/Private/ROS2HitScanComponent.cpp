// Fill out your copyright notice in the Description page of Project Settings.


#include "ROS2HitScanComponent.h"
#include "geometry_msgs/Transform.h"

void UROS2HitScanComponent::InitHitScan(FString RobotTopicPrefix)
{
	if (!rosinst->ROSIntegrationCore) {return;}
	R2S_Received_Topic = NewObject<UTopic>(UTopic::StaticClass());
	R2S_Received_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+RobotTopicPrefix+TEXT("/hit_scans_location"), TEXT("std_msgs/Float32MultiArray"));
	// ...
	TargetLocation_SubscribeCallback = [_pos = &TargetLocations, _pix = &TargetImagePos](TSharedPtr<FROSBaseMsg> msg) -> void
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::Float32MultiArray>(msg);
		if (Concrete.IsValid())
		{
			_pos->Empty();
			_pix->Empty();
			size_t num_targest = Concrete->layout.dim[0].size;
			size_t stride = Concrete->layout.dim[0].stride;
			for (size_t i = 0; i < num_targest; i++)
			{
				_pos->Add(FVector(Concrete->data[i*stride], Concrete->data[i*stride+1], Concrete->data[i*stride+2]));
				_pix->Add((FVector(Concrete->data[i*stride+3],Concrete->data[i*stride+4],0)));
			}
				
			//R2S_TArray_Helper(Concrete->position, _pos);
			//R2S_TArray_Helper(Concrete->name, _name);
		}
	};
	R2S_Received_Topic->Subscribe(TargetLocation_SubscribeCallback);

	S2R_Return_Topic = NewObject<UTopic>(UTopic::StaticClass());
	S2R_Return_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+RobotTopicPrefix+TEXT("/hit_scans_info"), TEXT("std_msgs/Float32MultiArray"));
	S2R_Return_Topic->Advertise();
}

void UROS2HitScanComponent::PublishHitScan()
{
	if (!rosinst->ROSIntegrationCore) {return;}
	
	S2R_Return_Topic->Publish(return_msg);
}

// Called when the game starts
void UROS2HitScanComponent::BeginPlay()
{
	Super::BeginPlay();
	received_msg = MakeShareable(new ROSMessages::std_msgs::Float32MultiArray());
	return_msg = MakeShareable(new ROSMessages::std_msgs::Float32MultiArray());
}