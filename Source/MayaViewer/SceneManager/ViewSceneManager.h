// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NetInfo/NetCameraInfo.h"
#include "JsonObject.h"
#include "ViewSceneManager.generated.h"


class UCommandList;
class ALiteratumActorBase;

USTRUCT(BlueprintType)
struct  FSceneDescription 
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FString Command;

	UPROPERTY()
		TMap<FString, FString> SceneObjects;
};

USTRUCT(BlueprintType)
struct  FUpdateObjectTransform
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FString Command;

	UPROPERTY()
		FString objectName;

	UPROPERTY()
		TArray<float> WorldMatrix;
};


/**
 * 
 */
UCLASS(Blueprintable)
class MAYAVIEWER_API ULiteratumSceneManager : public UObject
{
	GENERATED_BODY()
	
public:
	FNetCameraInfo m_CameraInfo;

	void UpdateSceneDescription(TSharedPtr<FJsonObject> InputJsonObject);
	void UpdateSceneObjectTransfrom(TSharedPtr<FJsonObject> InputString);
	void RequestObjectTransform(FString ObjectName);
	void RequestObjectMeta(FString ObjectName);
	void Setup(UCommandList* m_CommandList, UWorld* ParentWorld);
	void SetObjectMeta(TSharedPtr<FJsonObject> InputJsonObject);

	FVector MayaToUE4SpacePosition(FVector InPosition) { return FVector(InPosition.X,InPosition.Z,InPosition.Y); }

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
		const FNetCameraInfo GetActiveCameraInfo() { return m_CameraInfo; };

private:
	friend class UCommandList;
	void SetCamera(FNetCameraInfo NewCamerainfo) { m_CameraInfo = NewCamerainfo; }


	UPROPERTY()
		UCommandList* m_CommandList;

	UPROPERTY()
		UWorld* m_ParentWorld;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TMap<FString, ALiteratumActorBase*> m_SceneActors;
	
};
