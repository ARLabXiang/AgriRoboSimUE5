// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSSceneCapture.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneCaptureComponent2D.h"


// Sets default values for this component's properties
UROSSceneCapture::UROSSceneCapture()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// // Called when the game starts
// void UROSSceneCapture::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	// ...
// 	
// }
//
//
// // Called every frame
// void UROSSceneCapture::TickComponent(float DeltaTime, ELevelTick TickType,
//                                      FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
// 	// ...
// }

void UROSSceneCapture::Initialize(
	UROSIntegrationGameInstance* rosinst,
	FString& topic_name_,
	USceneCaptureComponent2D* SceneCapture_,
	ECaptureType CaptureType_
)
{
	topic_name = topic_name_;
	UE_LOG(LogTemp, Log, TEXT("A %s"),*topic_name)
	SceneCapture = SceneCapture_;
	CaptureType = CaptureType_;
	switch (CaptureType)
	{
	case ECaptureType::ColorCapture:
	case ECaptureType::SegmentationCapture:
		RenderTargetFormat = RTF_RGBA8;
		ROSStepMultiplier = 3;
		break;
	case ECaptureType::DepthCapture:
		RenderTargetFormat = RTF_RGBA32f;
		ROSStepMultiplier = 4;
		break;
	default:
		break;
	}
	if (!rosinst->ROSIntegrationCore) {return;}
	Topic = NewObject<UTopic>(UTopic::StaticClass());
	Topic->Init(rosinst->ROSIntegrationCore, topic_name, TEXT("sensor_msgs/Image"));
	Topic->Advertise();
}

/**
 * Compute the size of the ROS topic message size
 */
void UROSSceneCapture::RefreshImageTopicSize()
{
	//ROSEncoding = SceneCapture->TextureTarget->GetFormat();
	ImageMSG = MakeShareable(new ROSMessages::sensor_msgs::Image());
	ImageMSG->encoding = CheckROSEncoding();
	img = std::make_shared<uint8[]>(
		SceneCapture->TextureTarget->SizeX *
		SceneCapture->TextureTarget->SizeY *
		ROSStepMultiplier);
	ImageMSG->data = img.get();
	ImageMSG->header = ROSMessages::std_msgs::Header(0,FROSTime(0,0).Now(),"map");
	ImageMSG->width = SceneCapture->TextureTarget->SizeX;
	ImageMSG->height = SceneCapture->TextureTarget->SizeY;
	ImageMSG->is_bigendian = false;
	ImageMSG->step = SceneCapture->TextureTarget->SizeX * ROSStepMultiplier;
}

/**
 * read from current scene capture texture and publish it according to the datatype
 */
void UROSSceneCapture::Publish()
{
	switch (RenderTargetFormat)
	{
	case RTF_RGBA8:
		ReadRenderTargetPerRHI();
		Publish(&ImageData8Bit);
		break;
	case RTF_R16f:
	case RTF_RG16f:
	case RTF_RGBA16f:
		ReadRenderTargetPerRHI();
		Publish(&ImageData16Bit);
		break;
	case RTF_R32f:
	case RTF_RG32f:
	case RTF_RGBA32f:
		ReadRenderTargetPerRHI();
		Publish(&ImageData32Bit);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unimplemented Format"))
	}
}

/**
 * converts the ue5 TArray image to ROS image datatype then publish it over the topic
 * @tparam T datatype for the image
 * @param Image TArray holding the ue5 texture 
 */
template<typename T>
void UROSSceneCapture::Publish(TArray<T>* Image)
{
	if (UpdateImageMsg(Image, img.get()) && Topic)
	{
		//UE_LOG(LogTemp, Log, TEXT("publishing, %d, %p, %d"), Image->Num(), img.get(), img.get()!=nullptr)
		Topic->Publish(ImageMSG);
		//UE_LOG(LogTemp, Log, TEXT("published"))
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("failed publish"))
}

/**
 * fill the ROS buffer with UE5 TArray information based on the TArray datatype
 * @tparam T datatype for the image
 * @param Image UE5 image in form of TArray
 * @param data ROS image buffer
 * @return whether the buffer was filled
 */
template <typename T>
bool UROSSceneCapture::UpdateImageMsg(TArray<T>* Image, uint8* data)
{
	if (Image->Num() < 100)
	{
		UE_LOG(LogTemp, Log, TEXT("no image msg"))
		return false;
	}
	ImageMSG->header.time = FROSTime().Now();
	CheckROSEncoding();
	if constexpr (std::is_same_v<T, FColor>)
	{
		for (int i = 0; i<Image->Num(); i++)
		{
			data[i*ROSStepMultiplier] = Image->GetData()[i].R;
			data[i*ROSStepMultiplier+1] = Image->GetData()[i].G;
			data[i*ROSStepMultiplier+2] = Image->GetData()[i].B;
			//data[i*4+3] = Image->GetData()[i].A;
		}
	} else if constexpr (std::is_same_v<T, FLinearColor>)
	{
		union FloatBytes
		{
			float floatVal;
			uint8_t bytes[4];
		};
		for (int i = 0; i<Image->Num(); i++)
		{
			FloatBytes floatBytes;
			floatBytes.floatVal = Image->GetData()[i].R;
			data[i*ROSStepMultiplier]   = floatBytes.bytes[0];
			data[i*ROSStepMultiplier+1] = floatBytes.bytes[1];
			data[i*ROSStepMultiplier+2] = floatBytes.bytes[2];
			data[i*ROSStepMultiplier+3] = floatBytes.bytes[3];
			
			//data[i*4+3] = Image->GetData()[i].A;
		}
	}else if constexpr (std::is_same_v<T, FFloat16Color>)
	{
		union Float16Bytes
		{
			uint16 floatVal;
			uint8_t bytes[2];
		};
		for (int i = 0; i<Image->Num(); i++)
		{
			Float16Bytes floatBytes;
            floatBytes.floatVal = Image->GetData()[i].R;
			data[i*ROSStepMultiplier] = floatBytes.bytes[0];
			data[i*ROSStepMultiplier] = floatBytes.bytes[1];
			//data[i*4+3] = Image->GetData()[i].A;
		}
	}
	return true;
}

/**
 * read render target texture on GPU and put it into CPU for publishing,
 * slightly higher FPS than alternatives
 */
void UROSSceneCapture::ReadRenderTargetPerRHI()
{
	auto RenderTarget = SceneCapture->TextureTarget;
	FTextureRenderTargetResource *RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[RenderTargetResource, this, RenderTarget]()
	{
		//UE_LOG(LogTemp, Log, TEXT("AsyncTask"))
		if (!RenderTargetResource)
		{
			return;
		}
		switch (RenderTargetResource->GetRenderTargetTexture()->GetDesc().Format)
		{
		case PF_B8G8R8A8:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData8Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX)]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceData(RenderTarget_RT->GetShaderResourceTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
			});
			//RenderTargetResource->ReadPixels(this->ImageData8Bit);
			break;
		case PF_FloatRGBA:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData16Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX)]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceFloatData(RenderTarget_RT->GetShaderResourceTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
			});
			//RenderTargetResource->ReadFloat16Pixels(this->ImageData16Bit);
			break;
		case PF_A32B32G32R32F:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData32Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX)]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceData(RenderTarget_RT->GetShaderResourceTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
			});
			//RenderTargetResource->ReadLinearColorPixels(this->ImageData32Bit);
			//UE_LOG(LogTemp, Log, TEXT("RT READ: {%d}"), ImageData32Bit.Num())
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("NO RT READ: unclear pixel format"))
		}
	});
	
	//FlushRenderingCommands();
}

/**
 * update camera parameters using the Camera variable, and then filters for the actors that should be rendered
 * based on the capture type
 * @param Camera CameraComponent with desired parameters such as FOV, post process
 * @param WorldContext GetWorld()
 */
void UROSSceneCapture::UpdateSceneCaptureCameraParameters(
		UCameraComponent* Camera,
		UWorld* WorldContext
		)
{
	FMinimalViewInfo MinimalViewInfo;
	if (!Camera) {return;}
	Camera->GetCameraView(0.0, MinimalViewInfo);
	
	SceneCapture->FOVAngle = MinimalViewInfo.FOV;
	SceneCapture->PostProcessSettings = MinimalViewInfo.PostProcessSettings;
	SceneCapture->PostProcessBlendWeight = MinimalViewInfo.PostProcessBlendWeight;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	TArray<AActor*> ShowOnlyActors_L;
	switch (CaptureType)
	{
	case ECaptureType::ColorCapture:
		UGameplayStatics::GetAllActorsWithTag(WorldContext, "ColoredImageGen", ShowOnlyActors_L);
		break;
	case ECaptureType::SegmentationCapture:
		UGameplayStatics::GetAllActorsWithTag(WorldContext, "SegmentImageGen", ShowOnlyActors_L);
		break;
	case ECaptureType::DepthCapture:
		UGameplayStatics::GetAllActorsWithTag(WorldContext, "ColoredImageGen", ShowOnlyActors_L);
		break;
	default:
		break;
	}
	SceneCapture->ShowOnlyActors = ShowOnlyActors_L;
}

/**
 * 
 * @return the encoding string for the ROS Image Message
 */
FString UROSSceneCapture::CheckROSEncoding()
{
	switch (RenderTargetFormat)
	{
	case RTF_RGBA8:
		ROSStepMultiplier = 3;
		return "rgb8";
	case RTF_R16f:
	case RTF_RG16f:
	case RTF_RGBA16f:
		ROSStepMultiplier = 2;
        return "16SC1";
	case RTF_R32f:
	case RTF_RG32f:
	case RTF_RGBA32f:
		ROSStepMultiplier = 4;
		return "32FC1";
	default:
		ROSStepMultiplier = 1;
		return "undefined(UE5_Encoding)";
	}
}

